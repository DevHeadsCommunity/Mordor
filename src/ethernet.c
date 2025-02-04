#include "ethernet.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(mordor, LOG_LEVEL_INF);

#include <zephyr/net/ethernet.h>
#include <zephyr/net/ethernet_mgmt.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/net_mgmt.h>

#define NET_BUF_TIMEOUT K_MSEC(100)

static const struct net_eth_addr broadcast_eth_addr = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

#define print_ll_addrs(pkt, type, len, src, dst)                               \
  if (CONFIG_NET_L2_ETHERNET_LOG_LEVEL >= LOG_LEVEL_DBG) {                     \
    char out[sizeof("xx:xx:xx:xx:xx::xx")];                                    \
    snprintk(out, sizeof(out), "%s",                                           \
             net_sprint_ll_addr((src)->addr, sizeof(struct net_eth_addr)));    \
    NET_DBG("iface %d (%p) src %s dst %s type 0x%x len %zu",                   \
            net_if_get_by_iface(net_pkt_iface(pkt)), net_pkt_iface(pkt), out,  \
            net_sprint_ll_addr((dst)->addr, sizeof(struct net_eth_addr)),      \
            type, (size_t)len);                                                \
  }

static inline void update_ethernet_length(struct net_if *iface,
                                          struct net_pkt *pkt) {

  uint16_t len;

  /* Let's check IP payload's length. If it's smaller than 46 bytes,
   * i.e. smaller than minimal Ethernet frame size minus ethernet
   * header size, then Ethernet has padded so it fits in the minimal
   * frame size of 60 bytes. In that case, we need to get rid of it.
   */

  if (net_pkt_family(pkt) == AF_INET) {
    len = ntohs(NET_IPV4_HDR(pkt)->len);
  }

  if (len < NET_ETH_MINIMAL_FRAME_SIZE - sizeof(struct net_eth_addr)) {
    struct net_buf *frag;

    for (frag = pkt->frags; frag; frag = frag->frags) {
      if (frag->len < len) {
        len -= frag->len;
      } else {
        frag->len = len;
        len = 0U;
      }
    }
  }
}

static void update_ethernet_rx_stats(struct net_if *iface, size_t length) {
#if defined(CONFIG_NET_STATISTICS_ETHERNET)
  eth_stats_update_bytes_rx(iface, length);
  eth_stats_update_pkts_rx(iface);
  eth_stats_update_broadcast_rx(iface);
#endif /* CONFIG_NET_STATISTICS_ETHERNET */
}

/* Drop packet if it has broadcast destination MAC address but the IPv4
 * address is not multicast or broadcast address.
 */
static inline enum net_verdict
check_ethernet_ipv4_bcast_addr(struct net_pkt *pkt, struct net_eth_hdr *hdr) {
  if (net_eth_is_addr_broadcast(&hdr->dst) &&
      !(net_ipv4_is_addr_mcast((struct in_addr *)NET_IPV4_HDR(pkt)->dst) ||
        net_ipv4_is_addr_bcast(net_pkt_iface(pkt),
                               (struct in_addr *)NET_IPV4_HDR(pkt)->dst))) {
    return NET_DROP;
  }
  return NET_OK;
}

static enum net_verdict recv_ethernet(struct net_if *iface,
                                      struct net_pkt *pkt) {

  struct ethernet_context *ctx = net_if_l2_data(iface);
  uint8_t hdr_len = sizeof(struct net_eth_hdr);
  size_t body_len;
  struct net_eth_hdr *hdr = NET_ETH_HDR(pkt);
  enum net_verdict verdict = NET_CONTINUE;
  bool handled = false;
  struct net_linkaddr *lladdr;
  uint16_t type;

  bool dst_broadcast, dst_iface_addr;

  if (hdr == NULL || pkt->buffer->len < hdr_len) {
    goto drop;
  }

  type = ntohs(hdr->type);

  lladdr = net_pkt_lladdr_src(pkt);
  lladdr->addr = hdr->src.addr;
  lladdr->len = sizeof(struct net_eth_addr);
  lladdr->type = NET_LINK_ETHERNET;

  lladdr = net_pkt_lladdr_dst(pkt);
  lladdr->addr = hdr->dst.addr;
  lladdr->len = sizeof(struct net_eth_addr);
  lladdr->type = NET_LINK_ETHERNET;

  net_pkt_set_ll_proto_type(pkt, type);
  dst_broadcast =
      net_eth_is_addr_broadcast((struct net_eth_addr *)lladdr->addr);
  dst_iface_addr = net_linkaddr_cmp(net_if_get_link_addr(iface), lladdr);

  print_ll_addrs(pkt, type, net_pkt_get_len(pkt), net_pkt_lladdr_src(pkt),
                 net_pkt_lladdr_dst(pkt));

  if (!(dst_broadcast || dst_iface_addr)) {
    /* The ethernet frame is not for me as the link addresses
     * are different.
     */
    NET_DBG("Dropping frame, not for me[%s]",
            net_sprint_ll_addr(net_if_get_link_addr(iface)->addr,
                               sizeof(struct net_eth_addr)));
    goto drop;
  }

  /* Get rid of the Ethernet header */
  net_buf_pull(pkt->frags, hdr_len);

  body_len = net_pkt_get_len(pkt);

  STRUCT_SECTION_FOREACH(net_l3_register, l3) {
    if (l3->ptype != type || l3->l2 != &NET_L2_GET_NAME(ETHERNET) ||
        l3->handler == NULL) {
      continue;
    }

    NET_DBG("Calling L3 %s handler for type 0x%04x iface %d (%p)", l3->name,
            type, net_if_get_by_iface(iface), iface);

    verdict = l3->handler(iface, type, pkt);
    if (verdict == NET_OK) {
      /* the packet was consumed by the l3-handler */
      goto out;
    } else if (verdict == NET_DROP) {
      NET_DBG("Dropping frame, packet rejected by %s", l3->name);
      goto drop;
    }

    /* The packet will be processed further by IP-stack
     * when NET_CONTINUE is returned
     */
    handled = true;
    break;
  }

  if (!handled) {
    if (IS_ENABLED(CONFIG_NET_ETHERNET_FORWARD_UNRECOGNISED_ETHERTYPE)) {
      net_pkt_set_family(pkt, AF_UNSPEC);
    } else {
      NET_DBG("Unknown hdr type 0x%04x iface %d (%p)", type,
              net_if_get_by_iface(iface), iface);
      eth_stats_update_unknown_protocol(iface);
      return NET_DROP;
    }
  }

  if (type != NET_ETH_PTYPE_EAPOL) {
    update_ethernet_length(iface, pkt);
  }

out:
  update_ethernet_rx_stats(iface, body_len + hdr_len);
  return verdict;

drop:
  eth_stats_update_errors_rx(iface);
  return NET_DROP;
}

static enum net_verdict recv_ethernet_ip(struct net_if *iface, uint16_t ptype,
                                         struct net_pkt *pkt) {

  ARG_UNUSED(iface);

  if (ptype == NET_ETH_PTYPE_IP) {
    struct net_eth_hdr *hdr = NET_ETH_HDR(pkt);

    if (check_ethernet_ipv4_bcast_addr(pkt, hdr) == NET_DROP) {
      return NET_DROP;
    }

    net_pkt_set_family(pkt, AF_INET);
  } else {
    return NET_DROP;
  }

  return NET_CONTINUE;
}

ETH_NET_L3_REGISTER(IPv4, NET_ETH_PTYPE_IP, recv_ethernet_ip);

static inline bool ethernet_is_broadcast(struct net_pkt *pkt) {
  if (net_ipv4_is_addr_bcast(net_pkt_iface(pkt),
                             (struct in_addr *)NET_IPV4_HDR(pkt)->dst)) {
    return true;
  }
  return false;
}

static struct net_pkt *prepare_ethernet_ll(struct net_if *iface,
                                           struct net_pkt *pkt) {

  struct ethernet_context *ctx = net_if_l2_data(iface);

  if (ethernet_ipv4_dst_is_broadcast(pkt)) {
    return pkt;
  }

  if (IS_ENABLED(CONFIG_NET_ARP)) {
    struct net_pkt *arp_pkt;

    arp_pkt =
        net_arp_prepare(pkt, (struct in_addr *)NET_IPV4_HDR(pkt)->dst, NULL);

    if (!arp_pkt) {
      return NULL;
    }

    if (pkt != arp_pkt) {
      NET_DBG("Sending arp pkt %p (orig %p) to iface %d (%p)", arp_pkt, pkt,
              net_if_get_by_iface(iface), iface);
      net_pkt_unref(pkt);
      return arp_pkt;
    }

    NET_DBG("Found ARP entry, sending pkt %p to iface %d (%p)", pkt,
            net_if_get_by_iface(iface), iface);
  }

  return pkt;
}

static inline size_t get_reserve_ll_header_size(struct net_if *iface) {
  if (net_if_l2(iface) != &NET_L2_GET_NAME(ETHERNET)) {
    return 0U;
  }

  if (!IS_ENABLED(CONFIG_NET_L2_ETHERNET_RESERVE_HEADER)) {
    return 0U;
  }

  return sizeof(struct net_eth_hdr);
}

static struct net_buf *fill_ethernet_header(struct ethernet_context *ctx,
                                            struct net_if *iface,
                                            struct net_pkt *pkt,
                                            uint32_t ptype) {

  struct net_if *orig_iface = iface;
  struct net_buf *hdr_frag;
  struct net_eth_hdr *hdr;
  size_t reserve_ll_header;
  size_t hdr_len;

  reserve_ll_header = get_reserve_ll_header_size(orig_iface);
  if (reserve_ll_header > 0) {
    hdr_len = reserve_ll_header;
    hdr_frag = pkt->buffer;

    NET_DBG("Making room for link header %zd bytes", hdr_len);

    /* Make room for the header */
    net_buf_push(pkt->buffer, hdr_len);
  } else {
    hdr_len = sizeof(struct net_eth_hdr);
    hdr_frag = net_pkt_get_frag(pkt, hdr_len, NET_BUF_TIMEOUT);
    if (!hdr_frag) {
      return NULL;
    }
  }

  hdr = (struct net_eth_hdr *)(hdr_frag->data);

  if (reserve_ll_header == 0U) {
    hdr_len = sizeof(struct net_eth_hdr);
    net_buf_add(hdr_frag, hdr_len);
  }

  if (ptype == htons(NET_ETH_PTYPE_ARP)) { /* ??? */
    memcpy(&hdr->dst, net_pkt_lladdr_dst(pkt)->addr,
           sizeof(struct net_eth_addr));
  }

  memcpy(&hdr->src, net_pkt_lladdr_src(pkt)->addr, sizeof(struct net_eth_addr));
  hdr->type = ptype;
  print_ll_addrs(pkt, ntohs(hdr->type), hdr_len, &hdr->src, &hdr->dst);

  if (reserve_ll_header == 0U) {
    net_pkt_frag_insert(pkt, hdr_frag);
  }
  return hdr_frag;
}

static void update_ethernet_tx_stats(struct net_if *iface,
                                     struct net_pkt *pkt) {
#if CONFIG_NET_STATISTICS_ETHERNET
  struct net_eth_hdr *hdr = NET_ETH_HDR(pkt);
  eth_stats_update_bytes_tx(iface, net_pkt_get_len(pkt));
  eth_stats_update_pkts_tx(iface);

  if (net_eth_is_addr_broadcast(&hdr->dst)) {
    eth_stats_update_broadcast_tx(iface);
  }
#endif
}

static int send_ethernet(struct net_if *iface, struct net_pkt *pkt) {

  const struct ethernet_api *api = net_if_get_device(iface)->api;
  struct ethernet_context *ctx = net_if_l2_data(iface);
  uint16_t ptype = htons(net_pkt_ll_proto_type(pkt));
  struct net_pkt *orig_pkt = pkt;
  int ret;

  if (!api) {
    ret = -ENOENT;
    goto error;
  }

  if (!api->send) {
    ret = -ENOTSUP;
    goto error;
  }

  if (net_pkt_family(pkt == AF_INET &&
                     net_pkt_ll_proto_type(pkt) == NET_ETH_PTYPE_IP)) {
    if (!net_pkt_ipv4_acd(pkt)) {
      struct net_pkt *tmp;

      tmp = ethernet_ll_prepare(iface, pkt);
      if (tmp == NULL) {
        ret = -ENOMEM;
        goto error;
      } else if (IS_ENABLED(CONFIG_NET_ARP) && tmp != pkt) {
        /* Original pkt got queued and is replaced
         * by an ARP request packet.
         */
        pkt = tmp;
        ptype = htons(net_pkt_ll_proto_type(pkt));
      }
    }
  } else if (IS_ENABLED(CONFIG_NET_SOCKETS_PACKET) &&
             net_pkt_family(pkt) == AF_PACKET) {
    struct net_context *context = net_pkt_context(pkt);

    if (!(context && net_context_get_type(context) == SOCK_DGRAM)) {
      /* Raw packet, just send it */
      goto send;
    }
  }

  if (ptype == 0) {
    NET_ERR("No protocol set for pkt %p", pkt);
    ret = -ENOTSUP;
    goto error;
  }

  /* If the ll dst addr has not been set before, let's assume
   * temporarily it's a broadcast one. When filling the header,
   * it might detect this should be multicast and act accordingly.
   */
  if (!net_pkt_lladdr_dst(pkt)->addr) {
    net_pkt_lladdr_dst(pkt)->addr = (uint8_t *)broadcast_eth_addr.addr;
    net_pkt_lladdr_dst(pkt)->len = sizeof(struct net_eth_addr);
  }

  if (!fill_ethernet_header(ctx, iface, pkt, ptype)) {
    ret = -ENOMEM;
    goto arp_error;
  }

  net_pkt_cursor_init(pkt);

send:
  ret = net_l2_send(api->send, net_if_get_device(iface), iface, pkt);
  if (ret != 0) {
    eth_stats_update_errors_tx(iface);
    goto arp_error;
  }

  update_ethernet_tx_stats(iface, pkt);

  ret = net_pkt_get_len(pkt);

  net_pkt_unref(pkt);

error:
  return ret;

arp_error:
  if (IS_ENABLED(CONFIG_NET_ARP) && ptype == htons(NET_ETH_PTYPE_ARP)) {
    /* Original packet was added to ARP's pending Q, so, to avoid it
     * being freed, take a reference, the reference is dropped when we
     * clear the pending Q in ARP and then it will be freed by net_if.
     */

    net_pkt_ref(orig_pkt);
    if (net_arp_clear_pending(iface,
                              (struct in_addr *)NET_IPV4_HDR(pkt)->dst)) {
      NET_DBG("Could not find ARP entry");
    }

    net_pkt_unref(pkt);
  }
  return ret;
}

static int inline int enable_ethernet(struct net_if *iface, bool state) {
  int ret = 0;
  const struct ethernet_api *eth = net_if_get_device(iface)->api;

  if (!eth) {
    return -ENOENT;
  }

  if (!state) {
    net_arp_clear_cache(iface);

    if (eth->stop) {
      ret = eth->stop(net_if_get_device(iface));
    }
  } else {

    if (eth->start) {
      ret = eth->start(net_if_get_device(iface));
    }
  }
  return ret;
}

enum net_l2_flags ethernet_flags(struct net_if *iface) {
  struct ethernet_context *ctx = net_if_l2_data(iface);
  return ctx->ethernet_l2_flags;
}

static int alloc_l2_ethernet(struct net_if *iface, struct net_pkt *pkt,
                             size_t size, enum net_ip_protocol proto,
                             k_timeout_t timeout) {

  size_t reserve = get_reserve_ll_header_size(iface);
  struct ethernet_config config;

  if (net_eth_get_hw_config(iface, ETHERNET_CONFIG_TYPE_EXTRA_TX_PKT_HEADROOM,
                            &config) == 0) {
    reserve += config.extra_tx_pkt_headroom;
  }
  return net_pkt_alloc_buffer_with_reserve(pkt, size, reserve, proto, timeout);
}

NET_L2_INIT(ETHERNET_L2, recv_ethernet, send_ethernet, enable_ethernet,
            ethernet_flags, alloc_l2_ethernet);

static void carrier_on_off(struct k_work *work) {
  struct ethernet_context *ctx =
      CONTAINER_OF(work, struct ethernet_context, carrier_work);

  bool eth_carrier_up;

  if (ctx->iface == NULL) {
    return;
  }

  eth_carrier_up = atomic_test_bit(&ctx->flags, ETH_CARRIER_UP);

  if (eth_carrier_up == ctx->is_net_carrier_up) {
    return;
  }

  ctx->is_net_carrier_up = eth_carrier_up;

  NET_DBG("Carrier %s for interface %p", eth_carrier_up ? "ON" : "OFF",
          ctx->iface);

  if (eth_carrier_up) {
    ethernet_mgmt_raise_carrier_on_event(ctx->iface);
    net_if_carrier_on(ctx->iface);
  } else {
    ethernet_mgmt_raise_carrier_off_event(ctx->iface);
    net_if_carrier_off(ctx->iface);
  }
}

void net_eth_carrier_on(struct net_if *iface) {
  struct ethernet_context *ctx = net_if_l2_data(iface);

  if (!atomic_test_and_set_bit(&ctx->flags, ETH_CARRIER_UP)) {
    k_work_submit(&ctx->carrier_work);
  }
}

void net_eth_carrier_off(struct net_if *iface) {
  struct ethernet_context *ctx = net_if_l2_data(iface);

  if (atomic_test_and_set_bit(&ctx->flags, ETH_CARRIER_UP)) {
    k_work_submit(&ctx->carrier_work);
  }
}

const struct device *net_eth_get_phy(struct net_if *iface) {

  const struct device *dev = net_if_get_device(iface);
  const struct ethernet_api *api = dev->api;

  if (!api) {
    return NULL;
  }

  if (net_if_l2(iface) != &NET_L2_GET_NAME(ETHERNET)) {
    return NULL;
  }

  if (!api->get_phy) {
    return NULL;
  }

  return api->get_phy(net_if_get_device(iface));
}

int init_ethernet(struct net_if *iface) {
  struct ethernet_context *ctx = net_if_l2_data(iface);

  NET_DBG("Initializing Ethernet L2 %p for iface %d (%p)", ctx,
          net_if_get_by_iface(iface), iface);

  ctx->ethernet_l2_flags = NET_L2_POINT_TO_POINT;
  ctx->iface = iface;
  k_work_init(&ctx->carrier_work, carrier_on_off);

  return 0;
}

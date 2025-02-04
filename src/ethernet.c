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

const struct net_eth_addr *net_eth_broadcast_addr(void) {
  return &broadcast_eth_addr;
}

#define print_ll_addrs(pkt, type, len, src, dst)                               \
  if (CONFIG_NET_L2_ETHERNET_LOG_LEVEL >= LOG_LEVEL_DBG) {                     \
    char out[sizeof("xx:xx:xx:xx:xx::xx")];                                    \
    snprintk(out, sizeof(out), "%s",                                           \
             net_sprint_ll_addr((src)->addr, sizeof(struct net_eth_addr)));    \
    NET_DBG("iface %d (%p) src %s dst %s type 0x%x len %zu",                   \
            net_if_get_by_iface(net_pkt_iface(pkt)), net_pkt_iface(pkt), out,  \
            net_sprint_ll_addr((dst)->addr, sizeof(struct net_eth_addr)),      \
            type, (size_t)len);
}

static inline void ethernet_update_length(struct net_if *iface,
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

  if (len < NET_ETH_MINIMAIL_FRAME_SIZE - sizeof(struct net_eth_addr)) {
    struct net_buf *frag;

    for (frag = pkt->frags; frag = frag->frags) {
      if (frag->len < len) {
        len -= frag->len;
      } else {
        frag->len = len;
        len = 0U;
      }
    }
  }
}

static void ethernet_update_rx_stats(struct net_if *iface,
                                     struct net_eth_hdr *hdr, size_t length) {
#if defined(CONFIG_NET_STATISTICS_ETHERNET)
  eth_stats_update_bytes_rx(iface, length);
  eth_stats_update_pkts_rx(iface);
  eth_stats_update_broadcast_rx(iface);
#endif /* CONFIG_NET_STATISTICS_ETHERNET */
}

int init_ethernet(void) {}

int stop_ethernet(void) {}

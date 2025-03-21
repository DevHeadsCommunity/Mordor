#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(mordor, LOG_LEVEL_INF);

#include <errno.h>
#include <zephyr/kernel.h>

#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_l2.h>

static int setup_iface(struct net_if *iface, const char *ipv4_addr) {
  struct net_if_addr *ifaddr;
  struct in_addr addr4;

  if (net_addr_pton(AF_INET, ipv4_addr, &addr4)) {
    LOG_ERR("Invalid address: %s", ipv4_addr);
    return -EINVAL;
  }

  ifaddr = net_if_ipv4_addr_add(iface, &addr4, NET_ADDR_MANUAL, 0);
  if (!ifaddr) {
    LOG_ERR("Cannot add %s to interface %p", ipv4_addr, iface);
    return -EINVAL;
  }

  return 0;
}

static enum net_verdict parse_lldp(struct net_if *iface, struct net_pkt *pkt) {
  LOG_DBG("iface %p Parsing LLDP, len %zu", iface, net_pkt_get_len(pkt));

  net_pkt_cursor_init(pkt);

  while (1) {
    uint16_t type_length;
    uint16_t length;
    uint8_t type;

    if (net_pkt_read_be16(pkt, &type_length)) {
      LOG_DBG("End LLDP DU TLV");
      break;
    }

    length = type_length & 0x1FF;
    type = (uint8_t)(type_length >> 9);

    /* Skip for now data */
    if (net_pkt_skip(pkt, length)) {
      LOG_DBG("");
      break;
    }

    switch (type) {
    case LLDP_TLV_CHASSIS_ID:
      LOG_DBG("Chassis ID");
      break;
    case LLDP_TLV_PORT_ID:
      LOG_DBG("Port ID");
      break;
    case LLDP_TLV_TTL:
      LOG_DBG("TTL");
      break;
    default:
      LOG_DBG("TLV Not parsed");
      break;
    }

    LOG_DBG("type_length %u type %u length %u", type_length, type, length);
  }

  /* Let stack to free the packet */
  return NET_DROP;
}

int init_ethernet(struct net_if *iface) {
  enum ethernet_hw_caps caps;

  iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
  if (!iface) {
    LOG_ERR("No ethernet interfaces found.");
    return -ENOENT;
  }

  caps = net_eth_get_hw_capabilities(iface);
  if (!(caps & ETHERNET_LLDP)) {
    LOG_ERR("Interface %p does not support %s", iface, "LLDP");
    LOG_ERR("Cannot continue!");
    return -ENOENT;
  }

  setup_iface(iface, "0.0.0.0");

  net_lldp_register_callback(iface, parse_lldp);

  return 0;
}

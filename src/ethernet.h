#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <zephyr/net/net_if.h>

int init_ethernet(struct net_if *iface);
int stop_ethernet(void);

#endif

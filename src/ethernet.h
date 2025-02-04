#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <zephyr/logging/log.h>

int init_ethernet(void);
int stop_ethernet(void);

#endif

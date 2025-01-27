#ifndef __CANBUS_H__
#define __CANBUS_H__

#include <zephyr/drivers/can.h>

#define CANBUS_NODE DT_CHOSEN(zephyr_canbus)

int init_canbus(void);

#endif

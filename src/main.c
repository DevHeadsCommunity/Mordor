#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mordor);

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/sys/reboot.h>

#include "canbus.h"
#include "common.h"
#include "ethernet.h"
#include "foc_speed.h"

int main(void) {
  struct net_if iface;
  LOG_INF("Initializing CAN");
  if (!init_canbus()) {
    LOG_WRN("Failed to initialize canbus");
  }

  LOG_INF("Initializing ethernet");
  if (!init_ethernet(&iface)) {
    LOG_WRN("Failed to initialize ethernet");
  }

  foc_speed_t foc = {0};
  foc_speed_init(&foc);

  while (1) {
    foc_speed_step(&foc);

    k_sleep(K_USEC(20));
  }

  return 0;
}

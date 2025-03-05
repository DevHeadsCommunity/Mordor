#include "canbus.h"
#include "common.h"
#include "foc_speed.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mordor);

int main(void) {
  if (!init_canbus()) {
    LOG_INF("Failed to initialize canbus");
  }

  foc_speed_t foc = {0};

  foc_speed_init(&foc);

  while (1) {
    foc_speed_step(&foc);

    K_USEC(20);
  }

  return 0;
}

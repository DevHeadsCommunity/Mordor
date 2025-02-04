#include "canbus.h"
#include "common.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(mordor);

int main(void) {

  if (!init_canbus()) {
    LOG_INF("Failed to initialize canbus");
  }

  return 0;
}

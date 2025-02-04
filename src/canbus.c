#include "canbus.h"
#include "common.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(mordor, LOG_LEVEL_INF);

#include <zephyr/kernel.h>

// using uniform values for both threads, at least for now
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 2

CAN_MSGQ_DEFINE(canbus_rx_msgq, 8);

K_TIMER_DEFINE(can_tx_timer, NULL, NULL);

K_THREAD_STACK_DEFINE(rx_thread_stack, THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(tx_thread_stack, THREAD_STACK_SIZE);

struct k_thread rx_thread_data;
struct k_thread tx_thread_data;

struct k_mutex candata_mutex;

const struct device *const can_device = DEVICE_DT_GET(CANBUS_NODE);

void can_tx_thread(void *arg1, void *arg2, void *arg3) {
  // assuming
  ARG_UNUSED(arg1);
  ARG_UNUSED(arg2);
  ARG_UNUSED(arg3);

  int ret;
  struct can_frame frame = {0};

  k_timer_start(&can_tx_timer, K_SECONDS(1), K_SECONDS(1));

  while (1) {
    k_timer_status_sync(&can_tx_timer);

    // fill in frame data using args

    // sender blocks until message is sent
    ret = can_send(can_device, &frame, K_FOREVER, NULL, NULL);
    if (ret != 0) {
      LOG_INF("CAN send has failed [%d]", ret);
    }
  }
}

void can_rx_thread(void *arg1, void *arg2, void *arg3) {
  ARG_UNUSED(arg1);
  ARG_UNUSED(arg2);
  ARG_UNUSED(arg3);

  struct can_frame frame;
  // set can filter (mask = 0, takes all frames std and ext)
  // lets peek
  const struct can_filter filter = {
      .flags = CAN_FILTER_IDE, .id = 0, .mask = 0};

  can_add_rx_filter_msgq(can_device, &canbus_rx_msgq, &filter);

  while (1) {
    k_msgq_get(&canbus_rx_msgq, &frame, K_FOREVER);

    k_mutex_lock(&candata_mutex, K_FOREVER);

    // process our frames

    k_mutex_unlock(&candata_mutex);
  }
}

int init_canbus(void) {

  int ret;
  k_tid_t rx_tid, tx_tid;

  if (!device_is_ready(can_device)) {
    LOG_ERR("CAN device %s not ready\n", can_device->name);
    return APP_FAIL;
  }

  ret = can_start(can_device);
  if (ret != 0) {
    LOG_ERR("CAN could not start controller [%d]", ret);
    return APP_FAIL;
  }

  LOG_INF("Starting tx thread...\n");
  tx_tid = k_thread_create(
      &tx_thread_data, tx_thread_stack, K_THREAD_STACK_SIZEOF(tx_thread_stack),
      can_tx_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, K_NO_WAIT);
  if (!tx_tid) {
    LOG_INF("ERROR spawning tx thread\n");
  }

  LOG_INF("Starting rx thread...\n");
  rx_tid = k_thread_create(
      &rx_thread_data, rx_thread_stack, K_THREAD_STACK_SIZEOF(rx_thread_stack),
      can_rx_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, K_NO_WAIT);
  if (!rx_tid) {
    LOG_INF("ERROR spawning rx thread\n");
  }

  return APP_SUCC;
}

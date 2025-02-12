/*
 *
 *
 *
 * */

#define DT_DRV_COMPAT ti_drv8305

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(DRV8305, CONFIG_GATE_DRIVER_LOG_LEVEL);

#define DRV8305_STATUS_01_REG_ADDR 0x01  /* Warning and Watchdog Reset */
#define DRV8305_STATUS_02_REG_ADDR 0x02  /* OV/VDS Faults */
#define DRV8305_STATUS_03_REG_ADDR 0x03  /* IC Faults */
#define DRV8305_STATUS_04_REG_ADDR 0x04  /* VGS Faults */
#define DRV8305_CONTROL_05_REG_ADDR 0x05 /* HS Gate Drive */
#define DRV8305_CONTROL_06_REG_ADDR 0x06 /* LS Gate Drive */
#define DRV8305_CONTROL_07_REG_ADDR 0x07 /* Gate Drive Control */
#define DRV8305_CONTROL_09_REG_ADDR 0x09 /* IC Operation */
#define DRV8305_CONTROL_0A_REG_ADDR 0x0A /* Shunt Amp Control */
#define DRV8305_CONTROL_0B_REG_ADDR 0x0B /* Voltage Regulator Control */
#define DRV8305_CONTROL_0C_REG_ADDR 0x0C /* VDS Sense Control */

struct drv8305_config {
  struct spi_dt_spec spi;
  struct gpio_dt_spec en_gate;
  struct gpio_dt_spec n_fault;
};

struct drv8305_data {
  const struct device *dev;
};

static int drv8305_reset(const struct device *dev) { return 0; }

static int drv8305_gpio_config(const struct device *dev) {
  const struct drv8305_config *config = dev->config;
  int ret;

  if (config->en_gate.port != NULL) {
    if (!gpio_is_ready_dt(&config->en_gate)) {
      return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&config->en_gate, GPIO_OUTPUT_ACTIVE);
  }
}

#ifndef __DRV8305_H__
#define __DRV8305_H__

#define DT_DRV_COMPAT ti_drv8305

#include <zephyr/drivers/spi.h>

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
};

#endif

#include "foc.h"
#include "pmsm_foc.h"
#include "stepper_foc.h"

void foc_init(void *foc) {
#if CONFIG_STEPPER_FOC
  stepper_foc_t *stepper_foc = (stepper_foc_t *)foc;
  stepper_foc_init(stepper_foc);
#else
  pmsm_foc_t *pmsm_foc = (pmsm_foc_t *)foc;
  pmsm_foc_init(pmsm_foc);
#endif
}

void foc_step(void *foc) {
#if CONFIG_STEPPER_FOC
  stepper_foc_t *stepper_foc = (stepper_foc_t *)foc;
  stepper_foc_step(stepper_foc);
#else
  pmsm_foc_t *pmsm_foc = (pmsm_foc_t *)foc;
  pmsm_foc_step(pmsm_foc);
#endif
}

void foc_velocity_open_loop() {}

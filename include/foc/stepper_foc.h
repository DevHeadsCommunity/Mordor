#ifndef __STEPPER_FOC_H__
#define __STEPPER_FOC_H__

typedef struct {
} stepper_foc_t;

void stepper_foc_init(stepper_foc_t *stepper_foc);
void stepper_foc_step(stepper_foc_t *stepper_foc);

#endif

#ifndef __SVM_H__
#define __SVM_H__

#include "types.h"

#include <stdint.h>

/**
 * Three phase duty cycle
 */
typedef struct {
  float a;
  float b;
  float c;
} duty_cycle_t;

typedef struct {
  uint8_t sector;
  duty_cycle_t duties;

  float d_min;
  float d_max;
} svm_t;

int space_vector_pwm_run(svm_t *svm, const float alpha, const float beta);

#endif

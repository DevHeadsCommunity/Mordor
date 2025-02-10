#ifndef __PID_H__
#define __PID_H__

#include "types.h"

typedef struct {
  float setpoint;
  float measurement;

  float output;

  float sample_time;
  float upper_limit;
  float lower_limit;
  float upper_limit_int;
  float lower_limit_int;
  float kp;
  float ki;

  float integrator;
  float prev_error;
} pi_t;

void pi_init(pi_t *pi);

void pi_step(pi_t *pi);

#endif

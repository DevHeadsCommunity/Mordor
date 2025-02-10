#include "pid.h"
#include "types.h"

void pi_init(pi_t *pi) {
  pi->integrator = 0;
  pi->prev_error = 0;
}

void pi_step(pi_t *pi) {
  float error = pi->setpoint - pi->measurement;

  // Proportional
  float p = pi->kp * error;

  // Integral
  pi->integrator = pi->integrator +
                   0.5f * pi->ki * pi->sample_time * (error + pi->prev_error);

  // Integral anti-windup
  pi->integrator =
      sat(pi->integrator, pi->lower_limit_int, pi->upper_limit_int);

  // Don't perform derivative... (PI not PID)

  // Output and saturation
  pi->output = sat(p + pi->integrator, pi->lower_limit, pi->upper_limit);

  pi->prev_error = error;
}

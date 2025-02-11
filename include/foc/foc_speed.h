#ifndef __FOC_SPEED_H__
#define __FOC_SPEED_H__

#include "pid.h"
#include "pmsm_foc.h"

#include <stdbool.h>

typedef struct {
  float rpmReq;                    // Requested rpm [rpm]
  float rpmMeas;                   // Measured rpm [rpm]
  bool tqEnable;                   // Enable torque (false to coast)
  three_phase_current_t iabcSense; // 3ph current [A]
  float wSense;                    // mechnical angular velocity [rad/s]
  float mech_rotor_angle;                // mechanical angle [rad]
  float vdcSense;                  // DC bus voltage [V]

  // outputs
  svpwm_t svm;
  direct_quadrature_voltage_t vdq;
  direct_quadrature_current_t idq_ref;
  float torque_ref_saturation;
  float torque_limit;
  float torque_estimate;
  float rpm_base;

  // params
  float sample_time; // sample time
  float max_torque;  // Max torque [Nm]

  foc_t foc;
  pi_t pi_speed;
} foc_speed_t;

void foc_speed_init(foc_speed_t *foc);

void foc_speed_step(foc_speed_t *foc);

#endif

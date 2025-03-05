#ifndef __PMSM_FOC_H__
#define __PMSM_FOC_H__

#include "low_pass_filter.h"
#include "pmsm_current_controller.h"
#include "pmsm_current_ref.h"
#include "svpwm.h"

#define FOC_VDC_FILTER_TIME_CONST 0.01f;

typedef struct {

  float torque_ref;
  three_phase_current_t current; // 3ph current [A]
  float mech_velocity;           // mechanical angular velocity [rad/s]
  float mech_rotor_angle;
  float voltage_supply; // DC bus voltage [V]

  // outputs
  svpwm_t svm;
  direct_quadrature_voltage_t vdq;
  direct_quadrature_current_t idq_ref;
  float torque_ref_saturation;
  float torque_limit;
  float torque_estimate;
  float rpm_base;

  // params
  float sample_time;
  uint16_t pole_pairs;

  pmsm_current_ref_t current_ref;
  pmsm_current_controller_t controller;

  // internal variables
  low_pass_filter_t vdc_filter;
} pmsm_foc_t;

void pmsm_foc_init(pmsm_foc_t *foc);
void pmsm_foc_step(pmsm_foc_t *foc);
float pmsm_velocity_open_loop(pmsm_foc_t *foc, float target_speed);

#endif

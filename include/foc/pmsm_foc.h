#ifndef __PMSM_FOC_H__
#define __PMSM_FOC_H__

#include "low_pass_filter.h"
#include "pmsm_current_controller.h"
#include "pmsm_current_ref.h"
#include "svm.h"

#define FOC_VDC_FILTER_TIME_CONST 0.01f;

typedef struct {

  float torque_ref;                // Torque request [Nm]
  three_phase_current_t iabcSense; // 3ph current [A]
  float wSense;                    // mechanical angular velocity [rad/s]
  float thetaSense;                // mechanical angle [rad]
  float vdcSense;                  // DC bus voltage [V]

  // outputs
  svm_t svm;
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
} foc_t;

void foc_init(foc_t *foc);

void foc_step(foc_t *foc);

#endif

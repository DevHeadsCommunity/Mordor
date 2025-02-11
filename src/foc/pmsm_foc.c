#include "pmsm_foc.h"
#include "transforms.h"

#include <math.h>
#include <string.h>

void foc_init(foc_t *foc) {
  // initial values
  foc->torque_ref = 0.0f;
  foc->wSense = 0.0f;
  foc->mech_rotor_angle = 0.0f;
  foc->vdcSense = 0.0f;
  memset(&foc->iabcSense, 0, sizeof(three_phase_current_t));

  memset(&foc->svm.duties, 0, sizeof(duty_cycle_t));
  memset(&foc->vdq, 0, sizeof(direct_quadrature_voltage_t));
  memset(&foc->idq_ref, 0, sizeof(direct_quadrature_current_t));
  foc->torque_ref_saturation = 0.0f;
  foc->torque_limit = 0.0f;
  foc->torque_estimate = 0.0f;
  foc->rpm_base = 0.0f;

  // consistent params
  foc->controller.sample_time = foc->sample_time;
  foc->current_ref.pole_pairs = foc->pole_pairs;

  pmsm_current_ref_init(&foc->current_ref);
  pmsm_current_controller_init(&foc->controller);
  space_vector_pwm_init(&foc->svm);

  // VDC filter
  foc->vdc_filter.sample_time = foc->sample_time;
  foc->vdc_filter.time_constant = FOC_VDC_FILTER_TIME_CONST;
  low_pass_filter_init(&foc->vdc_filter);
}

void foc_step(foc_t *foc) {
  // measurements
  float electrical_speed = foc->pole_pairs * foc->wSense;
  float electrical_angle =
      fmodf(foc->pole_pairs * foc->mech_rotor_angle, TWO_PI);

  // filter DC bus voltage
  foc->vdc_filter.x = foc->vdcSense;
  low_pass_filter_step(&foc->vdc_filter);

  foc->current_ref.torque_ref = foc->torque_ref;
  foc->current_ref.mechanical_speed = foc->wSense;
  foc->current_ref.electrical_speed = electrical_speed;
  foc->current_ref.Vdc = foc->vdcSense;
  pmsm_current_ref_step(&foc->current_ref);

  foc->controller.idq_ref = foc->current_ref.idq_ref;
  foc->controller.iabcMeas = foc->iabcSense;
  foc->controller.electrical_angle = electrical_angle;
  foc->controller.speed = electrical_speed;
  foc->controller.Vdc = foc->vdcSense;
  pmsm_current_controller_step(&foc->controller);

  three_phase_voltage_t vabc = {0};
  inverse_park_transform(&vabc, &foc->controller.vdq_out, electrical_angle);

  float v_alpha, v_beta;
  clarke_transform(&vabc, &v_alpha, &v_beta);

  space_vector_pwm_run(&foc->svm, v_alpha, v_beta);

  // outputs
  foc->svm = foc->svm;
  foc->vdq = foc->controller.vdq_out;
  foc->idq_ref = foc->controller.idq_ref;
  foc->torque_ref_saturation = foc->current_ref.torque_ref_saturation;
  foc->torque_limit = foc->current_ref.torque_limit;
  foc->torque_estimate = 0.0f; // TODO
  foc->rpm_base = 0.0f;        // TODO
}

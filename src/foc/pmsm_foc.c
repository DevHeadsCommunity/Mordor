#include "pmsm_foc.h"

#include <math.h>
#include <string.h>

void foc_init(foc_t *foc) {
  // initial values
  foc->torque_ref = 0.0f;
  foc->wSense = 0.0f;
  foc->thetaSense = 0.0f;
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
  spwmInit(&foc->svm);

  // VDC filter
  foc->vdc_filter.sample_time = foc->sample_time;
  foc->vdc_filter.time_constant = FOC_VDC_FILTER_TIME_CONST;
  low_pass_filter_init(&foc->vdc_filter);
}

void foc_step(foc_t *foc) {
  // measurements
  float wElec = foc->pole_pairs * foc->wSense;
  float theta_e = fmodf(foc->pole_pairs * foc->thetaSense, TWO_PI);

  // filter DC bus voltage
  foc->vdc_filter.x = foc->vdcSense;
  low_pass_filter_step(&foc->vdc_filter);

  foc->current_ref.torque_ref = foc->torque_ref;
  foc->current_ref.wMech = foc->wSense;
  foc->current_ref.wElec = wElec;
  foc->current_ref.Vdc = foc->vdcSense;
  pmsm_current_ref_step(&foc->current_ref);

  foc->controller.idq_ref = foc->current_ref.idq_ref;
  foc->controller.iabcMeas = foc->iabcSense;
  foc->controller.theta_e = theta_e;
  foc->controller.we = wElec;
  foc->controller.Vdc = foc->vdcSense;
  pmsm_current_controller_step(&foc->controller);

  // TODO: turn this into space vector pwm
  /*******************************************/
  /* foc->spwm.Vdq = foc->controller.vdq_out; */
  /* foc->spwm.theta_e = theta_e;            */
  /* foc->spwm.Vdc = foc->vdcSense;          */
  /* spwmStep(&foc->spwm);                   */
  /*******************************************/

  // outputs
  foc->svm = foc->svm;
  foc->vdq = foc->controller.vdq_out;
  foc->idq_ref = foc->controller.idq_ref;
  foc->torque_ref_saturation = foc->current_ref.torque_ref_saturation;
  foc->torque_limit = foc->current_ref.torque_limit;
  foc->torque_estimate = 0.0f; // TODO
  foc->rpm_base = 0.0f;        // TODO
}

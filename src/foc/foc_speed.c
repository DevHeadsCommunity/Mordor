#include "foc_speed.h"
#include "foc.h"

#include <string.h>

void foc_speed_init(foc_speed_t *foc) {
  // initial values
  foc->rpm_requested = 0.0f;
  foc->rpm_measured = 0.0f;
  foc->mech_rotor_velocity = 0.0f;
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
  foc->foc.sample_time = foc->sample_time;
  foc->foc.current_ref.max_torque = foc->max_torque;
  foc->pi_speed.sample_time = foc->sample_time;

  // constant params
  foc->pi_speed.upper_limit = foc->max_torque;
  foc->pi_speed.lower_limit = -foc->max_torque;

  foc_init(&foc->foc);
  pi_init(&foc->pi_speed);
}

void foc_speed_step(foc_speed_t *foc) {
  pi_t *pi_speed = &foc->pi_speed;
  pi_speed->setpoint = foc->rpm_requested;
  pi_speed->measurement = foc->rpm_measured;
  pi_step(pi_speed);

  float torque_ref = 0.0f;
  if (true == foc->torque_enable) {
    torque_ref = pi_speed->output;
  }

  foc->foc.torque_ref = torque_ref;
  foc->foc.current = foc->iabcSense;
  foc->foc.mech_velocity = foc->mech_rotor_velocity;
  foc->foc.mech_rotor_angle = foc->mech_rotor_angle;
  foc->foc.voltage_supply = foc->vdcSense;
  foc_step(&foc->foc);

  foc->svm = foc->foc.svm;
  foc->vdq = foc->foc.vdq;
  foc->idq_ref = foc->foc.idq_ref;
  foc->torque_ref_saturation = foc->foc.torque_ref_saturation;
  foc->torque_limit = foc->foc.torque_limit;
  foc->torque_estimate = foc->foc.torque_estimate;
  foc->rpm_base = foc->foc.rpm_base;
}

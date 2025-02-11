#include <zephyr/sys/util.h>

#include "pmsm_current_controller.h"

#include "transforms.h"
#include "types.h"

#include <math.h>

void pmsm_current_controller_init(pmsm_current_controller_t *controller) {
  controller->idq_ref.id = 0.0f;
  controller->idq_ref.iq = 0.0f;
  controller->electrical_angle = 0.0f;
  controller->speed = 0.0f;
  controller->Vdc = 0.0f;

  controller->vdq_out.vd = 0.0f;
  controller->vdq_out.vq = 0.0f;

  controller->pi_id.sample_time =
      controller->sample_time; // make sure these are consistent
  controller->pi_iq.sample_time = controller->sample_time;

  pi_init(&controller->pi_id);
  pi_init(&controller->pi_iq);
}

void pmsm_current_controller_step(pmsm_current_controller_t *controller) {
  direct_quadrature_current_t idqMeas;
  park_transform(&idqMeas, &controller->iabcMeas, controller->electrical_angle);

  float max_phase_voltage = controller->Vdc * SQRT3;

  // D axis PI controller
  pi_t *pi_id = &controller->pi_id;
  pi_id->upper_limit = max_phase_voltage;
  pi_id->lower_limit = -max_phase_voltage;
  pi_id->setpoint = controller->idq_ref.id;
  pi_id->measurement = idqMeas.id;
  pi_step(pi_id);
  float vd = pi_id->output;

  // Q axis PI controller
  pi_t *pi_iq = &controller->pi_iq;
  pi_iq->upper_limit = max_phase_voltage;
  pi_iq->lower_limit = -max_phase_voltage;
  pi_iq->setpoint = controller->idq_ref.iq;
  pi_iq->measurement = idqMeas.iq;
  pi_step(pi_iq);
  float vq = pi_iq->output;

  // voltage limiter
  // Don't need to saturate vq - the PI controller already does this
  float vdLim = sqrtf(max_phase_voltage * max_phase_voltage - vq * vq);
  float vdSat = CLAMP(vd, -vdLim, vdLim);

  controller->vdq_out.vd = vdSat;
  controller->vdq_out.vq = vq; // saturated by PI controller
}

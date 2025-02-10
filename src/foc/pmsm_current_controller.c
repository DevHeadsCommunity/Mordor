#include "pmsm_current_controller.h"

#include "transforms.h"
#include "types.h"

#include <math.h>

void pmsm_current_controller_init(pmsm_current_controller_t *controller) {
  controller->idq_ref.id = 0.0f;
  controller->idq_ref.iq = 0.0f;
  controller->theta_e = 0.0f;
  controller->we = 0.0f;
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
  park_transform(&idqMeas, &controller->iabcMeas, controller->theta_e);

  float vphMax = controller->Vdc * SQRT3;

  // D axis PI controller
  pi_t *pi_id = &controller->pi_id;
  pi_id->upper_limit = vphMax;
  pi_id->lower_limit = -vphMax;
  pi_id->setpoint = controller->idq_ref.id;
  pi_id->measurement = idqMeas.id;
  pi_step(pi_id);
  float vd = pi_id->output;

  // Q axis PI controller
  pi_t *pi_iq = &controller->pi_iq;
  pi_iq->upper_limit = vphMax;
  pi_iq->lower_limit = -vphMax;
  pi_iq->setpoint = controller->idq_ref.iq;
  pi_iq->measurement = idqMeas.iq;
  pi_step(pi_iq);
  float vq = pi_iq->output;

  // TODO feedforward control

  // voltage limiter
  // Don't need to saturate vq - the PI controller already does this
  float vdLim = sqrtf(vphMax * vphMax - vq * vq);
  float vdSat = sat(vd, -vdLim, vdLim);

  controller->vdq_out.vd = vdSat;
  controller->vdq_out.vq = vq; // saturated by PI controller
}

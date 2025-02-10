#include "pmsm_current_ref.h"

#include <math.h>

void pmsm_current_ref_init(pmsm_current_ref_t *current_ref) {
  current_ref->torque_ref = 0.0f;
  current_ref->wMech = 0.0f;
  current_ref->wElec = 0.0f;
  current_ref->Vdc = 0.0f;

  current_ref->idq_ref.id = 0.0f;
  current_ref->idq_ref.iq = 0.0f;
  current_ref->torque_ref_saturation = 0.0f;
  current_ref->torque_limit = 0.0f;

  current_ref->weBase = current_ref->pole_pairs * current_ref->wBase;
}

void pmsm_current_ref_step(pmsm_current_ref_t *current_ref) {
  float speed =
      fmaxf(fabsf(current_ref->wMech), current_ref->max_power / current_ref->max_torque);
  current_ref->torque_limit = (current_ref->Vdc / current_ref->nominal_voltage) *
                      fminf(current_ref->max_torque, current_ref->max_power / speed);
  current_ref->torque_ref_saturation =
      sat(current_ref->torque_ref, -(current_ref->torque_limit), current_ref->torque_limit);

  if (current_ref->wMech < current_ref->wBase) {
    // zero d-axis control (ZDAC)
    current_ref->idq_ref.id = 0.0f;

    current_ref->idq_ref.iq =
        2.0f * (current_ref->torque_ref_saturation) /
        (3.0f * current_ref->pole_pairs * current_ref->flux_link);
    current_ref->idq_ref.iq =
        sat(current_ref->idq_ref.iq, -current_ref->max_current, current_ref->max_current);
  } else {
    // Field weakening
    float id_fw = 2.0f * (current_ref->weBase - current_ref->wElec) *
                  current_ref->flux_link / (current_ref->wElec * current_ref->Ld);
    float id = fmaxf(id_fw, -current_ref->max_current);

    float iq_fw = 2.0f * (current_ref->torque_ref_saturation) /
                  (3.0f * current_ref->pole_pairs * current_ref->flux_link);
    float iq_lim = sqrtf(current_ref->max_current * current_ref->max_current - id * id);
    float iq = sat(iq_fw, -iq_lim, iq_lim);

    current_ref->idq_ref.id = id;
    current_ref->idq_ref.iq = iq;
  }
}

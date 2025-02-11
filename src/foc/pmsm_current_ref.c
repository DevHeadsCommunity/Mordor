#include <zephyr/sys/util.h>

#include "pmsm_current_ref.h"

#include <math.h>

void pmsm_current_ref_init(pmsm_current_ref_t *current_ref) {
  current_ref->torque_ref = 0.0f;
  current_ref->mechanical_speed = 0.0f;
  current_ref->electrical_speed = 0.0f;
  current_ref->Vdc = 0.0f;

  current_ref->idq_ref.id = 0.0f;
  current_ref->idq_ref.iq = 0.0f;
  current_ref->torque_ref_saturation = 0.0f;
  current_ref->torque_limit = 0.0f;

  current_ref->electrical_base_speed = current_ref->pole_pairs * current_ref->base_speed;
}

void pmsm_current_ref_step(pmsm_current_ref_t *current_ref) {
  float speed = fmaxf(fabsf(current_ref->mechanical_speed),
                      current_ref->max_power / current_ref->max_torque);
  current_ref->torque_limit =
      (current_ref->Vdc / current_ref->nominal_voltage) *
      fminf(current_ref->max_torque, current_ref->max_power / speed);
  current_ref->torque_ref_saturation =
      CLAMP(current_ref->torque_ref, -(current_ref->torque_limit),
            current_ref->torque_limit);

  if (current_ref->mechanical_speed < current_ref->base_speed) {
    // zero d-axis control (ZDAC)
    current_ref->idq_ref.id = 0.0f;

    current_ref->idq_ref.iq =
        2.0f * (current_ref->torque_ref_saturation) /
        (3.0f * current_ref->pole_pairs * current_ref->flux_linkage);
    current_ref->idq_ref.iq =
        CLAMP(current_ref->idq_ref.iq, -current_ref->max_current,
              current_ref->max_current);
  } else {
    // Field weakening
    float id_fw = 2.0f * (current_ref->electrical_base_speed - current_ref->electrical_speed) *
                  current_ref->flux_linkage /
                  (current_ref->electrical_speed * current_ref->Ld);
    float id = fmaxf(id_fw, -current_ref->max_current);

    float iq_fw = 2.0f * (current_ref->torque_ref_saturation) /
                  (3.0f * current_ref->pole_pairs * current_ref->flux_linkage);
    float iq_lim =
        sqrtf(current_ref->max_current * current_ref->max_current - id * id);
    float iq = CLAMP(iq_fw, -iq_lim, iq_lim);

    current_ref->idq_ref.id = id;
    current_ref->idq_ref.iq = iq;
  }
}

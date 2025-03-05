#include "transforms.h"

#include <math.h>

void park_transform(direct_quadrature_current_t *idq,
                    const three_phase_current_t *iabc,
                    const float electrical_angle) {
  idq->id = sinf(electrical_angle) * iabc->ia +
            sinf(electrical_angle - TWO_PI_3) * iabc->ib +
            sinf(electrical_angle + TWO_PI_3) * iabc->ic;
  idq->id *= 0.666667F;

  idq->iq = cosf(electrical_angle) * iabc->ia +
            cosf(electrical_angle - TWO_PI_3) * iabc->ib +
            cosf(electrical_angle + TWO_PI_3) * iabc->ic;
  idq->iq *= 0.666667F;
}

void inverse_park_transform(three_phase_voltage_t *Vabc,
                            const direct_quadrature_voltage_t *vdq,
                            const float electrical_angle) {
  Vabc->va =
      sinf(electrical_angle) * vdq->vd + cosf(electrical_angle) * vdq->vq;

  Vabc->vb = sinf(electrical_angle - TWO_PI_3) * vdq->vd +
             cosf(electrical_angle - TWO_PI_3) * vdq->vq;

  Vabc->vc = sinf(electrical_angle + TWO_PI_3) * vdq->vd +
             cosf(electrical_angle + TWO_PI_3) * vdq->vq;
}

void clarke_transform(const three_phase_voltage_t *Vabc, float *v_alpha,
                      float *v_beta) {
  *v_alpha = (2 / 3) * (Vabc->va - 0.5f * Vabc->vb - 0.5f * Vabc->vc);
  *v_beta = (2 / 3) * (SQRT3_2 * Vabc->vb - SQRT3_2 * Vabc->vc);
}

float normalize_angle(float angle) {
  float ang = fmodf(angle, TWO_PI);
  return ang >= 0 ? ang : (ang + TWO_PI);
}

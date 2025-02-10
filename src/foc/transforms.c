#include "transforms.h"

#include <math.h>

void park_transform(direct_quadrature_current_t *idq,
                    const three_phase_current_t *iabc, const float theta_e) {
  idq->id = sinf(theta_e) * iabc->ia + sinf(theta_e - TWO_PI_3) * iabc->ib +
            sinf(theta_e + TWO_PI_3) * iabc->ic;
  idq->id *= 0.666667F;

  idq->iq = cosf(theta_e) * iabc->ia + cosf(theta_e - TWO_PI_3) * iabc->ib +
            cosf(theta_e + TWO_PI_3) * iabc->ic;
  idq->iq *= 0.666667F;
}

void inverse_park_transform(three_phase_voltage_t *Vabc,
                            const direct_quadrature_voltage_t *vdq,
                            const float theta_e) {
  Vabc->va = sin(theta_e) * vdq->vd + cos(theta_e) * vdq->vq;

  Vabc->vb =
      sin(theta_e - TWO_PI_3) * vdq->vd + cos(theta_e - TWO_PI_3) * vdq->vq;

  Vabc->vc =
      sin(theta_e + TWO_PI_3) * vdq->vd + cos(theta_e + TWO_PI_3) * vdq->vq;
}

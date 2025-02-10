#ifndef __TRANSFORMS_H__
#define __TRANSFORMS_H__

#include "types.h"

void park_transform(direct_quadrature_current_t *idq,
                    const three_phase_current_t *iabc, const float theta_e);

void inverse_park_transform(three_phase_voltage_t *Vabc,
                            const direct_quadrature_voltage_t *vdq,
                            const float theta_e);

#endif

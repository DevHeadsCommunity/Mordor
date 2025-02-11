#ifndef __PMSM_CURRENT_CONTROLLER__
#define __PMSM_CURRENT_CONTROLLER__

#include "pid.h"
#include "types.h"

typedef struct {
  direct_quadrature_current_t idq_ref;
  three_phase_current_t iabcMeas;
  float electrical_angle;
  float speed;
  float Vdc;

  direct_quadrature_voltage_t vdq_out;

  float sample_time;

  // Sub-modules
  pi_t pi_iq;
  pi_t pi_id;
} pmsm_current_controller_t;

/**
 * Initializes the internal fields.
 * Does not update the parameters - these must be manually set.
 * controller->pi parameters must also be set.
 */
void pmsm_current_controller_init(pmsm_current_controller_t *controller);

/**
 * Performs an update of the PMSM Current Controller
 * @param controller data used for input, output, and parameters
 */
void pmsm_current_controller_step(pmsm_current_controller_t *controller);

#endif

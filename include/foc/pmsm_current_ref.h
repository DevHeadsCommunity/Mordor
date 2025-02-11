#ifndef __PMSM_CURRENT_REF_H__
#define __PMSM_CURRENT_REF_H__

#include "types.h"

#include <stdint.h>

typedef struct {
  // Inputs
  float torque_ref;
  float mechanical_speed;
  float electrical_speed;
  float Vdc; // DC bus voltage [V]

  // Outputs
  direct_quadrature_current_t idq_ref;
  float torque_ref_saturation;
  float torque_limit;

  // Parameters
  float nominal_voltage;
  float max_power;
  float max_torque;
  float max_current;
  float base_speed;
  uint16_t pole_pairs;
  float flux_linkage;
  float Ld; // D axis inductance [H]

  // Internal variables
  float electrical_base_speed; // Electrical base speed
} pmsm_current_ref_t;

/**
 * Initializes the internal fields.
 */
void pmsm_current_ref_init(pmsm_current_ref_t *current_ref);

/**
 * Performs an update of the PMSM Current Reference generator
 * @param current_ref Current reference data
 */
void pmsm_current_ref_step(pmsm_current_ref_t *current_ref);

#endif

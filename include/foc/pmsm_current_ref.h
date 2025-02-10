#ifndef __PMSM_CURRENT_REF_H__
#define __PMSM_CURRENT_REF_H__

#include "types.h"

#include <stdint.h>

typedef struct {
  // Inputs
  float torque_ref; // Reference torque [Nm]
  float wMech;      // Mechanical speed [rad/s]
  float wElec;      // Electrical speed [rad/s]
  float Vdc;        // DC bus voltage [V]

  // Outputs
  direct_quadrature_current_t idq_ref; // Output idq reference current [A]
  float torque_ref_saturation;                      // Troque reference (saturated) [Nm]
  float torque_limit;                         // Torque limit [Nm]

  // Parameters
  float nominal_voltage;
  float max_power;     // Max power [W]
  float max_torque;    // Max torque [Nm]
  float max_current;   // Max current [A]
  float wBase;         // Base speed [rad/s]
  uint16_t pole_pairs; // Pole pairs
  float flux_link;     // Flux linkage [Webers]
  float Ld;            // D axis inductance [H]

  // Internal variables
  float weBase; // Electrical base speed
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

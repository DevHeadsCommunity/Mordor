#include <zephyr/sys/util.h>

#include "svpwm.h"

#include <math.h>
#include <stdint.h>

static uint8_t get_sector(float a, float b, float c) {
  uint8_t sector = 0U;

  if (c < 0.0f) {
    if (a < 0.0f) {
      sector = 2U;
    } else {
      if (b < 0.0f) {
        sector = 6U;
      } else {
        sector = 1U;
      }
    }
  } else {
    if (a < 0.0f) {
      if (b <= 0.0f) {
        sector = 4U;
      } else {
        sector = 3U;
      }
    } else {
      sector = 5U;
    }
  }

  return sector;
}

void space_vector_pwm_init(svpwm_t *svm) {
  svm->sector = 0U;
  svm->duties.a = 0.0f;
  svm->duties.b = 0.0f;
  svm->duties.c = 0.0f;
  svm->d_min = 0.0f;
  svm->d_max = 1.0f;
}

int space_vector_pwm_run(svpwm_t *svm, const float v_alpha,
                         const float v_beta) {

  float a, b, c, mod, x, y, z;
  float alpha = v_alpha, beta = v_beta;

  /* limit maximum amplitude to avoid distortions */
  mod = sqrtf(v_alpha * v_alpha + v_beta * v_beta);

  if (mod > SQRT3_2) {
    alpha = v_alpha / mod * SQRT3_2;
    beta = v_beta / mod * SQRT3_2;
  }

  a = alpha - SQRT3 * beta;
  b = SQRT3 * beta;
  c = -(a + b);

  svm->sector = get_sector(a, b, c);
  switch (svm->sector) {
  case 1U:
    x = a;
    y = b;
    z = 1.0f - (x + y);

    svm->duties.a = x + y + z * 0.5f;
    svm->duties.b = y + z * 0.5f;
    svm->duties.c = z * 0.5f;
    break;
  case 2U:
    x = -c;
    y = -a;
    z = 1.0f - (x + y);

    svm->duties.a = x + z * 0.5f;
    svm->duties.b = x + y + z * 0.5f;
    svm->duties.c = z * 0.5f;
    break;
  case 3U:
    x = b;
    y = c;
    z = 1.0f - (x + y);

    svm->duties.a = z * 0.5f;
    svm->duties.b = x + y + z * 0.5f;
    svm->duties.c = y + z * 0.5f;
    break;
  case 4U:
    x = -a;
    y = -b;
    z = 1.0f - (x + y);

    svm->duties.a = z * 0.5f;
    svm->duties.b = x + z * 0.5f;
    svm->duties.c = x + y + z * 0.5f;
    break;
  case 5U:
    x = c;
    y = a;
    z = 1.0f - (x + y);

    svm->duties.a = y + z * 0.5f;
    svm->duties.b = z * 0.5f;
    svm->duties.c = x + y + z * 0.5f;
    break;
  case 6U:
    x = -b;
    y = -c;
    z = 1.0f - (x + y);

    svm->duties.a = x + y + z * 0.5f;
    svm->duties.b = z * 0.5f;
    svm->duties.c = x + z * 0.5f;
    break;
  default:
    break;
  }

  svm->duties.a = CLAMP(svm->duties.a, svm->d_min, svm->d_max);
  svm->duties.b = CLAMP(svm->duties.b, svm->d_min, svm->d_max);
  svm->duties.c = CLAMP(svm->duties.c, svm->d_min, svm->d_max);
  return 0;
}

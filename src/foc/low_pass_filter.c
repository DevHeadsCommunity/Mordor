#include "low_pass_filter.h"

void low_pass_filter_init(low_pass_filter_t *lpf) {
  lpf->x = 0.0f;
  lpf->y = 0.0f;
  lpf->xPrev = 0.0f;
  lpf->yPrev = 0.0f;

  // pre-compute gain factors
  lpf->alpha = lpf->sample_time / (lpf->time_constant + lpf->sample_time);
  lpf->oneMinusAlpha = 1.0f - lpf->alpha;
}

void low_pass_filter_step(low_pass_filter_t *lpf) {
  // calculate the 1st order low pass filter difference equation
  lpf->y = lpf->alpha * lpf->x + lpf->oneMinusAlpha * lpf->yPrev;

  // store memory elements of filter
  lpf->xPrev = lpf->x;
  lpf->yPrev = lpf->y;
}

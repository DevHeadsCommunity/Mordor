#ifndef __LOW_PASS_FILTER_H__
#define __LOW_PASS_FILTER_H__

typedef struct {
  float x;

  float y;

  float sample_time;
  float time_constant;

  // Internal variables
  float alpha;
  float oneMinusAlpha;
  float xPrev;
  float yPrev;
} low_pass_filter_t;

void low_pass_filter_init(low_pass_filter_t *lpf);

void low_pass_filter_step(low_pass_filter_t *lpf);

#endif

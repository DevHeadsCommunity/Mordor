#ifndef __TYPES_H__
#define __TYPES_H__

#define SQRT3 1.732051f
#define SQRT3_2 (SQRT3 / 2.0f)
#define TWO_PI 6.283185f
#define TWO_PI_3 2.094395f
#define TWO_THIRDS 0.666667f

typedef struct {
  float va;
  float vb;
  float vc;
} three_phase_voltage_t;

typedef struct {
  float ia;
  float ib;
  float ic;
} three_phase_current_t;

typedef struct {
  float vd;
  float vq;
} direct_quadrature_voltage_t;

typedef struct {
  float id;
  float iq;
} direct_quadrature_current_t;

#endif

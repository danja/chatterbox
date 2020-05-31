#include <Softclip.h>

Softclip::Softclip(){}

// TODO these can be static?

/*
   tanh approximation
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float Softclip::process(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}

float Softclip::clamp(float x, float lowerlimit, float upperlimit) {
  if (x < lowerlimit)
    x = lowerlimit;
  if (x > upperlimit)
    x = upperlimit;
  return x;
}
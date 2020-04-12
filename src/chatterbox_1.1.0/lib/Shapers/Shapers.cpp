/*
   tanh approximation
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float softClip(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}

float clamp(float x, float lowerlimit, float upperlimit) {
  if (x < lowerlimit)
    x = lowerlimit;
  if (x > upperlimit)
    x = upperlimit;
  return x;
}

/* from https://en.wikipedia.org/wiki/Smoothstep */
float smootherstep(float edge0, float edge1, float x) {
  // Scale, and clamp x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  // Evaluate polynomial
  return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

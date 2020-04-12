#pragma once

class Shapers {

public:
  Shapers();
  static float softClip(float x);

  static float clamp(float x, float lowerlimit, float upperlimit);
  static float smootherstep(float edge0, float edge1, float x);
};

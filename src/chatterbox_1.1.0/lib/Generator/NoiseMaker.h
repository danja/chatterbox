#pragma once

class NoiseMaker {

public:
  NoiseMaker();
  float pink(float white);
  float logistic();
  float stretchedNoise();
private:
  // pink
  float b0 = 0.0f;
  float b1 = 0.0f;
  float b2 = 0.0f;
  // logistic
  float lx = 0.5f;
  float logisticK = 3.5f;
  float x;
  int logisticLoops = 1;
  // stretchedNoise
  int stretch = 64;
  int is = 0;
  float stretchStartLevel = 0.5f;
  float stretchEndLevel = 0.5f;
  float stretchStep = 0.0f;
  float currentX = 0;
};

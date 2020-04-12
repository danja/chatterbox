
#include <Arduino.h> // for random()
#include <Shapers.h>
#include <NoiseMaker.h>

NoiseMaker::NoiseMaker(){}

// https://web.archive.org/web/20150226171912/http://home.earthlink.net/~ltrammell/tech/newpink.htm
// http://www.firstpr.com.au/dsp/pink-noise/


float NoiseMaker::pink(float white) {
  //float white = (float)random(0, 32767) / 32768.0f; // TODO check white is +/- 1

  b0 = Shapers::softClip(0.99765f * b0 + white * 0.0990460f);
  b1 = Shapers::softClip(0.96300f * b1 + white * 0.2965164f);
  b2 = Shapers::softClip(0.57000f * b2 + white * 1.0526913f);
  return Shapers::softClip(b0 + b1 + b2 + white * 0.1848f);
}

// Logistic map chaotic signal


float NoiseMaker::logistic() {
  for (int i = 0; i < logisticLoops; i++) {
    x = lx * logisticK * (1 - lx);
    if (x > 1) x = 0.5;
    lx = x;
  }
  /*
    Serial.println();
    Serial.println("logisticK");
    Serial.println(logisticK, DEC);
    Serial.println(x, DEC);
  */
  return x;
}



float NoiseMaker::stretchedNoise() { // TODO refactor me
  if (is < stretch) {
    currentX = currentX + stretchStep;

    is++;
    //smootherstep(-0.99f, 0.99f, raw);
    //  return current;
    return 2.0f * Shapers::smootherstep(0.0f, 0.99f, currentX) - 1;
  }
  is = 0;
  stretchStartLevel = stretchEndLevel;
  currentX = stretchStartLevel;
  stretchEndLevel = (float)random(0, 32767) / 32768.0f;
  stretchStep = (stretchEndLevel - stretchStartLevel) / (float)stretch;
  return 2.0f * Shapers::smootherstep(0.0f, 0.99f, stretchStartLevel) - 1;
  //  return startLevel;
}

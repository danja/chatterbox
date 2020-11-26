#ifndef _SINWAVETABLE_H_
#define _SINWAVETABLE_H_

#include <Arduino.h>
#include <Wavetable.h>

const float sinScale = 2.0f * PI / tablesize;

class SineWavetable : public Wavetable
{
public:
  SineWavetable();
  ~SineWavetable();
  void init();
};

#endif
#include <array>
#include <Wavetable.h>
#include <Arduino.h>

const float sinScale = 2.0f * PI / tablesize;

class SineWavetable : public Wavetable
{
public:
  SineWavetable();
  ~SineWavetable();
  void init();
  const float get(const float hop);
  int pointer = 0;
};

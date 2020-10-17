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

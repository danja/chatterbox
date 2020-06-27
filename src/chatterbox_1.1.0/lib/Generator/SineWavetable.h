#include <array>
#include <Wavetable.h>
#include <Arduino.h>

const float sinScale = 2.0f * PI / tablesize;

class SineWavetable : public Wavetable
{
public:
    SineWavetable();
   // void init();
    const float &get(const int i);
  //  static float sineWavetable[TABLESIZE];
  static std::array<float, TABLESIZE> sineWavetable;
};

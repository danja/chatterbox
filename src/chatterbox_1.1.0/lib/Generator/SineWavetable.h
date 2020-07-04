#include <array>
#include <Wavetable.h>
#include <Arduino.h>

const float sinScale = 2.0f * PI / tablesize;

class SineWavetable : public Wavetable
{
public:
  SineWavetable();
  ~SineWavetable();
  // void init();
  const float &get(const int i);
  //float sineWavetable[TABLESIZE];
   static float sineWavetable[TABLESIZE];
  // static std::array<float, TABLESIZE> sineWavetable;
  // std::array<float, TABLESIZE> sineWavetable;
//private:
  //float *sineWavetable;
};

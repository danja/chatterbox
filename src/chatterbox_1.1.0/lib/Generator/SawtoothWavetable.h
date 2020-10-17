#include <Arduino.h>
#include <Wavetable.h>

const float sawScale = 2.0f / tablesize;

class SawtoothWavetable : public Wavetable
{
public:
   SawtoothWavetable();
   ~SawtoothWavetable();
   void init();
};

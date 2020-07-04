#include <array>
#include <Wavetable.h>
#include <Arduino.h>

// const float sinScale = 2.0f * PI / (float)TABLESIZE;

const float sawScale = 2.0f / tablesize;

class SawtoothWavetable : public Wavetable
{
public:
    SawtoothWavetable();
    ~SawtoothWavetable();
    // void init();
    const float &get(const int i);
 static float sawtoothWavetable[TABLESIZE];
    // static std::array<float, TABLESIZE> sawtoothWavetable;
// private:
   // float *sawtoothWavetable;
};

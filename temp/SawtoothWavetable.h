#include <Wavetable.h>
#include <Arduino.h>

// const float sinScale = 2.0f * PI / (float)TABLESIZE;

const float sawScale = 2.0f / tablesize;

class SawtoothWavetable : public Wavetable
{
public:
    SawtoothWavetable();
    void init();
    float get(int i);
};

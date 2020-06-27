#include <Wavetable.h>
#include <Arduino.h>

const float sinScale = 2.0f * PI / tablesize;

class SineWavetable : public Wavetable
{
public:
    SineWavetable();
    void init();
    float get(int i);
};

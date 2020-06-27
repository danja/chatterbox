#include <SineWavetable.h>
#include <Arduino.h>

SineWavetable::SineWavetable()
{
}

void SineWavetable::init()
{
    for (unsigned int i = 0; i < tablesize; i++)
    {
        wavetable[i] = sin((float)i * sinScale);
    }
}

float SineWavetable::get(int i)
{
    return wavetable[i];
}
#include <SineWavetable.h>
#include <Arduino.h>


SineWavetable::SineWavetable()
{
}


void SineWavetable::init()
{
    for (unsigned int i = 0; i < TABLESIZE; i++)
    {
        wavetable[i] = sin((float)i * sinScale);
    }
}
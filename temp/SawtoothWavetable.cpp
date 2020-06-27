#include <SawtoothWavetable.h>
#include <Arduino.h>

SawtoothWavetable::SawtoothWavetable()
{
}

void SawtoothWavetable::init()
{
    for (unsigned int i = 0; i < tablesize; i++)
    {
    wavetable[i] = ((float)i * sawScale - 1);
    }
}

float SawtoothWavetable::get(int i)
{
    return wavetable[i];
}
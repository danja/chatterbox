#include <SawtoothWavetable.h>
#include <Arduino.h>

// float SawtoothWavetable::sawtoothWavetable[TABLESIZE];
std::array<float, TABLESIZE> SawtoothWavetable::sawtoothWavetable;

SawtoothWavetable::SawtoothWavetable()
{
     for (unsigned int i = 0; i < tablesize; i++)
    {
    SawtoothWavetable::sawtoothWavetable[i] = ((float)i * sawScale - 1);
    }
}

/*
void SawtoothWavetable::init()
{
    for (unsigned int i = 0; i < tablesize; i++)
    {
    SawtoothWavetable::sawtoothWavetable[i] = ((float)i * sawScale - 1);
    }
}
*/

const float &SawtoothWavetable::get(const int i)
{
    return SawtoothWavetable::sawtoothWavetable[i];
}
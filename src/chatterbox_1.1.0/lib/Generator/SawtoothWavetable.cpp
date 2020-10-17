#include <Arduino.h>
#include <Wavetable.h>
#include <SawtoothWavetable.h>


SawtoothWavetable::SawtoothWavetable()
{
}       

SawtoothWavetable::~SawtoothWavetable()
{
    delete[] Wavetable::wavetable;
}

void SawtoothWavetable::init()
{
   for (unsigned int i = 0; i < tablesize; i++)
    {
        Wavetable::wavetable[i] = ((float)i * sawScale - 1);
    }
}

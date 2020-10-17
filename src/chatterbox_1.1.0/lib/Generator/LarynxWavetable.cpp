#include <Arduino.h>
#include <Wavetable.h>
#include <LarynxWavetable.h>


LarynxWavetable::LarynxWavetable()
{
}       

LarynxWavetable::~LarynxWavetable()
{
    delete[] Wavetable::wavetable;
}

void LarynxWavetable::init()
{
    /*
   for (unsigned int i = 0; i < tablesize; i++)
    {
        Wavetable::wavetable[i] = ((float)i * sawScale - 1);
    }
    */
}

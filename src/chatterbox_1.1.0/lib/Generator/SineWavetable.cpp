#include <Arduino.h>
#include <SineWavetable.h>


SineWavetable::SineWavetable()
{
}

SineWavetable::~SineWavetable()
{
  //  delete[] SineWavetable::wavetable;
   delete[] Wavetable::wavetable;
}

void SineWavetable::init()
{
    for (unsigned int i = 0; i < tablesize; i++) 
    {
        Wavetable::wavetable[i] = sin((float)i * sinScale * 0.9f);
    }
}


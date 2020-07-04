#include <SineWavetable.h>
#include <Arduino.h>

float SineWavetable::sineWavetable[TABLESIZE];
// std::array<float, TABLESIZE> SineWavetable::sineWavetable;

SineWavetable::SineWavetable()
{
     for (unsigned int i = 0; i < tablesize; i++)
    {
        SineWavetable::sineWavetable[i] = sin((float)i * sinScale);
    }
}

SineWavetable::~SineWavetable()
{
    delete[] SineWavetable::sineWavetable;
}

/*
void SineWavetable::init()
{
    for (unsigned int i = 0; i < tablesize; i++)
    {
        SineWavetable::sineWavetable[i] = sin((float)i * sinScale);
    }
}
*/

const float &SineWavetable::get(const int i)
{
     
    return sineWavetable[i];
    // return SineWavetable::sineWavetable[i];
}
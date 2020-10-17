#include <SineWavetable.h>
#include <Arduino.h>

// float SineWavetable::wavetable[2048]; // TABLESIZE
// std::array<float, TABLESIZE> SineWavetable::sineWavetable;

SineWavetable::SineWavetable()
{
}

SineWavetable::~SineWavetable()
{
    delete[] SineWavetable::wavetable;
}

void SineWavetable::init()
{
    for (unsigned int i = 0; i < 2048; i++) // tablesize
    {
        Wavetable::wavetable[i] = sin((float)i * sinScale);
    }
}

// float wavetable[], int size, float offset

const float SineWavetable::get(float hop) // &SineWavetable
{
    pointer = pointer + hop;

    if (pointer < 0)
        pointer = 0;

    if (pointer >= tablesize)
        pointer = pointer - tablesize;

    float err = 0;
    float flr = floor(pointer);
    // interpolate between neighbouring values
    err = pointer - flr;

    int lower = (int)flr;
    int upper = ((int)ceil(pointer)) % 2048; // TABLESIZE
    float f = Wavetable::wavetable[lower] * err + Wavetable::wavetable[upper] * (1 - err);
    return f;
}
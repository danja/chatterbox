#include <Arduino.h>
#include <SineWavetable.h>

const float Wavetable::get(float hop) // &SineWavetable
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
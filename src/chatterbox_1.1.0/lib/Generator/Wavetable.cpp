#include <Arduino.h>
#include <Wavetable.h>

const float Wavetable::get(float hop) // &SineWavetable
{
    pointer = pointer + hop;

    if (pointer < 0.0f)
        pointer = tablesize - pointer;

    if (pointer >= tablesize)
        pointer = pointer - tablesize;

    float flr = floor(pointer);
    // interpolate between neighbouring values

    float err = pointer - flr;

    int lower = (int)flr;
    int upper = ((int)ceil(pointer)) % TABLESIZE; // TABLESIZE 2048
    float f = Wavetable::wavetable[lower] * err + Wavetable::wavetable[upper] * (1 - err);
    return f;
}
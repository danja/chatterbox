#include <Arduino.h>
#include <Wavetable.h>
#include <LarynxWavetable.h>
#include <Patchbay.h>
#include <Softclip.h>

LarynxWavetable::LarynxWavetable()
{
}       

LarynxWavetable::~LarynxWavetable()
{
    delete[] Wavetable::wavetable;
}

void LarynxWavetable::init(Patchbay patchbay)
{
    /*
   for (unsigned int i = 0; i < tablesize; i++)
    {
        Wavetable::wavetable[i] = ((float)i * sawScale - 1);
    }
    */
     float larynxPeak = 0.5f * (float)(patchbay.larynxSplit * patchbay.larynxSplit / tablesize); //  * larynxR * larynxR;

    for (unsigned int i = 0; i < larynxPeak; i++)
    { // up slope+
        Wavetable::wavetable[i] = 2.0f * (float)i / larynxPeak - 1.0f;
    }
    for (unsigned int i = larynxPeak; i < patchbay.larynxSplit; i++)
    { // down slope
        Wavetable::wavetable[i] = 1.0f - 2.0f * (i - larynxPeak) / (patchbay.larynxSplit - larynxPeak);
    }
    for (unsigned int i = patchbay.larynxSplit; i < TABLESIZE; i++)
    { // flat section __
        Wavetable::wavetable[i] = -1.0f;
    }
    for (unsigned int i = 0; i < TABLESIZE; i++)
    {
        Wavetable::wavetable[i] = softClip.process(Wavetable::wavetable[i]);
    }
}

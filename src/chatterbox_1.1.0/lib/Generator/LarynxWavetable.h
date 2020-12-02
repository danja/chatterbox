#ifndef _LARYNXWAVETABLE_H_
#define _LARYNXWAVETABLE_H_

#include <Arduino.h>
#include <Wavetable.h>
#include <Patchbay.h>
#include <Softclip.h>

class LarynxWavetable : public Wavetable
{
public:
   LarynxWavetable();
   ~LarynxWavetable();
   void init(Patchbay patchbay);
   //  void init();
// static Softclip softClip;
};

#endif
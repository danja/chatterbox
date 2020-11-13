#include <Arduino.h>
#include <Wavetable.h>


class LarynxWavetable : public Wavetable
{
public:
   LarynxWavetable();
   ~LarynxWavetable();
   void init(Patchbay patchbay);
   static Softclip softClip;
};

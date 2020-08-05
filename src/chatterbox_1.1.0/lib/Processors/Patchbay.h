#ifndef Patchbay_h
#define Patchbay_h

#include <WString.h>
#include <Processor.h>
#include <SVF.h>

class Patchbay : public Processor
{

public:
    Patchbay();

    float process(const float x);
    void setModules(SVF const& svf1);

    SVF svf1;
    SVF svf2;
    SVF svf3;

    SVF fricative1;
    SVF fricative2;
    SVF fricative3;

    SVF nasalLP;
    SVF nasalFixedBP;
    SVF nasalFixedNotch;

    SVF sing1;
    SVF sing2;

    float pitch = 0;
    float larynxPart = 0;

    float f1f = 0;
    float f2f = 0;
    float f3f = 0;
    float f3q = 0;

    float emphasisGain = 1.0f;

    float larynxRatio;
    float sineRatio;
    float sawtoothRatio;

    float growl;

    float attackTime;
    float attackStep;
    float decayTime;
    float decayStep;

// private:
  //  SVF svf1_;
};

#endif
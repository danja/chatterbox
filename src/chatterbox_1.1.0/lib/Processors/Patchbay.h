#ifndef Patchbay_h
#define Patchbay_h

#include <WString.h>
#include <Wavetable.h> // only for tablesize

#include <SVF.h>
#include <EventReceiver.h> // was "

// const int TABLESIZE = 2048;
//const float tablesize = 2048.0f;

class Patchbay : public EventReceiver
{
public:
    Patchbay();
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);

    float process(const float x);
    void setModules(SVF const &svf1);

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
    
    int larynxSplit = TABLESIZE / 2;
    int larynxSplitPrevious = TABLESIZE / 2;

    float larynxPart = 0;

    float f1f = 0;
    float f2f = 0;
    float f3f = 0;
    float f3q = 0;

    float voicedGain = 1;

    float emphasisGain = 1.0f;

    float larynxRatio;
    float sineRatio;
    float sawtoothRatio;

    float growl;

    float attackTime;
    float attackStep;
    float decayTime;
    float decayStep;

private:
    void listener(const EventType& type, const String& name, const float& value);
    //  SVF svf1_;
};

#endif
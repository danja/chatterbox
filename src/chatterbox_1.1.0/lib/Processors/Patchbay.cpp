#include <Patchbay.h>
#include <SVF.h>
#include <Arduino.h>
#include <dispatcher.hpp>

Patchbay::Patchbay() {
   Serial.println("Patchbay::Patchbay()");
}

void Patchbay::registerCallback(Dispatcher<EventType, String, float> &dispatcher)
{
    using namespace std::placeholders;
    dispatcher.addCallback(std::bind(&Patchbay::listener, this, _1, _2, _3));
}

void Patchbay::listener(EventType type, String name, float value)
{
    /*
    Serial.print("PB: ");
    Serial.print(name);
    Serial.print(" : ");
    Serial.println(value);
    */
    if (name == "pitch") {
        pitch = value;
        return;
    }
    if (name == "gain") {
        voicedGain = value;
        return;
    }
        if (name == "larynxSplit") {
        larynxSplit = value;
        return;
    }
    
}


///////////////////////////////////

void Patchbay::setModules(SVF const& svf1) {
    //    svf1_ = svf1;
}

float Patchbay::process(const float x)
{
    return 0;
}


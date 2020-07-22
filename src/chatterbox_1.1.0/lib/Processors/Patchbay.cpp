#include <Patchbay.h>
#include <SVF.h>
#include <Arduino.h>

Patchbay::Patchbay() {}

void Patchbay::setModules(SVF const& svf1){
    svf1_ = svf1;
}

float Patchbay::process(const float x)
{
    Serial.println("PB");
    Serial.println(x, DEC);
    float out = svf1_.process(x);
    Serial.println("PB out v");
    Serial.println(out, DEC);
    Serial.println("PB out ^");
    return out;
}


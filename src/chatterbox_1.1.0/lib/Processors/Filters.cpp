// NOT USED

#include <Arduino.h>
#include <Filters.h>
#include <SVF.h>

SVF Filters::svf3;

Filters::Filters() {}

const float Filters::process(const float x)
{
    /*
    if (name == "svf3")
    {
        return Filters::svf3.process(x);
    }
    return x;
*/
    return Filters::svf3.process(x);
}

void Filters::update(String name, float freq, float q, String typeString, float sampleRate)
{
    if (name == "svf3")
    {
        Filters::svf3.initParameters(freq, q, typeString, sampleRate);
       // Serial.println("svf3 update");
    }
}
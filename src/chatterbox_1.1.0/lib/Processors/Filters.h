#ifndef Filters_h
#define Filters_h

// #include <Arduino.h>
#include <WString.h>
#include <Processor.h>
#include <SVF.h>

class Filters : public Processor
{

public:
    Filters();
    void update(String name, float cutoff, float q, String type, float sampleRate, float gain);
    const float process(const float x);
   
private:
     static SVF svf3;
};

#endif
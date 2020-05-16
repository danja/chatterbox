#include <Processor.h>

#include <WString.h>
#include <Node.h>

Processor::Processor() {}

float Processor::process(float input)
{
    return input;
}

/* Yay! WString has an == operator
https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WString.h
*/

void Processor::boolParameter(String name, bool boolParameter)
{
    name_ = name;
    boolParameter_ = boolParameter;
}

bool Processor::boolParameter(String name)
{
    return boolParameter_;
}

void Processor::intParameter(String name, int intParameter)
{
    name_ = name;
    intParameter_ = intParameter;
}

int Processor::intParameter(String name)
{
    return intParameter_;
}

void Processor::floatParameter(String name, float floatParameter)
{
    name_ = name;
    floatParameter_ = floatParameter;
}

float Processor::floatParameter(String name)
{
    return floatParameter_;
}

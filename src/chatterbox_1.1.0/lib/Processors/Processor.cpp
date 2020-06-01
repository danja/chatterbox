#include <map>

#include <Processor.h>

#include <WString.h>
#include <Node.h>

Processor::Processor() {}

/*
  std::map<String, bool> mapOfBool;
  std::map<String, int> mapOfInt;
  std::map<String, float> mapOfFloat;
  */
 
 float Processor::process(float input)
{
    return input;
}

/* Yay! WString has an == operator
https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WString.h
*/

void Processor::boolParameter(String name, bool boolParameter)
{

    mapOfBool_[name] = boolParameter;
}

bool Processor::boolParameter(String name)
{
    return mapOfBool_[name];
}

void Processor::intParameter(String name, int intParameter)
{
    mapOfInt_[name] = intParameter;
}

int Processor::intParameter(String name)
{
    return mapOfInt_[name];
}

void Processor::floatParameter(String name, float floatParameter)
{
    mapOfFloat_[name] = floatParameter;
}

float Processor::floatParameter(String name)
{
    return mapOfFloat_[name];
}

void Processor::stringParameter(String name, String stringParameter)
{
    mapOfString_[name] = stringParameter;
}

String Processor::stringParameter(String name)
{
    return mapOfString_[name];
}

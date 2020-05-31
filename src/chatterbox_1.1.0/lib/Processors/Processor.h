#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <map>

#include <WString.h>
#include <Node.h>

// SHOULD BE A TEMPLATE???
class Processor : public Node
{

public:
  Processor();

  float process(float input);

  void boolParameter(String name, bool boolParameter);
  bool boolParameter(String name);

  void intParameter(String name, int intParameter);
  int intParameter(String name);

  void floatParameter(String name, float floatParameter);
  float floatParameter(String name);

private:
 // String name_;
  // bool boolParameter_;
  // int intParameter_;
  // float floatParameter_;
  std::map<String, bool> mapOfBool_;
  std::map<String, int> mapOfInt_;
  std::map<String, float> mapOfFloat_;
};

#endif
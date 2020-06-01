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

  void stringParameter(String name, String stringParameter);
  String stringParameter(String name);

  /*
  enum TYPE
  {
    LOW_PASS_FILTER,
    BAND_PASS_FILTER,
    HIGH_PASS_FILTER,
    NOTCH_FILTER,
    PEAK_FILTER,
    ALL_PASS_FILTER,
    BELL_FILTER,
    LOW_SHELF_FILTER,
    HIGH_SHELF_FILTER,
    NO_FLT_TYPE
  };
  */

private:
  // String name_;
  // bool boolParameter_;
  // int intParameter_;
  // float floatParameter_;
  std::map<String, bool> mapOfBool_;
  std::map<String, int> mapOfInt_;
  std::map<String, float> mapOfFloat_;
  std::map<String, String> mapOfString_;
};

#endif
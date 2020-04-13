#pragma once

#include <ControlNode.h>

#define SWITCH_PUSH 0
#define SWITCH_TOGGLE 1

class Switch : public ControlNode {
  
  public:
    Switch();
    void setChannel(int ch);
    int getChannel();
    void setRawValue(bool val);
    void setPreviousRawValue(bool val); // TODO refactor
    bool getRawValue();
    bool getPreviousRawValue();
  private:
    int channel;
    bool rawValue;
    bool previousRawValue;
};

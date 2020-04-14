#pragma once

#include <WString.h>
#include <ControlNode.h>

#define PUSH 0
#define TOGGLE 1

class Switch : public ControlNode {
  
  public:
    Switch();
    Switch(String id, int channel, int type);
    // void setChannel(int channel);
    int getChannel();
    int getType();
    void setValue(bool val); // change to isOn() ?
    void setPreviousValue(bool val); // TODO refactor
    bool getValue();
    bool getPreviousValue();
    float getGain();
    void setGain(float gain);

    // used for push switches, redundant for toggles
    void setHold(bool hold); // TODO refactor/rename
    bool getHold();

  private:
    int channel;
    int type;
    bool value;
    bool previousValue;
    float gain; // TODO refactor
    bool hold;
};

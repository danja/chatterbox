#pragma once

#include <WString.h>
#include <ControlNode.h>

#define PUSH 0
#define TOGGLE 1

class Switch : public ControlNode {
  
  public:
    Switch();
    // Switch(String id);
    Switch(String id, int channel, int type);
    // void setChannel(int channel);
    int channel();
    int type();
    void on(bool on); // change to isOn() ?
    void previous(bool on); // TODO refactor
    bool on();
    bool previous();
    float gain();
    void gain(float gain);

    // used for push switches, redundant for toggles
    bool hold();
    void hold(bool hold); 
    
  private:
    int channel_;
    int type_;
    bool on_ = false;
    bool previous_;
    float gain_; // TODO refactor
    bool hold_;
};

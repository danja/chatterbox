#pragma once

#include <WString.h>
#include <ControlNode.h>

#define N_SWITCHES 12 // NEEDED
#define N_PUSH_SWITCHES 8

#define PUSH 0
#define TOGGLE 1

#define SWITCH_SF1 0 // // NEEDED Sibilance Filter 1
#define SWITCH_SF2 1
#define SWITCH_SF3 2

#define SWITCH_VOICED 3
#define SWITCH_ASPIRATED 4

#define SWITCH_NASAL 5
#define SWITCH_DESTRESS 6
#define SWITCH_STRESS 7

#define TOGGLE_HOLD 8
#define TOGGLE_CREAK 9
#define TOGGLE_SING 10
#define TOGGLE_SHOUT 11

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

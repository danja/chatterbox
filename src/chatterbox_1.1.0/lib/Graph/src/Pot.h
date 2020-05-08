#pragma once

#include <WString.h>
#include <Control.h>

/* CONTROLS */
#define N_POTS_ACTUAL 6
#define N_POTS_VIRTUAL 7

// ACTUAL Pots
#define POT_P0 0
#define POT_P1 1
#define POT_P2 2
#define POT_P3 3
#define POT_P4 4
#define POT_P5 5

// VIRTUAL Pots
#define POT_GROWL 6 // ????

class Pot : public Control {
  
  public:
    Pot();
    Pot(String id, int channel);

    int channel();
    void raw(int raw); 
    int raw();
    void previous(int previous);
    int previous();
    float value();
    // void offset(float offset);
    // void scale(float scale);
    void range(int inputRange, float min, float max);
    
  private:
    int channel_;
    int raw_;
    float rawFloat_ = 0.0f;
    int previous_ = 0;
    float value_ = 0.0f;

    float offset_ = 0;
    float scale_ = 1.0f;
};

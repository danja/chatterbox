#pragma once

#include <WString.h>
#include <ControlNode.h>

class Pot : public ControlNode {
  
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

#pragma once

// #include <WString.h>
#include <Node.h>


class Processor : public Node {
  
  public:
    Processor();
    float process(float input);
    
    /*
  private:
    int channel_;
    int type_;
    bool on_ = false;
    bool previous_;
    float gain_; // TODO refactor
    bool hold_;
    */
};

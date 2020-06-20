#ifndef Switches_h
#define Switches_h

#include <Switch.h>
// #include <WString.h>
#include <array>

class Switches
{

public:
  Switches();

  Switch &getSwitch(int switchN);
  void init();

private:
  // std::array<Switch, N_SWITCHES> switchArray;
  Switch  switchArray[N_SWITCHES];
};

#endif
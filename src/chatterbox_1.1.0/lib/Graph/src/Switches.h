#include <Switch.h>
// #include <WString.h>
#include <array>

class Switches
{

public:
  Switches();
  // void setValue(int switchN, bool value);
  // bool getValue(int switchN);
  Switch& getSwitch(int switchN);
  void init();

private:
 //  Switch switchArray[N_SWITCHES];
std::array<Switch, N_SWITCHES> switchArray;
};
#include <Switch.h>
// #include <WString.h>

class Switches {

  public:
      Switches();
      // void setValue(int switchN, bool value);
      // bool getValue(int switchN);
      Switch getSwitch(int switchN);
  private:
  Switch svitches[N_SWITCHES];

};
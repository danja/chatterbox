#include <WString.h>
#include <Node.h>
#include <Control.h>

// Control::Control(){}

  void Control::raw(int raw) {
    raw_ = raw;
}

  int Control::raw() {
    return raw_;
}
#pragma once

#include <Node.h>

class Control: public Node {
  public:
    // Control();
    void raw(int raw);
    int raw();

  private:
    int raw_;
};

#pragma once

#include <Node.h>

class ControlNode: public Node {
  public:
    // ControlNode();
    void raw(int raw);
    int raw();

  private:
    int raw_;
};

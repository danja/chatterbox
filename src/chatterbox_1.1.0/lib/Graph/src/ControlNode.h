#pragma once

#include <Node.h>

class ControlNode: public Node {
  public:
    ControlNode();
    void setRawValue(int rawValue);
    int getRawValue();

  private:
    int rawValue;
};

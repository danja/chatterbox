#include <WString.h>
#include <Node.h>
#include <ControlNode.h>

ControlNode::ControlNode(){}

  void ControlNode::setRawValue(int i) {
    rawValue = i;
}

  int ControlNode::getRawValue() {
    return rawValue;
}
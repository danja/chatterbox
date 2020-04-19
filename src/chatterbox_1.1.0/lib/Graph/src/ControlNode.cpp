#include <WString.h>
#include <Node.h>
#include <ControlNode.h>

// ControlNode::ControlNode(){}

  void ControlNode::raw(int raw) {
    raw_ = raw;
}

  int ControlNode::raw() {
    return raw_;
}
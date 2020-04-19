#include <WString.h>
#include <Node.h>

Node::Node(){}

Node::Node(String id){
id_ = id;
}

  void Node::id(String id) {
    id_ = id;
}

  String Node::id() {
    return id_;
}



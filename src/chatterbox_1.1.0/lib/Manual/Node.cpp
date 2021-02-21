#include <WString.h>
#include <Node.h>

Node::Node(){}

Node::Node(const String& id){
id_ = id;
}

  void Node::id(const String& id) {
    id_ = id;
}

  String& Node::id() {
    return id_;
}



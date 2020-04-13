#include <Arduino.h>
#include <Node.h>

Node::Node(){}

Node::Node(String string){
id = string;
}

  void Node::setID(String string) {
    id = string;
}

  String Node::getID() {
    return id;
}



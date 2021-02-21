#ifndef _NODE_H
#define _NODE_H

#include <WString.h>

class Node {

  public:
      Node();
      Node(const String& id);
      void id(const String& id);
      String& id();
  private:
      String id_;
};

#endif
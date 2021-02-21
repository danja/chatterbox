#ifndef _NODE_H
#define _NODE_H

#include <WString.h>

class Node {

  public:
      Node();
      Node(String id);
      void id(String id);
      String id();
  private:
      String id_;
};

#endif
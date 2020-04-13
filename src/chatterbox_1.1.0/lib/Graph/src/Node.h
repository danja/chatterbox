#pragma once

#include <WString.h>

class Node {

  public:
        Node();
      Node(String id);
      void setID(String id);
      String getID();
  private:
      String id;
};

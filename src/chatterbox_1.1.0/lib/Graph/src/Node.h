#pragma once

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

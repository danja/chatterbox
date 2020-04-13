#include<WString.h> // Arduino's String
#include <unity.h>
#include<Node.h>

#ifndef UNIT_TEST

void setup () {
// delay(2000);
  String idString = "test";
  Node node = Node(idString);
  // node.setID(idString);
// delay(2000);
  TEST_ASSERT_TRUE(idString.equals(node.getID()));
  
// Serial.println("after test");
}

void loop(){}

void loop () {}

#endif

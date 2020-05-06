#include <iostream>
#include <string>
#include "dispatcher.hpp" // the implementation is here

using namespace std;

// a dummy enum for this example

enum EventEnum {EVENT1, EVENT2, EVENT3};

int main(int argc, char *argv[])
{
    // 2 example dispatchers, any number of arguments and types can be used:
    Dispatcher<string, EventEnum> d1;// here any cb(string, EventEnum) can register
    Dispatcher<int, long, double> d2;// here any cb(int, long, double) can register

    // From the "most simple" lambda usage example ...

    auto cbid1 = d1.addCB([](string str, EventEnum evt) {
                          cout << "CB1:" << str << " got event " << evt << endl;
                          });
    auto cbid2 = d1.addCB([](string str, EventEnum evt) {
                          cout << "CB2:" << str << " got event " << evt << endl;
                          });

    d1.broadcast("** Dispatching to 2 is **", EVENT1);
    d1.broadcast("**       E a s y       **", EVENT2);

    d1.delCB(cbid1); // remove the first callback
    d1.broadcast("** Dispatching to 1 is **", EVENT1);
    d1.broadcast("**       E a s y       **", EVENT2);

    d1.delCB(cbid2); // remove the second callback
    d1.broadcast("** No one will see this **",EVENT3);

    // ... to the "most complex" **live** instance (not copy) usage example:

    class MyClassWithMethod {
    public:
        void registerCB(Dispatcher<int, long, double> &dispatcher) {
            using namespace std::placeholders;
            dispatcher.addCB(std::bind(&MyClassWithMethod::listener, this, _1, _2, _3));
        }
    private:
        // any method with the right signature can be used:
        void listener(int i, long l, double d) {
            cout << "listener() for " << this << ", got: " <<
            i << ", " << l << ", " << d << endl;
        }
    };

    MyClassWithMethod instance1;
    MyClassWithMethod instance2;
    instance1.registerCB(d2);
    instance2.registerCB(d2);
    d2.broadcast(65000, 12345678910, 3.14159265);
    d2.broadcast(56000, 1987654321, 14159265.3);


    return 0;
}
/* Example output:
CB1:** Dispatching to 2 is ** got event 0
CB2:** Dispatching to 2 is ** got event 0
CB1:**       E a s y       ** got event 1
CB2:**       E a s y       ** got event 1
CB2:** Dispatching to 1 is ** got event 0
CB2:**       E a s y       ** got event 1
listener() for 0x7ffcb1bfe6fe, got: 65000, 12345678910, 3.14159
listener() for 0x7ffcb1bfe6ff, got: 65000, 12345678910, 3.14159
listener() for 0x7ffcb1bfe6fe, got: 56000, 1987654321, 1.41593e+07
listener() for 0x7ffcb1bfe6ff, got: 56000, 1987654321, 1.41593e+07
 */
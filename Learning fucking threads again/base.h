#include <iostream>
#include <thread>
#include <conio.h>
#include <vector>
#include <string>


// defining clasc
class base {
public:
    // non-static member function
    void foo(int param) { std::cout << param * 5; }
};
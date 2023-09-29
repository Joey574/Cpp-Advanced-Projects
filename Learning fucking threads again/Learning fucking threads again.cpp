#pragma once
#include <iostream>
#include <thread>
#include <conio.h>
#include <vector>
#include <string>

#include "base.h"

class Base;


int main()
{
    base b;

    // first parameter is the reference to the functionn
    // and second paramter is reference of the object
    // at last we have arguments
    std::thread thread_obj(&base::foo, &b, 1);

    thread_obj.join();
}
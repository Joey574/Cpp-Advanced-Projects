/*

    It's me, im the dummy

*/

#include <iostream>

void foo() {
    std::cout << "is true!";
}

void bar() {
    std::cout << "is false!";
}

template<int N>
struct factorial {
    static constexpr int value = N * factorial<N - 1>::value;
};

template<>
struct factorial<0> {
    static constexpr int value = 1;
};

template<bool cond>
struct funcCall {
    static void Call() {
        foo();
    }
};

template<>
struct funcCall<false> {
    static void Call() {
        bar();
    }
};

int main()
{
    constexpr int num = 5;
    constexpr int result = factorial<num>::value;

    std::cout << "factorial of " << num << " is " << result << std::endl;

    constexpr bool cond = true;

    funcCall<cond>::Call();
}
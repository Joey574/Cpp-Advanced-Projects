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



template<float l1, float l2, float momentum>
struct back_prop {
    static float compute(float a, float b) {

        if constexpr (momentum) {
            b = b * momentum;
        }

        if constexpr (l1) {
            a = a * l1;
        }

        if constexpr (l2) {
            a = l2 - a;
        }

        return a - b;
    }
};


int main()
{
   /* constexpr int num = 5;
    constexpr int result = factorial<num>::value;

    std::cout << "factorial of " << num << " is " << result << std::endl;

    constexpr bool cond = true;

    funcCall<cond>::Call();*/

    constexpr float l1 = 0.0f;
    constexpr float l2 = 1.0f;
    constexpr float momentum = 0.5f;

    std::cout << back_prop<l1, l2, momentum>::compute(2, 3);

}
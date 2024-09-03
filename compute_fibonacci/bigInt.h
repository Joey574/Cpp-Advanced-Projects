#pragma once
#include <string>
#include <vector>

class bigInt
{
public:
    bigInt();
    bigInt(int);
    bigInt(const bigInt &);

    const bigInt & operator = (const bigInt &);
    const bigInt& operator += (const bigInt&);
    const bigInt& operator -= (const bigInt&);
    const bigInt& operator *= (const bigInt&);
    const bigInt& operator *= (int num);

    std::string ToString() const;
    int ToInt() const;
    double ToDouble() const; 

private:
    enum Sign { positive, negative };
    Sign _sign; 
    std::vector<char> _digits; 
    int _num_digits; 
};


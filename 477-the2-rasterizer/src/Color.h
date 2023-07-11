#ifndef __COLOR_H__
#define __COLOR_H__

#include <iostream>
#include <cmath>

class Color
{
public:
    double r, g, b;

    Color();
    Color(double r, double g, double b);
    Color(const Color &other);
    Color operator-(const Color &other);
    Color operator+(const Color &other);
    Color operator/(double fact);
    Color operator*(double fact);
    Color round_color();
    friend std::ostream& operator<<(std::ostream& os, const Color& c);
};

#endif
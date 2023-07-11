#include "Color.h"
#include <iostream>
#include <iomanip>

using namespace std;

Color::Color() {}

Color::Color(double r, double g, double b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

Color Color::operator-(const Color &other)
{
    return Color(this->r - other.r, this->g - other.g, this->b - other.b);
}

Color Color:: operator+(const Color &other)
{
    return Color(this->r + other.r, this->g + other.g, this->b + other.b);
}


Color Color::operator/(double fact)
{
    return Color(this->r / fact, this->g / fact, this->b / fact);
}

Color Color::round_color()
{
    return Color((int)(r+0.5), (int)(g+0.5), (int)(b+0.5));
}

Color Color::operator*(double fact)
{
    return Color(this->r * fact, this->g * fact, this->b * fact);
}

ostream& operator<<(ostream& os, const Color& c)
{
    os << fixed << setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}

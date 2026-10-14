#include <Helpers/Vector.hh>

#include <Helpers/Math.hh>

#include <cmath>

Vector::Vector() {}

Vector::Vector(Vector &o) : x(o.x) , y(o.y) {}

Vector::Vector(float _x, float _y) : x(_x), y(_y) {}

Vector Vector::rand(float magnitude) {
    float angle = frand() * 2 * M_PI;
    return { cosf(angle) * magnitude, sinf(angle) * magnitude };
}

void Vector::set(float _x, float _y) {
    x = _x;
    y = _y;
}


Vector &Vector::operator=(Vector const &o) {
    x = o.x;
    y = o.y;
    return *this;
}

Vector &Vector::operator+=(Vector const &o) {
    x += o.x;
    y += o.y;
    return *this;
}

Vector &Vector::operator-=(Vector const &o) {
    x -= o.x;
    y -= o.y;
    return *this;
}

Vector &Vector::operator*=(float v) {
    x *= v;
    y *= v;
    return *this;
}

Vector Vector::operator+(Vector const &v) const {
    return Vector{ x + v.x, y + v.y };
}

Vector Vector::operator-(Vector const &v) const {
    return Vector{ x - v.x, y - v.y };
}

Vector Vector::operator*(float v) const {
    return Vector{ x * v, y * v };
}

float Vector::magnitude() const {
    return sqrtf(x * x + y * y);
}

float Vector::angle() const {
    if (x == 0 && y == 0) return 0;
    return atan2f(y, x);
}

Vector &Vector::normalize() {
    float mag = magnitude();
    // NaN check first: under -ffast-math, mag==0 is also true for NaN
    if (!is_finite(mag)) { x = 0; y = 0; return *this; }
    if (mag == 0.f) return *this;
    x /= mag;
    y /= mag;
    return *this;
}

Vector &Vector::set_magnitude(float v) {
    float mag = magnitude();
    if (!is_finite(mag)) { x = 0; y = 0; return *this; }
    if (mag == 0.f) return *this;
    x = x / mag * v;
    y = y / mag * v;
    return *this;
}

Vector &Vector::unit_normal(float a) {
    x = cosf(a);
    y = sinf(a);
    return *this;
}
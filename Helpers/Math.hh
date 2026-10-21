#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

constexpr uint32_t div_round_up(uint32_t a, uint32_t b) { return (a + b - 1) / b; }

// True if x is NOT NaN and NOT ±Inf. Bit-check on IEEE-754 exponent so it
// still works under -ffast-math / -ffinite-math-only (std::isfinite can be
// optimized to a constant true). Copied from florr_incremental.
static inline bool is_finite(float x) {
    uint32_t bits;
    std::memcpy(&bits, &x, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}
static inline bool is_finite(double x) {
    uint64_t bits;
    std::memcpy(&bits, &x, sizeof(bits));
    return (bits & 0x7ff0000000000000ull) != 0x7ff0000000000000ull;
}

double frand();
float fclamp(float, float, float);
float lerp(float, float, float);
float angle_lerp(float, float, float);
float normalize_angle(float);
float angle_within(float, float, float);

class LerpFloat {
    float value;
    float lerp_value;
    uint32_t touched;
public:
    LerpFloat();
    void operator=(float);
    void set(float);
    operator float() const;
    void step(float);
    void step_angle(float);
    float anchor() const;
};

class SeedGenerator {
    uint32_t seed;
public:
    SeedGenerator(uint32_t);
    float next();
    float binext();
};

class RangeValue {
public:
    float lower;
    float upper;
    RangeValue(float, float);
    RangeValue(float);
    float get_single(float) const;
    std::string const to_string() const;
};

std::string format_pct(float);
std::string format_score(float);
std::string format_number(float);
// Stack badge: x12 / x1.2k / x1.5m
std::string format_stack(uint32_t);
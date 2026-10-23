#include <Helpers/Math.hh>

#include <cmath>
#include <cstdio>
#include <string>

float fclamp(float v, float s, float e) {
    if (!(v >= s)) return s;
    if (!(v <= e)) return e;
    return v;
}

double frand() {
    return std::rand() / (((double) RAND_MAX) + 1);
}

float lerp(float v, float e, float a) {
    a = fclamp(a, 0, 1);
    return v * (1 - a) + e * a;
}

float angle_lerp(float start, float end, float t) {
    t = fclamp(t, 0, 1);

    start = fmod(start, 2 * M_PI);
    end = fmod(end, 2 * M_PI);
    if (fabs(end - start) < M_PI)
        return (end - start) * t + start;
    else
    {
        if (end > start)
            start += 2 * M_PI;
        else
            end += 2 * M_PI;
        return fmod((end - start) * t + start + 2 * M_PI, 2 * M_PI);
    }
}

float normalize_angle(float angle) {
    angle = fmod(angle, 2 * M_PI);
    if (angle < 0) angle += 2 * M_PI;
    return angle;
}

float angle_within(float angle, float target, float dist) {
    angle = normalize_angle(angle);
    target = normalize_angle(target);
    float diff = normalize_angle(angle - target);
    return diff <= dist || diff >= 2 * M_PI - dist;
}

LerpFloat::LerpFloat() {
    value = lerp_value = 0;
    touched = 0;
}

void LerpFloat::operator=(float v) {
    value = lerp_value = v;
    touched = 0;
}

void LerpFloat::set(float v) {
    value = v;
    if (!touched) {
        touched = 1;
        lerp_value = v;
    }
}

LerpFloat::operator float() const {
    return lerp_value;
}

void LerpFloat::step(float amt) {
    lerp_value = lerp(lerp_value, value, amt);
}

void LerpFloat::step_angle(float amt) {
    lerp_value = angle_lerp(lerp_value, value, amt);
}

float LerpFloat::anchor() const {
    return value;
}

SeedGenerator::SeedGenerator(uint32_t s) : seed(s) {}

float SeedGenerator::next() {
    seed *= 167436543;
    seed += 5832385;
    seed *= (76372345 + seed);
    seed += 937323;
    return (seed % 65536) / 65536.0f;
}

float SeedGenerator::binext() {
    return next() * 2 - 1;
}

RangeValue::RangeValue(float l, float u) : lower(l), upper(u) {}

RangeValue::RangeValue(float l) : lower(l), upper(l) {}

float RangeValue::get_single(float a) const {
    if (lower == upper) return lower;
    return lower + (upper - lower) * fclamp(a, 0, 1);
}

std::string const RangeValue::to_string() const {
    if (lower == upper) return format_score(lower);
    return format_score(lower) + '~' + format_score(upper);
}

static std::string fmtf(char const *fmt, float v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), fmt, (double)v);
    return std::string(buf);
}

std::string format_pct(float pct) {
    if (pct <= 0) return "0%";
    if (pct >= 0.9) return fmtf("%.0f%%", pct);
    if (pct >= 0.09) return fmtf(".%.0f%%", pct * 10);
    if (pct >= 0.009) return fmtf(".0%.0f%%", pct * 100);
    return fmtf(".00%.0f%%", pct * 1000);
}

std::string format_score(float score) {
    if (score < 1000.f) return fmtf("%.0f", score);
    if (score < 1000000.f) {
        float k = score / 1000.f;
        if (std::fabs(k - std::round(k)) < 0.05f)
            return fmtf("%.0fk", std::round(k));
        return fmtf("%.1fk", k);
    }
    float m = score / 1000000.f;
    if (std::fabs(m - std::round(m)) < 0.05f)
        return fmtf("%.0fm", std::round(m));
    return fmtf("%.1fm", m);
}

std::string format_number(float num) {
    if (num < 0.1) return fmtf("%.2f", num);
    if (num < 10 && num != ::floorf(num)) return fmtf("%.1f", num);
    return format_score(num);
}

std::string format_stack(uint32_t n) {
    return "x" + format_score((float) n);
}
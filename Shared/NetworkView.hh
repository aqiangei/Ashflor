#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace NetworkView {
inline bool finite_screen_value(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    volatile uint32_t raw = bits; // retain the check under -ffast-math
    return (raw & 0x7f800000u) != 0x7f800000u;
}
// Send screen extents before FOV, so server-side vision changes take effect
// immediately. Cosmetic zoom and window resizing do not alter the protocol's
// world coordinates or the server's authoritative camera.
inline uint16_t screen_half(float pixels, float scale, float zoom) {
    if (!finite_screen_value(pixels) || !finite_screen_value(scale) || !finite_screen_value(zoom)
        || pixels <= 0.f || scale <= 0.f || zoom <= 0.f) return 0;
    return (uint16_t)std::ceil(std::clamp(pixels / (2.f * scale * zoom), 1.f, 4096.f));
}

inline void half_extents(float fov, float base_fov, uint16_t width, uint16_t height,
                         float &half_w, float &half_h) {
    fov = std::clamp(fov, base_fov * .08f, base_fov);
    if (width == 0 || height == 0) {
        // Clients predating viewport input retain their original visibility.
        half_w = 960.f / (fov * .68f) + 160.f;
        half_h = 540.f / (fov * .68f) + 160.f;
    } else {
        // Limit a reported view to the supported maximum zoom-out. Include a
        // margin for movement between packets and sprites crossing the screen.
        half_w = std::clamp((float)width, 160.f, 1746.f) / fov + 200.f;
        half_h = std::clamp((float)height, 90.f, 982.f) / fov + 200.f;
    }
}
}

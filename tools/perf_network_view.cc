#include <Shared/NetworkView.hh>
#include <Shared/Binary.hh>
#include <cassert>
#include <cstdio>
#include <limits>

int main() {
    unsigned checked = 0;
    for (float scale : {.5f, .6666667f, 1.f, 2.f})
        for (float zoom : {.55f, .68f, 1.f, 1.28f, 1.8f})
            for (float fov : {.072f, .45f, .9f}) {
                float w = 1920.f * scale, h = 1080.f * scale;
                auto rw = NetworkView::screen_half(w, scale, zoom);
                auto rh = NetworkView::screen_half(h, scale, zoom);
                float hw, hh;
                NetworkView::half_extents(fov, .9f, rw, rh, hw, hh);
                assert(hw >= w / (2.f * scale * zoom * fov));
                assert(hh >= h / (2.f * scale * zoom * fov));
                ++checked;
            }
    float old_w, old_h, new_w, new_h;
    NetworkView::half_extents(.9f, .9f, 0, 0, old_w, old_h);
    assert(std::abs(old_w - (960.f / (.9f * .68f) + 160.f)) < .001f);
    assert(std::abs(old_h - (540.f / (.9f * .68f) + 160.f)) < .001f);
    NetworkView::half_extents(.9f, .9f, 750, 422, new_w, new_h);
    assert(new_w * new_h < old_w * old_h * .4f);
    NetworkView::half_extents(.9f, .9f, 65535, 65535, new_w, new_h);
    assert(new_w < 2200.f && new_h < 1400.f);
    assert(NetworkView::screen_half(1280, 0, 1) == 0);
    assert(NetworkView::screen_half(1280, 1, 0) == 0);
    volatile uint32_t invalid_bits = 0x7f800000u;
    uint32_t bits = invalid_bits;
    float invalid;
    std::memcpy(&invalid, &bits, sizeof(invalid));
    assert(NetworkView::screen_half(invalid, 1, 1) == 0);
    uint8_t bytes[16];
    Writer writer(bytes);
    writer.write<uint16_t>(750); writer.write<uint16_t>(422);
    Validator validator(bytes, writer.at);
    assert(validator.validate_uint16() && validator.validate_uint16());
    assert(validator.at == validator.end);
    Reader reader(bytes);
    assert(reader.read<uint16_t>() == 750 && reader.read<uint16_t>() == 422);
    Validator partial(bytes, writer.at - 1);
    assert(partial.validate_uint16() && !partial.validate_uint16());
    std::printf("PASS %u view configurations, legacy fallback, view limits, input validation and wire roundtrip\n", checked);
}

#pragma once

#include <Client/Render/Renderer.hh>
#include <cstdint>

namespace Ui {
    class Element;
}

// AshFlor shared UI chrome — heavy plate / brass rim (not candy bubbles)
namespace Ui::Chrome {
    // Surfaces
    inline constexpr uint32_t kPanelDeep   = 0xf2141a12; // almost-opaque olive black
    inline constexpr uint32_t kPanelFace   = 0xff2a3226;
    inline constexpr uint32_t kPanelInset  = 0xff1a2218;
    inline constexpr uint32_t kRimBrass    = 0xffc9a24a;
    inline constexpr uint32_t kRimDark     = 0xff080a06;
    // Ink
    inline constexpr uint32_t kTitle       = 0xfff3ebe0;
    inline constexpr uint32_t kBody        = 0xffddd2c0;
    inline constexpr uint32_t kMuted       = 0xffa89878;
    inline constexpr uint32_t kGold        = 0xffe8c86a;
    // Action stones (muted, desaturated)
    inline constexpr uint32_t kBtnMoss     = 0xff3e5a38;
    inline constexpr uint32_t kBtnSlate    = 0xff3a4850;
    inline constexpr uint32_t kBtnClay     = 0xff7a3e36;
    inline constexpr uint32_t kBtnStone    = 0xff4a4e44;
    inline constexpr uint32_t kBtnBronze   = 0xff6a5430;
    inline constexpr uint32_t kBtnIron     = 0xff3c403c;
    inline constexpr uint32_t kBtnDanger   = 0xff8a3030;

    inline constexpr float kRadius = 3.f;
    inline constexpr float kRim = 6.f;

    // Beveled plate: dark outer + brass stroke + face + top sheen
    void draw_plate(Renderer &ctx, float w, float h, uint32_t face,
                    float radius = kRadius, float rim = kRim);
}

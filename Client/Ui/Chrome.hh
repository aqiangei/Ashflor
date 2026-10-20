#pragma once

#include <Client/Render/Renderer.hh>
#include <cstdint>

namespace Ui {
    class Element;
}

// Warm, playful game chrome shared by title screens and in-game panels.
namespace Ui::Chrome {
    // Surfaces
    inline constexpr uint32_t kPanelDeep   = 0xffd89e64;
    inline constexpr uint32_t kPanelFace   = 0xffe2ac72;
    inline constexpr uint32_t kPanelInset  = 0xffb7804f;
    inline constexpr uint32_t kRimBrass    = 0xff915e39;
    inline constexpr uint32_t kRimDark     = 0xff704629;
    // Ink — bright on warm panels (muddy brown reads as “gray” on title UI)
    inline constexpr uint32_t kTitle       = 0xffffffff;
    inline constexpr uint32_t kBody        = 0xfffff8ec;
    inline constexpr uint32_t kMuted       = 0xffc4894a; // warm readable secondary
    inline constexpr uint32_t kGold        = 0xffffebac;
    inline constexpr uint32_t kHint        = 0xfffff3c8; // cream prompts (Enter / map)
    // Bright cartoon buttons (kid-friendly, high chroma — not stone/gray).
    // Moss=play/go · Slate=login/info · Clay=warm accent · Stone=soft violet
    // Bronze=sunny gold · Iron=settings periwinkle · Danger=close/logout
    inline constexpr uint32_t kBtnMoss     = 0xff4ecb71; // grass green
    inline constexpr uint32_t kBtnSlate    = 0xff5ba3f5; // sky blue
    inline constexpr uint32_t kBtnClay     = 0xffff7a59; // coral
    inline constexpr uint32_t kBtnStone    = 0xffa78bfa; // soft violet
    inline constexpr uint32_t kBtnBronze   = 0xffffc94a; // sunny gold
    inline constexpr uint32_t kBtnIron     = 0xff7b8cff; // periwinkle
    inline constexpr uint32_t kBtnDanger   = 0xffff6b6b; // bright red

    inline constexpr float kRadius = 8.f;
    inline constexpr float kRim = 7.f;

    // Rounded tan plate with the thick brown edge used throughout the game.
    void draw_plate(Renderer &ctx, float w, float h, uint32_t face,
                    float radius = kRadius, float rim = kRim);
}

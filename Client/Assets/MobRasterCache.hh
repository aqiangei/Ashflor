#pragma once

#include <Client/Assets/Assets.hh>

#include <cstdint>

class Renderer;

// Discrete-pose bake for procedural mobs (skip ThemeArt / flower-bodied).
namespace MobRasterCache {
    uint8_t try_draw(MobID::T id, MobRenderAttributes const &attrs, Renderer &dest);
    void clear();
}

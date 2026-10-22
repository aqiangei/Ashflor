#pragma once

#include <Client/Render/Renderer.hh>

#include <Shared/StaticDefinitions.hh>

#include <cstdint>
#include <string_view>

// Procedural themed art for explore NPCs / escorts / Yahaha props (design V2.0).
// Does not change collision, AI, or rewards — draw only.
namespace ThemeArt {

enum class Visual : uint8_t {
    None = 0,
    MainNpc,
    WaysideNpc,
    Escort,
    Pad,
    Crate,
    Chest,
    Mirage,
    EventFlee,   // fleeing bush / dune / kelp / …
    EventHeight, // ordered mushrooms / cactus / shells / …
    EventLore,   // seeds, steles, footprints (P2 light)
};

Visual resolve(MobID::T mob_id, std::string_view name, uint8_t map_id);

// Local design space ≈ ±48 → scaled by radius/48.
// `name` used for height-tier / lore variant selection.
void draw(Renderer &ctx, Visual kind, uint8_t map_id, float radius,
          float animation, uint32_t seed, std::string_view name = {});

}

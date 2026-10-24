#pragma once

#include <Client/Render/Renderer.hh>

#include <Shared/StaticData.hh>
#include <stdint.h>

struct FlowerRenderAttributes {
    float radius = 25;
    float eye_x = 3;
    float eye_y = 0;
    float mouth = 15;
    float cutter_angle = 0;
    uint8_t face_flags = 0;
    uint8_t equip_flags = 0;
    uint8_t color = 0;
};

struct MobRenderAttributes {
    float animation;
    float radius;
    uint32_t seed;
    uint32_t flags;
    uint8_t color;
    FlowerRenderAttributes flower_attrs;
};

// reload: 0..1 CD pie. health: 0..1 live durability (bottom rarity-tint wash when < 1).
void draw_loadout_background(Renderer &, PetalID::T, float reload=1, uint8_t draw_name=1, float health=1);

void draw_static_flower(Renderer &, FlowerRenderAttributes);

void draw_static_petal_single(PetalID::T, Renderer &);

void draw_static_petal(PetalID::T, Renderer &);

void draw_static_mob(MobID::T, Renderer &, MobRenderAttributes);

void draw_web(Renderer &);

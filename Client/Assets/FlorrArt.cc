#include <Client/Assets/FlorrArt.hh>

#include <Client/Render/Renderer.hh>
#include <Shared/StaticData.hh>

#include <cstring>

#include "FlorrArtMap.inc"

namespace FlorrArt {

static char const *const kFlorrWornUrl[kWornCount] = {
    "/florr-art/worn/bandage.png",
    "/florr-art/worn/champion-crown.png",
    "/florr-art/worn/pharaoh-crown.png",
    "/florr-art/worn/goggles.png",
};

static uint8_t petal_url_ok(PetalID::T id) {
    if (id >= PetalID::kNumPetals) return 0;
    // Cutter: ashflor procedural blades hug the flower rim; wiki sprite sits wrong.
    if (id == PetalID::kCutter
        || (PETAL_DATA[id].name != nullptr
            && std::strcmp(PETAL_DATA[id].name, "Cutter") == 0))
        return 0;
    char const *url = kFlorrPetalUrl[id];
    return (url != nullptr && url[0] != '\0') ? 1 : 0;
}

static uint8_t mob_url_ok(MobID::T id) {
    if (id >= MobID::kNumMobs) return 0;
    char const *url = kFlorrMobUrl[id];
    if (url == nullptr || url[0] == '\0') return 0;
    // One PNG is a head segment — body segments need procedural (no antennae / face).
    if (MOB_DATA[id].attributes.segments > 1) return 0;
    // Sandstorm PNG is a static hex; procedural spins with animation.
    if (id == MobID::kSandstorm) return 0;
    // Scorpion PNG facing ≠ ashflor +X claws; FlorrArt blit won't track chase angle clearly.
    if (id == MobID::kScorpion) return 0;
    // These PNGs bake legs/stinger/antennae into the frame. try_draw_mob fits the
    // whole sprite into diameter 2*radius, so the body shrinks well below the
    // hitbox (hornet ~0.56R, bee ~0.71R, spider legs clipped inside). Procedural
    // draws the body at radius with overhang — matches collision and animates.
    if (id == MobID::kSpider || id == MobID::kHornet || id == MobID::kBee
        || id == MobID::kQueenAnt || id == MobID::kAntHole
        || id == MobID::kDigger
        // Antennae overhang same shrink (soldier fire/termite PNGs ~0.64R body).
        || id == MobID::kSoldierFireAnt || id == MobID::kSoldierTermite) return 0;
    return 1;
}

uint8_t has_petal_sprite(PetalID::T id) { return petal_url_ok(id); }
uint8_t has_mob_sprite(MobID::T id) { return mob_url_ok(id); }

uint8_t try_draw_petal(PetalID::T id, Renderer &ctx, float radius) {
    if (!petal_url_ok(id)) return 0;
    float d = radius * 2.f;
    if (d < 8.f) d = 8.f;
    return ctx.try_draw_image_data(kFlorrPetalUrl[id], d, d);
}

uint8_t try_draw_mob(MobID::T id, Renderer &ctx, float radius,
        uint32_t flags, uint8_t rarity) {
    // Prefer florrclone in-game mobs/ sprites (gen_florr_art_map prefers those over wiki cards).
    if (!mob_url_ok(id)) return 0;
    // Body segment of a multi-part mob (flag bit1) — never blit the head sprite.
    if (flags & (1u << 1)) return 0;
    // Super / Unique: procedural stays crisp at 5.6×–7× radius; PNG upscales soft.
    if (rarity >= RarityID::kSuper) return 0;
    float d = radius * 2.f;
    if (d < 8.f) d = 8.f;
    return ctx.try_draw_image_data(kFlorrMobUrl[id], d, d);
}

uint8_t try_draw_worn(WornKind kind, Renderer &ctx, float size) {
    if (kind >= kWornCount) return 0;
    char const *url = kFlorrWornUrl[kind];
    if (url == nullptr || url[0] == '\0') return 0;
    float d = size;
    if (d < 8.f) d = 8.f;
    return ctx.try_draw_image_data(url, d, d);
}

uint8_t is_body_worn_petal(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    char const *name = PETAL_DATA[id].name;
    if (name == nullptr) return 0;
    // Equipped look uses dedicated worn sprites on the flower; orbit clone is hidden.
    if (std::strcmp(name, "Champion's Crown") == 0) return 1;
    if (std::strcmp(name, "Pharaoh's Crown") == 0) return 1;
    if (std::strcmp(name, "Paranormal Goggles") == 0) return 1;
    return 0;
}

}

#include <Client/Assets/Assets.hh>
#include <Client/Assets/ThemeArt.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string_view>

#include <emscripten.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

Renderer *g_art = nullptr;

void ensure_art() {
    if (g_art != nullptr) return;
    EM_ASM({
        if (!Module.ctxs) Module.ctxs = [];
        if (!Module.availableCtxs) Module.availableCtxs = [];
    });
    g_art = new Renderer();
}

void clear_art(int w, int h) {
    ensure_art();
    g_art->set_dimensions((float) w, (float) h);
    g_art->reset();
    EM_ASM({
        var c = Module.ctxs[$0];
        c.setTransform(1, 0, 0, 1, 0, 0);
        c.clearRect(0, 0, $1, $2);
        // Soft panel fill so transparent petals read on dark codex chrome
        c.fillStyle = "rgba(18,32,24,1)";
        c.fillRect(0, 0, $1, $2);
    }, g_art->id, w, h);
    g_art->reset();
}

} // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE void ashflor_art_boot() {
    ensure_art();
}

EMSCRIPTEN_KEEPALIVE int ashflor_art_ctx_id() {
    ensure_art();
    return (int) g_art->id;
}

EMSCRIPTEN_KEEPALIVE void ashflor_art_draw_petal(int petal_id, int w, int h) {
    if (petal_id <= 0 || petal_id >= (int) PetalID::kNumPetals) return;
    if (w < 8) w = 8;
    if (h < 8) h = 8;
    clear_art(w, h);
    float scale = (float) std::fmin(w, h) / 60.f;
    g_art->translate(w * 0.5f, h * 0.5f);
    g_art->scale(scale);
    // Same path as PetalGallery — rarity tile + petal art, no name text.
    draw_loadout_background(*g_art, (PetalID::T) petal_id, 1.f, /*draw_name=*/0, 1.f);
}

EMSCRIPTEN_KEEPALIVE void ashflor_art_draw_mob(int mob_id, int rarity, int w, int h) {
    if (mob_id < 0 || mob_id >= (int) MobID::kNumMobs) return;
    if (w < 8) w = 8;
    if (h < 8) h = 8;
    if (rarity < 0) rarity = 0;
    if (rarity >= (int) RarityID::kNumRarities) rarity = RarityID::kNumRarities - 1;
    clear_art(w, h);

    // Match MobGallery::on_render framing (clip + rotate + scale).
    float side = (float) std::fmin(w, h);
    g_art->translate(w * 0.5f, h * 0.5f);
    {
        RenderContext clip(g_art);
        g_art->begin_path();
        g_art->round_rect(-side * 0.5f, -side * 0.5f, side, side, side / 12.f);
        g_art->clip();

        MobID::T id = (MobID::T) mob_id;
        struct MobData const &data = MOB_DATA[id];
        if (id != MobID::kDigger)
            g_art->rotate(-3.f * (float) M_PI / 4.f);
        if (id == MobID::kBeetle || id == MobID::kMassiveBeetle || id == MobID::kFlowerGuardian)
            g_art->translate(-5.f, 0);
        float radius = (data.radius.upper + data.radius.lower) * 0.5f;
        radius *= mob_rarity_radius_mult((uint8_t) rarity);
        if (radius > side * 0.5f) g_art->scale(0.5f * side / radius);
        g_art->scale(0.5f);
        draw_static_mob(id, *g_art, {
            .animation = 0.f,
            .radius = radius,
            .seed = 1u,
            .flags = 0,
            .color = 0,
            .flower_attrs = { .color = ColorID::kGray }
        });
        if (data.attributes.segments > 1) {
            g_art->translate(-2.f * radius, 0);
            draw_static_mob(id, *g_art, {
                .animation = 0.f,
                .radius = radius,
                .seed = 1u,
                .flags = 1u << 1,
                .color = 0,
                .flower_attrs = { .color = ColorID::kGray }
            });
        }
    }
}

static char const *theme_sample_name(int visual, int map_id) {
    // Height / lore variants key off display name substrings (ThemeArt::resolve).
    if (visual == (int) ThemeArt::Visual::EventHeight) {
        static char const *const k[] = {
            "树脂菇", "仙人掌", "贝壳", "冰晶", "菌", "蚁丘菇", "硫磺菇", "纹章"
        };
        return k[map_id % 8];
    }
    if (visual == (int) ThemeArt::Visual::EventLore) {
        static char const *const k[] = {
            "种子", "石碑", "脚印", "风痕", "藤种", "巢芯", "余烬", "圣印"
        };
        return k[map_id % 8];
    }
    if (visual == (int) ThemeArt::Visual::EventFlee) {
        static char const *const k[] = {
            "草丛", "沙丘", "海草", "雪堆", "灌木", "蚁丘", "灰堆", "旗帜"
        };
        return k[map_id % 8];
    }
    if (visual == (int) ThemeArt::Visual::Mirage) return "蜃楼宝箱";
    if (visual == (int) ThemeArt::Visual::WaysideNpc) {
        static char const *const k[] = {
            "路旁叶语", "沙路掮客", "礁边向导", "霜路残影",
            "林缘贤影", "穴道哨蚁", "灰道囚人", "街角掮客"
        };
        return k[map_id % 8];
    }
    if (visual == (int) ThemeArt::Visual::Escort) return "迷路精灵";
    return "";
}

EMSCRIPTEN_KEEPALIVE void ashflor_art_draw_theme(int visual, int map_id, int w, int h,
                                                  float anim, unsigned seed, char const *name) {
    if (visual <= 0 || visual > (int) ThemeArt::Visual::EventLore) return;
    if (map_id < 0) map_id = 0;
    if (map_id >= (int) MapID::kNumMaps) map_id = 0;
    if (w < 8) w = 8;
    if (h < 8) h = 8;
    clear_art(w, h);
    float side = (float) std::fmin(w, h);
    float radius = side * 0.42f;
    g_art->translate(w * 0.5f, h * 0.55f);
    std::string_view nm = (name && *name) ? name : theme_sample_name(visual, map_id);
    ThemeArt::draw(*g_art, (ThemeArt::Visual) visual, (uint8_t) map_id,
                   radius, anim, seed, nm);
}

} // extern "C"

int main() {
    ensure_art();
    return 0;
}

#include <Client/Assets/FlorrTerrain.hh>

#include <Client/Render/Renderer.hh>

#include <emscripten.h>

#include <cmath>
#include <cstdint>

namespace FlorrTerrain {
namespace {

constexpr float kTile = 256.f;

struct TileSet {
    char const *const *urls;
    uint32_t n;
};

// Ground center variants (florr world-tiles). Empty n = keep procedural only.
char const *const kGardenG[] = {
    "/florr-art/tiles/grass_c_0.svg",
    "/florr-art/tiles/grass_c_1.svg",
    "/florr-art/tiles/grass_c_2.svg",
    "/florr-art/tiles/grass_c_3.svg",
};
char const *const kDesertG[] = {
    "/florr-art/tiles/desert_c_0.svg",
    "/florr-art/tiles/desert_c_1.svg",
    "/florr-art/tiles/desert_c_2.svg",
    "/florr-art/tiles/desert_c_3.svg",
    "/florr-art/tiles/desert_c_4.svg",
};
char const *const kOceanG[] = {
    "/florr-art/tiles/ocean_c_0.svg",
    "/florr-art/tiles/ocean_c_1.svg",
    "/florr-art/tiles/ocean_c_2.svg",
    "/florr-art/tiles/ocean_c_3.svg",
};
char const *const kJungleG[] = {
    "/florr-art/tiles/grass2_c_0.svg",
    "/florr-art/tiles/grass2_c_1.svg",
    "/florr-art/tiles/grass2_c_2.svg",
    "/florr-art/tiles/grass2_c_3.svg",
};
char const *const kAntG[] = {
    "/florr-art/tiles/dirt_c_0.svg",
    "/florr-art/tiles/dirt2_c_0.svg",
    "/florr-art/tiles/dirt2_c_1.svg",
};
char const *const kVolcanoG[] = {
    "/florr-art/tiles/factory_floor_c_0.svg",
    "/florr-art/tiles/factory_floor_c_1.svg",
    "/florr-art/tiles/factory_floor_c_2.svg",
    "/florr-art/tiles/factory_floor_c_3.svg",
    "/florr-art/tiles/worm_c_0.svg",
    "/florr-art/tiles/worm_c_1.svg",
};
char const *const kRoyalG[] = {
    "/florr-art/tiles/cobblestone_c_0.svg",
    "/florr-art/tiles/castle_c_0.svg",
    "/florr-art/tiles/wood_c_0.svg",
};
// Polar has no snow tiles in the dump — leave empty (procedural ThemeMaterial).
char const *const kWallGarden[] = {
    "/florr-art/tiles/organic_wall_c_0.svg",
    "/florr-art/tiles/bush_c_0.svg",
};
char const *const kWallDesert[] = {
    "/florr-art/tiles/scliff_c_0.svg",
    "/florr-art/tiles/desert_c_0.svg",
};
char const *const kWallOcean[] = {
    "/florr-art/tiles/coral_c_0.svg",
};
char const *const kWallJungle[] = {
    "/florr-art/tiles/organic_wall_c_0.svg",
    "/florr-art/tiles/foliage_t_0.svg",
};
char const *const kWallAnt[] = {
    "/florr-art/tiles/dirt_c_0.svg",
    "/florr-art/tiles/anthole.svg",
};
char const *const kWallVolcano[] = {
    "/florr-art/tiles/factory_wall_c_0.svg",
};
char const *const kWallRoyal[] = {
    "/florr-art/tiles/castle_c_0.svg",
    "/florr-art/tiles/factory_wall_c_0.svg",
};
char const *const kWater[] = {
    "/florr-art/tiles/water_c_0.svg",
};
char const *const kChannel[] = {
    "/florr-art/tiles/dirt_c_0.svg",
    "/florr-art/tiles/cobblestone_c_0.svg",
};

TileSet ground_set(uint8_t map_id) {
    switch (map_id) {
    case MapID::kGarden:
    case MapID::kPrimeGarden: return { kGardenG, 4 };
    case MapID::kDesert:
    case MapID::kPrimeDesert:
    case MapID::kPyramid: return { kDesertG, 5 };
    case MapID::kOcean:
    case MapID::kPrimeOcean: return { kOceanG, 4 };
    case MapID::kJungle:
    case MapID::kWildwood: return { kJungleG, 4 };
    case MapID::kAntHell:
    case MapID::kAntNest: return { kAntG, 3 };
    case MapID::kVolcano:
    case MapID::kFactory:
    case MapID::kSewers:
    case MapID::kHel: return { kVolcanoG, 6 };
    case MapID::kRoyalCapital: return { kRoyalG, 3 };
    default: return { nullptr, 0 }; // Polar etc.
    }
}

TileSet wall_set(uint8_t map_id) {
    switch (map_id) {
    case MapID::kGarden:
    case MapID::kPrimeGarden: return { kWallGarden, 2 };
    case MapID::kDesert:
    case MapID::kPrimeDesert:
    case MapID::kPyramid: return { kWallDesert, 2 };
    case MapID::kOcean:
    case MapID::kPrimeOcean: return { kWallOcean, 1 };
    case MapID::kJungle:
    case MapID::kWildwood: return { kWallJungle, 2 };
    case MapID::kAntHell:
    case MapID::kAntNest: return { kWallAnt, 2 };
    case MapID::kVolcano:
    case MapID::kFactory:
    case MapID::kSewers:
    case MapID::kHel: return { kWallVolcano, 1 };
    case MapID::kRoyalCapital: return { kWallRoyal, 2 };
    default: return { nullptr, 0 };
    }
}

uint32_t hash2(int x, int y) {
    uint32_t n = (uint32_t)(x * 374761393 + y * 668265263);
    n = (n ^ (n >> 13)) * 1274126177u;
    return n ^ (n >> 16);
}

char const *pick(TileSet s, int ix, int iy) {
    if (!s.urls || s.n == 0) return nullptr;
    return s.urls[hash2(ix, iy) % s.n];
}

uint32_t stamp(Renderer &ctx, TileSet s, float left, float top, float right, float bottom,
               float alpha) {
    if (!s.urls || s.n == 0) return 0;
    int x0 = (int)std::floor(left / kTile);
    int y0 = (int)std::floor(top / kTile);
    int x1 = (int)std::floor((right - 1.f) / kTile);
    int y1 = (int)std::floor((bottom - 1.f) / kTile);
    // Slight oversize hides 1px hairline seams that read as a white grid.
    float const draw = kTile + 2.f;
    uint32_t painted = 0;
    RenderContext alpha_guard(&ctx);
    if (alpha < 0.999f) ctx.set_global_alpha(alpha);
    for (int iy = y0; iy <= y1; ++iy) {
        for (int ix = x0; ix <= x1; ++ix) {
            char const *url = pick(s, ix, iy);
            if (!url) continue;
            float cx = ((float)ix + 0.5f) * kTile;
            float cy = ((float)iy + 0.5f) * kTile;
            RenderContext guard(&ctx);
            ctx.translate(cx, cy);
            if (ctx.try_draw_image_data(url, draw, draw))
                ++painted;
        }
    }
    return painted;
}

uint8_t g_ready = 0;
uint8_t g_preload_started = 0;

} // namespace

uint8_t tiles_ready() { return g_ready; }

void ensure_preload() {
    if (g_preload_started) return;
    g_preload_started = 1;
    // Only world-tiles — never block on petal/mob urls. Fail-open after 2.5s.
    EM_ASM({
        if (!Module.embeddedImages) Module.embeddedImages = new Map();
        var finish = function () { Module.florrTilesReady = 1; };
        setTimeout(finish, 2500);
        fetch("/florr-art/urls.json", { cache: "force-cache" })
            .then(function (r) { return r.ok ? r.json() : []; })
            .then(function (urls) {
                if (!Array.isArray(urls)) urls = [];
                urls = urls.filter(function (u) {
                    return u && String(u).indexOf("/florr-art/tiles/") === 0;
                });
                var pending = 0;
                var done = function () {
                    if (pending > 0) return;
                    finish();
                };
                urls.forEach(function (url) {
                    var img = Module.embeddedImages.get(url);
                    if (img && img.complete && img.naturalWidth) return;
                    pending++;
                    img = new Image();
                    img.onload = img.onerror = function () {
                        pending--;
                        done();
                    };
                    img.src = url;
                    Module.embeddedImages.set(url, img);
                });
                if (pending === 0) finish();
            })
            .catch(finish);
    });
}

static void poll_ready() {
    if (g_ready) return;
    if (EM_ASM_INT({ return Module.florrTilesReady ? 1 : 0; }))
        g_ready = 1;
}

uint32_t paint_ground(Renderer &ctx, uint8_t map_id,
                      float left, float top, float right, float bottom) {
    ensure_preload();
    poll_ready();
    // Soft blend over ThemeMaterial — full-opaque stamps made a tiled "格子" look.
    return stamp(ctx, ground_set(map_id), left, top, right, bottom, 0.42f);
}

uint32_t paint_wall_fill(Renderer &, uint8_t, float, float, float, float) {
    return 0;
}

uint32_t paint_water_fill(Renderer &, uint8_t, float, float, float, float) {
    return 0;
}

uint32_t paint_channel_fill(Renderer &, uint8_t, float, float, float, float) {
    return 0;
}

}

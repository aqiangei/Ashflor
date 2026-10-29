#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

// Per-map procedural terrain art (no texture files).
// Inspired by florr_clone-web: fill pass then hard edge pass for contrast.
// Palettes follow todo/map/*-terrain-art-spec.md.
namespace TerrainArt {
    enum Style : uint8_t {
        kHedge = 0,  // Garden leafy walls
        kJungle,     // denser / darker hedge
        kRock,       // Desert sandstone
        kReef,       // Ocean
        kIce,        // Snow
        kDirt,       // Ant Hell
        kLava,       // Volcano scorched rock
        kMasonry,    // Royal brick / castle
    };

    struct ThemeMaterial {
        uint32_t ground;
        uint32_t ground_shade;   // low-freq blobs
        uint32_t ground_light;   // pollen / frost / sparkle
        uint32_t wall_deep;      // wall underfill
        uint32_t wall_mid;       // cluster body
        uint32_t wall_hi;        // highlight arc
        uint32_t wall_dark;      // cluster underside
        uint32_t wall_outline;   // hard silhouette (must read at distance)
        uint32_t water;
        uint32_t water_hi;
        uint32_t channel;
        uint32_t channel_speck;
        uint8_t style;
        float outline_w;        // world units
        float cluster_r_lo;
        float cluster_r_hi;
        float ground_step;      // detail grid
    };

    inline ThemeMaterial const &theme(uint8_t map_id) {
        static ThemeMaterial const kTable[MapID::kNumMaps] = {
            // Garden — G01/G04/G06/G07 (channel: olive dirt, not warm pink bar)
            { 0xffB9DB91, 0x338FC777, 0x55F6E9B5,
              0xff285B35, 0xff4E914C, 0xff85C96D, 0xff34733D, 0xff1E472A,
              0xff40916C, 0x66A8E6C0, 0xffC8D6A8, 0x99A8B87A,
              Style::kHedge, 7.f, 22.f, 36.f, 72.f },
            // Desert — sand + sandstone
            { 0xffF2D79B, 0x40E6BE79, 0x55F9E7BA,
              0xff593A2C, 0xffB77B4C, 0xffE8AD69, 0xff815337, 0xff3E281C,
              0xff2A9D8F, 0x6640C4B0, 0xffC9955C, 0x99E8AD69,
              Style::kRock, 7.f, 18.f, 32.f, 80.f },
            // Ocean — shallow sea + reef
            { 0xff3AA0C8, 0x400077B6, 0x5590E0F0,
              0xff021F45, 0xff023E8A, 0xff4CC9F0, 0xff012A55, 0xff011428,
              0xff00B4D8, 0x66A0E8FF, 0xffFF85A1, 0x99FFE0EA,
              Style::kReef, 6.5f, 16.f, 28.f, 76.f },
            // Snow — cool walkable vs blue-grey ice walls.
            // Avoid near-#FFFFFF sparkle/speck: on pale ground + chunk rebake
            // it reads as screen flicker (other biomes already fixed; Polar lagged).
            { 0xffEAF3F7, 0x40D4E5ED, 0x40C1DBE7,
              0xff294D69, 0xff4B829F, 0xffA5DDEA, 0xff3A6A82, 0xff1C3548,
              0xff7FB9D1, 0x66B9D8E8, 0xffB8D4E0, 0x66A8C8D8,
              Style::kIce, 7.f, 20.f, 34.f, 78.f },
            // Jungle — dense canopy
            { 0xff52B788, 0x402D6A4F, 0x4495D5B2,
              0xff081C15, 0xff1B4332, 0xff40916C, 0xff0D2818, 0xff040F0A,
              0xff2D6A4F, 0x6652B788, 0xff95D5B2, 0x99D8F3DC,
              Style::kJungle, 8.f, 24.f, 40.f, 64.f },
            // Ant Hell — Anthill-terrain-art-spec nest floor / dirt wall
            { 0xffC9A477, 0x40A77E58, 0x55E0BF92,
              0xff4B3026, 0xff704A35, 0xffA77751, 0xff856044, 0xff38241E,
              0xff582F0E, 0x668C5E3B, 0xffB08968, 0x99D9C09B,
              Style::kDirt, 7.5f, 18.f, 32.f, 68.f },
            // Volcano — ash + ember
            { 0xff4A4E6A, 0x403D405B, 0x55E76F51,
              0xff0D0D14, 0xff1A1A2E, 0xffE76F51, 0xff12121F, 0xff050508,
              0xffE76F51, 0x66F4A261, 0xffF4A261, 0x99FFD6A5,
              Style::kLava, 7.5f, 18.f, 30.f, 74.f },
            // Royal — warm stone + masonry (art spec)
            { 0xffDCC9A6, 0x40BDA783, 0x55EFE0C2,
              0xff3E4355, 0xff8B8D98, 0xffB5B4BB, 0xff646878, 0xff2A2E3A,
              0xff4A6D8C, 0x6680A8C8, 0xffC94D4C, 0x99D5AD55,
              Style::kMasonry, 7.5f, 12.f, 22.f, 64.f },
            // Ant Nest — same dirt palette as Ant Hell (must match kNumMaps)
            { 0xffC9A477, 0x40A77E58, 0x55E0BF92,
              0xff4B3026, 0xff704A35, 0xffA77751, 0xff856044, 0xff38241E,
              0xff582F0E, 0x668C5E3B, 0xffB08968, 0x99D9C09B,
              Style::kDirt, 7.5f, 18.f, 32.f, 68.f },
            // Wildwood — L2 jungle twin (slightly cooler canopy)
            { 0xff4AA87A, 0x40255A42, 0x4488C8A0,
              0xff061812, 0xff163828, 0xff358060, 0xff0A2218, 0xff030C08,
              0xff255A42, 0x664AA87A, 0xff88C8A0, 0x99C8E8D0,
              Style::kJungle, 8.f, 24.f, 40.f, 64.f },
            // Sewers — damp concrete
            { 0xff6B7068, 0x403A3E38, 0x445A6A4A,
              0xff1A1E18, 0xff2A2E28, 0xff7A8A6A, 0xff222620, 0xff0E100C,
              0xff3D5A3A, 0x665A7A4A, 0xff8A9A7A, 0x99A8B898,
              Style::kDirt, 7.5f, 16.f, 28.f, 70.f },
            // Prime Garden — brighter twin of L1 garden
            { 0xffB9DB91, 0x338FC777, 0x55F6E9B5,
              0xff285B35, 0xff4E914C, 0xff85C96D, 0xff34733D, 0xff1E472A,
              0xff40916C, 0x66A8E6C0, 0xffC8D6A8, 0x99A8B87A,
              Style::kHedge, 7.f, 22.f, 36.f, 72.f },
            // Prime Desert / Sand Seas
            { 0xffF2D79B, 0x40E6BE79, 0x55F9E7BA,
              0xff593A2C, 0xffB77B4C, 0xffE8AD69, 0xff815337, 0xff3E281C,
              0xff2A9D8F, 0x6640C4B0, 0xffC9955C, 0x99E8AD69,
              Style::kRock, 7.f, 18.f, 32.f, 80.f },
            // Prime Ocean / Deep Ocean
            { 0xff3AA0C8, 0x400077B6, 0x5590E0F0,
              0xff021F45, 0xff023E8A, 0xff4CC9F0, 0xff012A55, 0xff011428,
              0xff00B4D8, 0x66A0E8FF, 0xffFF85A1, 0x99FFE0EA,
              Style::kReef, 6.5f, 16.f, 28.f, 76.f },
            // Factory — steel deck
            { 0xff7A8088, 0x403A4550, 0x55C94D4C,
              0xff1A2028, 0xff2A3038, 0xff9AA0A8, 0xff3A4550, 0xff101418,
              0xffC94D4C, 0x66E07070, 0xffA8B0B8, 0x99C0C8D0,
              Style::kMasonry, 7.5f, 14.f, 26.f, 68.f },
            // Pyramid — sandstone tomb
            { 0xffE2C48A, 0x40B08A4A, 0x55D5AD55,
              0xff4A3018, 0xff6A4A28, 0xffE8C878, 0xff8A6A3A, 0xff2A1C0C,
              0xff3A5A6A, 0x665A8A9A, 0xffD5AD55, 0x99E8D0A0,
              Style::kRock, 7.f, 18.f, 32.f, 76.f },
            // Hel — ashen crimson underworld
            { 0xff3A1A22, 0x40201018, 0x55C94D4C,
              0xff0A0408, 0xff1A0A10, 0xffC94D4C, 0xff2A1018, 0xff050208,
              0xffC94D4C, 0x66E07070, 0xffF4A261, 0x99FFD6A5,
              Style::kLava, 7.5f, 18.f, 30.f, 74.f },
        };
        static_assert(sizeof(kTable) / sizeof(kTable[0]) == MapID::kNumMaps,
                      "ThemeMaterial table must cover every MapID");
        if (map_id >= MapID::kNumMaps) map_id = 0;
        return kTable[map_id];
    }
}

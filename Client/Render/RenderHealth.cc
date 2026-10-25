#include <Client/Render/RenderEntity.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/MobLabelCache.hh>
#include <Client/StaticData.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/Entity.hh>
#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

void render_health(Renderer &ctx, Entity const &ent) {
    if (ent.has_component(kPetal)) return;
    // Worm body segments (kSegmented): each link has its own HP. Show a bar
    // when damaged — skipping them entirely hid chrome on non-head hits.
    // Name/rarity stay on the head only so the worm is not a label stack.
    uint8_t const is_segment = ent.has_component(kSegmented);

    uint8_t is_player = ent.has_component(kFlower) && !ent.has_component(kMob)
        && ent.has_component(kName);
    uint8_t is_mob = ent.has_component(kMob);
    uint8_t is_self = is_player && Game::simulation.ent_alive(Game::player_id)
        && (ent.id == Game::player_id);
    // Level / guild stay visible at full HP; only the bar fades with opacity.
    uint8_t show_identity = is_player && !is_self;
    // Reward loot chests: always draw an HP bar (kill-to-open).
    // Other mobs: name+rarity always; bar after damage.
    uint8_t is_loot_chest = is_mob && ent.get_mob_id() == MobID::kLootChest;
    bool const is_training = is_mob && ent.get_mob_id() == MobID::kTrainingDummy;
    uint8_t show_bar = is_loot_chest || is_training || ent.healthbar_opacity >= 0.01f;
    // Head always; body links once damaged (orphan remnants promote off
    // kSegmented server-side — until then, name rides the HP bar).
    uint8_t show_mob_label = is_mob && (!is_segment || show_bar);
    if (!show_bar && !show_identity && !show_mob_label) return;

    // Skip non-combat props (pads / crates / guide + explore escort NPCs).
    // Loot chests keep a bar.
    if (is_mob) {
        MobID::T mid = ent.get_mob_id();
        if (mid == MobID::kPushCrate || mid == MobID::kYahahaPad
            || mid == MobID::kTreasureNpc)
            return;
        // Themed escorts (bee / jelly / …) use nametag_visible — no HP chrome.
        if (ent.has_component(kName) && ent.get_nametag_visible()
            && !ent.get_name().empty())
            return;
    }

    float const rad = ent.get_radius();
    float w = rad * 1.33f;
    float bar_w = 2 * w;
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    // Origin = left end of the HP bar (below the entity). Extra clearance so
    // oversized mob art (beyond physics radius) does not swallow the chrome.
    float y_below = rad + 18.f + std::min(rad * 0.22f, 48.f);
    ctx.translate(-w, y_below);
    ctx.round_line_cap();

    float const label_alpha = 1.f - ent.deletion_animation;
    // Loot chests keep a solid bar even at full HP (opacity otherwise fades to 0).
    float const bar_alpha = label_alpha * (is_loot_chest || is_training ? 1.f
        : (show_bar ? ent.healthbar_opacity : 1.f));

    char const *mob_name = nullptr;
    char const *rarity_label = nullptr;
    uint8_t rarity = RarityID::kNumRarities;
    MobID::T mid = MobID::kNumMobs;
    if (is_mob) {
        mid = ent.get_mob_id();
        if (mid < MobID::kNumMobs) {
            mob_name = I18n::mob_name(mid);
            if (mob_name != nullptr && mob_name[0] == '\0') mob_name = nullptr;
            if (!is_loot_chest) {
                rarity = ent.get_rarity();
                if (rarity >= RarityID::kNumRarities)
                    rarity = MOB_DATA[mid].rarity;
                if (rarity < RarityID::kNumRarities)
                    rarity_label = I18n::rarity_name(rarity);
            }
        }
    }

    if (show_bar) {
        RenderContext bar(&ctx);
        ctx.set_global_alpha(bar_alpha);
        ctx.set_stroke(0xff222222);
        ctx.set_line_width(9);
        ctx.begin_path();
        ctx.move_to(0, 0);
        ctx.line_to(bar_w, 0);
        ctx.stroke();
        if (ent.healthbar_lag > ent.get_health_ratio()) {
            ctx.set_stroke(0xffed2f31);
            ctx.set_line_width(7);
            ctx.begin_path();
            ctx.move_to(bar_w * ent.get_health_ratio(), 0);
            ctx.line_to(bar_w * ent.healthbar_lag, 0);
            ctx.stroke();
        }
        ctx.set_stroke(0xff75dd34);
        ctx.set_line_width(7);
        ctx.begin_path();
        ctx.move_to(0, 0);
        ctx.line_to(bar_w * ent.get_health_ratio(), 0);
        ctx.stroke();
    }

    // Mob chrome under the bar line: name LEFT, rarity RIGHT (same as
    // player Lv / guild). Small ants have a short bar_w so Chinese name +
    // rarity overlap — expand the label span like player Lv/guild.
    float const under_y = 13.f;
    if (show_mob_label && !is_loot_chest) {
        Renderer *name_baked = nullptr;
        Renderer *rare_baked = nullptr;
        if (mob_name != nullptr)
            name_baked = MobLabelCache::get(1, (uint8_t)mid, 0, mob_name, nullptr, 0xffffffff);
        if (rarity_label != nullptr)
            rare_baked = MobLabelCache::get(2, (uint8_t)mid, rarity, nullptr, rarity_label,
                RARITY_COLORS[rarity]);
        float name_w = name_baked != nullptr ? name_baked->width : 0.f;
        float rare_w = rare_baked != nullptr ? rare_baked->width : 0.f;
        float span = bar_w;
        if (name_w > 0.f && rare_w > 0.f)
            span = std::max(bar_w, name_w + 10.f + rare_w);
        float const x0 = (bar_w - span) * 0.5f;
        if (name_baked != nullptr) {
            RenderContext nm(&ctx);
            ctx.set_global_alpha(label_alpha);
            ctx.draw_image_at(*name_baked, x0, under_y - name_baked->height * 0.5f);
        }
        if (rare_baked != nullptr) {
            RenderContext rare(&ctx);
            ctx.set_global_alpha(label_alpha);
            ctx.draw_image_at(*rare_baked, x0 + span - rare_w, under_y - rare_baked->height * 0.5f);
        }
    }

    if (is_training) {
        uint8_t rarity = ent.get_rarity();
        if (rarity < RarityID::kNumRarities) {
            auto const &data = Game::training_readouts[rarity];
            std::string text = std::format("{} {}  ·  DPS {}",
                I18n::use_chinese ? "单次" : "Hit", data.last, data.dps);
            RenderContext stats(&ctx);
            ctx.translate(bar_w * 0.5f, under_y + 22.f);
            ctx.draw_text(text.c_str(), {.fill = 0xffffe6a5, .stroke = 0xff1b211d, .size = 13});
            ctx.translate(0, 19.f);
            std::string total = std::format("{} {}", I18n::use_chinese ? "我的累计伤害" : "My Total Damage", data.total);
            ctx.draw_text(total.c_str(), {.fill = 0xffe7efff, .stroke = 0xff1b211d, .size = 13});
        }
    }

    if (!show_identity) return;

    // Lv left / guild right under the bar. Small flowers have a short bar_w so
    // Chinese "等级 N" + "[guild]" would overlap — expand the label span.
    RenderContext id(&ctx);
    ctx.set_global_alpha(1 - ent.deletion_animation);
    float const label_y = under_y;
    ctx.set_text_size(12);
    std::string lv_text;
    float lv_tw = 0.f;
    if (ent.has_component(kScore)) {
        uint32_t lv = score_to_level(ent.get_score());
        lv_text = I18n::use_chinese
            ? std::format("等级 {}", lv)
            : std::format("Lv {}", lv);
        lv_tw = ctx.get_text_size(lv_text.c_str());
    }
    std::string g_text;
    float g_tw = 0.f;
    if (!ent.get_guild().empty()) {
        g_text = "[" + ent.get_guild() + "]";
        g_tw = ctx.get_text_size(g_text.c_str());
    }
    float span = bar_w;
    if (lv_tw > 0.f && g_tw > 0.f)
        span = std::max(bar_w, lv_tw + 10.f + g_tw);
    float const x0 = (bar_w - span) * 0.5f;
    if (lv_tw > 0.f) {
        RenderContext left(&ctx);
        ctx.translate(x0 + lv_tw * 0.5f, label_y);
        ctx.draw_text(lv_text.c_str(), {
            .fill = 0xff75dd34,
            .stroke = 0xff000000,
            .size = 12,
            .stroke_scale = 0.14f
        });
    }
    if (g_tw > 0.f) {
        RenderContext right(&ctx);
        ctx.translate(x0 + span - g_tw * 0.5f, label_y);
        ctx.draw_text(g_text.c_str(), {
            .fill = 0xff27dade,
            .stroke = 0xff000000,
            .size = 12,
            .stroke_scale = 0.14f
        });
    }
}

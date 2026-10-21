#include <Client/Render/RenderEntity.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <format>
#include <string>

void render_health(Renderer &ctx, Entity const &ent) {
    if (ent.has_component(kPetal)) return;
    // Worm body segments: only the head shows a bar / name
    if (ent.has_component(kSegmented)) return;

    uint8_t is_player = ent.has_component(kFlower) && !ent.has_component(kMob)
        && ent.has_component(kName);
    uint8_t is_mob = ent.has_component(kMob);
    uint8_t is_self = is_player && Game::simulation.ent_alive(Game::player_id)
        && (ent.id == Game::player_id);
    // Level / guild stay visible at full HP; only the bar fades with opacity.
    uint8_t show_identity = is_player && !is_self;
    // Reward loot chests: always draw an HP bar (walk-up open, but still a
    // combat-looking prop). Other mobs: name+rarity always; bar after damage.
    uint8_t is_loot_chest = is_mob && ent.get_mob_id() == MobID::kLootChest;
    uint8_t show_mob_label = is_mob;
    uint8_t show_bar = is_loot_chest || ent.healthbar_opacity >= 0.01f;
    if (!show_bar && !show_identity && !show_mob_label) return;

    // Skip non-combat props (pads / crates / guide NPC). Loot chests keep a bar.
    if (is_mob) {
        MobID::T mid = ent.get_mob_id();
        if (mid == MobID::kPushCrate || mid == MobID::kYahahaPad
            || mid == MobID::kTreasureNpc)
            return;
    }

    float w = ent.get_radius() * 1.33f;
    float bar_w = 2 * w;
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    // Origin = left end of the HP bar (below the entity)
    ctx.translate(-w, w + 15);
    ctx.round_line_cap();

    float const label_alpha = 1.f - ent.deletion_animation;
    // Loot chests keep a solid bar even at full HP (opacity otherwise fades to 0).
    float const bar_alpha = label_alpha * (is_loot_chest ? 1.f
        : (show_bar ? ent.healthbar_opacity : 1.f));

    if (is_mob) {
        MobID::T mid = ent.get_mob_id();
        if (mid < MobID::kNumMobs) {
            char const *name = I18n::mob_name(mid);
            if (name != nullptr && name[0] != '\0') {
                RenderContext nm(&ctx);
                ctx.set_global_alpha(label_alpha);
                ctx.set_text_size(12);
                float tw = ctx.get_text_size(name);
                ctx.translate(tw * 0.5f, -11.f);
                ctx.draw_text(name, {
                    .fill = 0xffffffff,
                    .stroke = 0xff000000,
                    .size = 12,
                    .stroke_scale = 0.14f
                });
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

    if (is_mob && !is_loot_chest) {
        MobID::T mid = ent.get_mob_id();
        if (mid < MobID::kNumMobs) {
            uint8_t rarity = ent.get_rarity();
            if (rarity >= RarityID::kNumRarities)
                rarity = MOB_DATA[mid].rarity;
            if (rarity < RarityID::kNumRarities) {
                char const *label = I18n::rarity_name(rarity);
                RenderContext rare(&ctx);
                ctx.set_global_alpha(label_alpha);
                ctx.set_text_size(12);
                float tw = ctx.get_text_size(label);
                // Always bottom-right of the HP bar slot (even if bar is hidden).
                ctx.translate(bar_w - tw * 0.5f, 12.f);
                ctx.draw_text(label, {
                    .fill = RARITY_COLORS[rarity] | 0xff000000,
                    .stroke = 0xff000000,
                    .size = 12,
                    .stroke_scale = 0.14f
                });
            }
        }
    }

    if (!show_identity) return;

    RenderContext id(&ctx);
    ctx.set_global_alpha(1 - ent.deletion_animation);
    float label_y = 13.f;
    if (ent.has_component(kScore)) {
        uint32_t lv = score_to_level(ent.get_score());
        std::string lv_text = I18n::use_chinese
            ? std::format("等级 {}", lv)
            : std::format("Lv {}", lv);
        RenderContext left(&ctx);
        ctx.set_text_size(12);
        float tw = ctx.get_text_size(lv_text.c_str());
        ctx.translate(tw * 0.5f, label_y);
        ctx.draw_text(lv_text.c_str(), {
            .fill = 0xff75dd34,
            .stroke = 0xff000000,
            .size = 12,
            .stroke_scale = 0.14f
        });
    }
    if (!ent.get_guild().empty()) {
        std::string g = "[" + ent.get_guild() + "]";
        RenderContext right(&ctx);
        ctx.set_text_size(12);
        float tw = ctx.get_text_size(g.c_str());
        ctx.translate(bar_w - tw * 0.5f, label_y);
        ctx.draw_text(g.c_str(), {
            .fill = 0xff27dade,
            .stroke = 0xff000000,
            .size = 12,
            .stroke_scale = 0.14f
        });
    }
}

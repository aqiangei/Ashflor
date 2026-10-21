#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/StaticData.hh>
#include <Client/I18n.hh>

#include <Client/Game.hh>

#include <format>

using namespace Ui;

Element *Ui::UiLoadout::petal_tooltips[PetalID::kNumPetals] = {nullptr};

static float get_reload_factor() {
    if (!Game::alive()) return 1;
    float factor = 1;
    Entity &player = Game::simulation.get_ent(Game::player_id);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        factor *= PETAL_DATA[player.get_loadout_ids(i)].attributes.extra_reload_factor;
    }
    return factor;
}

static float get_damage_factor() {
    if (!Game::alive()) return 1;
    float factor = 1;
    Entity &player = Game::simulation.get_ent(Game::player_id);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        factor *= PETAL_DATA[player.get_loadout_ids(i)].attributes.extra_damage_factor;
    }
    return factor;
}

static Ui::Element *make_petal_stat_container(PetalID::T id) {
    std::vector<Ui::Element *> stats = {new Ui::Element(0,10)};
    struct PetalData const &petal_data = PETAL_DATA[id];
    struct PetalAttributes const &attrs = petal_data.attributes;
    if (petal_data.health > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHealth); }, { .fill = 0xff77ff77 }),
            new Ui::StaticText(12, format_number(petal_data.health))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (petal_data.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamage); }, { .fill = 0xffff7777 }),
            new Ui::DynamicText(12, [&](){
                return format_number(petal_data.damage * get_damage_factor());
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kArmor); }, { .fill = 0xff777777 }),
            new Ui::StaticText(12, format_number(attrs.armor))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.damage_reflection > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamageReflection); }, { .fill = 0xff777777 }),
            new Ui::StaticText(12, format_pct(attrs.damage_reflection * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.constant_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::StaticText(12, format_number(attrs.constant_heal) + "/s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burst_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::StaticText(12, format_number(attrs.burst_heal))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoison); }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, format_number(attrs.poison_damage.time * attrs.poison_damage.damage) + " (" + format_number(attrs.poison_damage.damage) + "/s)")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_health > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kFlowerHealth); }, { .fill = 0xff77ff77 }),
            new Ui::StaticText(12, format_number(attrs.extra_health))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_body_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBodyDamage); }, { .fill = 0xffff7777 }),
            new Ui::StaticText(12, format_number(attrs.extra_body_damage))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoisonArmor); }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, format_number(attrs.poison_armor) + "/s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_rotation_speed > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kRotationSpeed); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, "+" + format_number(attrs.extra_rotation_speed) + " rad/s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.spawns != MobID::kNumMobs && attrs.spawn_count == 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kContents); }, { .fill = 0xffd2eb34 }),
            new Ui::DynamicText(12, [=](){
                return (petal_data.count > 1 ? format_number(petal_data.count) + "x " : "") + I18n::mob_name(attrs.spawns);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.spawns != MobID::kNumMobs && attrs.spawn_count > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kSpawns); }, { .fill = 0xffd2eb34 }),
            new Ui::DynamicText(12, [=](){
                return format_number(attrs.spawn_count) + "x " + I18n::mob_name(attrs.spawns);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.vision_factor < 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraVision); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, format_pct(100 / attrs.vision_factor))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.move_speed_factor != 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kMoveSpeed); }, { .fill = 0xff3ecf8e }),
            new Ui::StaticText(12, format_pct(attrs.move_speed_factor * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.magnet_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "磁力范围：" : "Magnet Range:";
            }, { .fill = 0xff4a7fd9 }),
            new Ui::StaticText(12, format_number(attrs.magnet_radius))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_range > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kAttackRange); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, "+" + format_number(attrs.extra_range))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.slow_inflict_seconds > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "减速：" : "Slow:";
            }, { .fill = 0xff7ec8e3 }),
            new Ui::StaticText(12, format_number(attrs.slow_inflict_seconds) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.defend_only) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "仅防御态生效" : "Defend only";
            }, { .fill = 0xffaaaaaa })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_damage_factor > 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraDamage); }, { .fill = 0xffff7777 }),
            new Ui::StaticText(12, "+"+format_pct(100 * (attrs.extra_damage_factor - 1)))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_reload_factor > 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraReload); }, { .fill = 0xff7777ff }),
            new Ui::StaticText(12, "+"+format_pct(100 * (attrs.extra_reload_factor - 1)))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    return new Ui::VContainer(stats, 0, 2, { .h_justify = Style::Left });
}

static void make_petal_tooltip(PetalID::T id) {
    Element *tooltip = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(20, [id](){ return I18n::petal_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
            new Ui::DynamicText(16, [=](){
                float reload = PETAL_DATA[id].reload * get_reload_factor();
                if (reload <= 0.f) return std::string{};
                if (PETAL_DATA[id].attributes.secondary_reload == 0)
                    return std::format("{:.1f}s ⟳", reload);
                return std::format("{:.1f} + {:.1f}s ⟳", reload,
                    PETAL_DATA[id].attributes.secondary_reload);
            }, { .fill = 0xffffffff, .v_justify = Style::Top }),
            5, 10, {}
        ),
        new Ui::DynamicText(14, [id](){ return I18n::rarity_name(PETAL_DATA[id].rarity); }, { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        new Ui::DynamicText(12, [id](){ return I18n::petal_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        make_petal_stat_container(id)
    }, 5, 2);
    tooltip->style.fill = 0xb01c2418;
    tooltip->style.round_radius = 12;
    tooltip->refactor();
    Ui::UiLoadout::petal_tooltips[id] = tooltip;
}

void Ui::make_petal_tooltips() {
    for (PetalID::T i = 0; i < PetalID::kNumPetals; ++i)
        make_petal_tooltip(i);
}
#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <algorithm>
#include <cstring>
#include <string>

using namespace Ui;

GalleryMob::GalleryMob(MobID::T id, float w) : 
    Element(w,w,{ .fill=0xff5a9fdb, .stroke_hsv=1, .line_width=3, .round_radius=6, .v_justify=Style::Top }), id(id) {}

void GalleryMob::on_render(Renderer &ctx) {
    Element::on_render(ctx);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, style.round_radius);
    ctx.clip();
    struct MobData const &data = MOB_DATA[id];
    if (id != MobID::kDigger)
        ctx.rotate(-3*M_PI/4);
    if (id == MobID::kBeetle || id == MobID::kMassiveBeetle)
        ctx.translate(-5,0);
    float radius = (data.radius.upper + data.radius.lower) / 2;
    if (radius > width * 0.5) ctx.scale(0.5 * width / radius);
    ctx.scale(0.5);
    draw_static_mob(id, ctx, { .radius = radius, .flower_attrs = { .color = ColorID::kGray } });
    if (data.attributes.segments > 1) {
        ctx.translate(-2 * radius, 0);
        draw_static_mob(id, ctx, { .radius = radius, .flags = 1<<1, .flower_attrs = { .color = ColorID::kGray } });
    }
}

static Element *make_mob_drops(MobID::T id) {
    Element *elt = new Ui::HContainer({}, 0, 6, { .h_justify = Style::Left });
    struct MobData const &data = MOB_DATA[id];
    StaticArray<float, MAX_DROPS_PER_MOB> const &drop_chances = MOB_DROP_CHANCES[id];
    std::vector<uint8_t> order;
    for (uint32_t i = 0; i < data.drops.size(); ++i)
        order.push_back(i);
    
    std::sort(order.begin(), order.end(), [&](uint8_t a, uint8_t b) {
        return drop_chances[a] > drop_chances[b];
    });

    for (uint32_t i = 0; i < data.drops.size(); ++i) {
        uint32_t j = order[i];
        elt->add_child(new Ui::VContainer({
            new GalleryPetal(data.drops[j], 45),
            new StaticText(12, format_pct(drop_chances[j] * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    return elt;
}

static Element *make_mob_stat_container(MobID::T id) {
    std::vector<Ui::Element *> stats;
    struct MobData const &mob_data = MOB_DATA[id];
    struct MobAttributes const &attrs = mob_data.attributes;
    stats.push_back(new Ui::HContainer({
        new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHealth); }, { .fill = 0xff77ff77 }),
        new Ui::StaticText(12, mob_data.health.to_string())
    }, 0, 5, { .h_justify = Style::Left }));
    stats.push_back(new Ui::HContainer({
        new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamage); }, { .fill = 0xffff7777 }),
        new Ui::StaticText(12, format_number(mob_data.damage))
    }, 0, 5, { .h_justify = Style::Left }));
    if (attrs.poison_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoison); }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, format_number(attrs.poison_damage.damage * attrs.poison_damage.time) + " (" + format_number(attrs.poison_damage.damage) + "/s)")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    stats.push_back(new Ui::HContainer({
        new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kXp); }, { .fill = 0xff7777ff }),
        new Ui::StaticText(12, format_score(mob_data.xp))
    }, 0, 5, { .h_justify = Style::Left }));
    return new Ui::VContainer(stats, 0, 2, { .h_justify = Style::Left });
}

static Element *make_mob_card(MobID::T id) {
    Element *elt = new Ui::VContainer({
        new Ui::Element(300,0),
        new Ui::HFlexContainer(
            new Ui::VContainer({
                new Ui::DynamicText(18, [id](){ return I18n::mob_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
                new Ui::DynamicText(14, [id](){ return I18n::rarity_name(MOB_DATA[id].rarity); }, { .fill = RARITY_COLORS[MOB_DATA[id].rarity], .h_justify = Style::Left }),
                new Ui::Element(0,2),
                new Ui::DynamicText(14, [id](){ return I18n::mob_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
                new Ui::DynamicText(12, [](){
                    return I18n::use_chinese
                        ? std::string("属性随地图难度缩放")
                        : std::string("Stats scale with map zone difficulty");
                }, { .fill = 0xffaaaaaa, .h_justify = Style::Left })
            }, 0, 5),
            new GalleryMob(id, 60),
            10, 10
        ),
        new Ui::Element(0,10),
        DEBUG_ONLY(make_mob_stat_container(id),)
        new Ui::Element(0,10),
        make_mob_drops(id)
    }, 10, 0, { .fill = 0x33000000, .stroke_hsv = 1, .line_width = 3, .round_radius = 6, .v_justify = Style::Top, .no_animation = 1 });
    Element *chooser = new Ui::Choose(
        new Ui::VContainer({
            new Ui::Element(300,5),
            new Ui::StaticText(16, "?"),
            new Ui::Element(300,5)
        }, 10, 0, { .fill = 0x33000000, .stroke_hsv = 1, .line_width = 3, .round_radius = 6, .v_justify = Style::Top, .no_animation = 1 }),
        elt,
        [=](){ return Game::seen_mobs[id]; }
    );
    chooser->style.v_justify = Style::Top;
    chooser->style.no_animation = 1;
    return chooser;
}

static Element *make_scroll() {
    Element *elt = new Ui::VContainer({}, 0, 10, {});
    MobID::T id_list[MobID::kNumMobs];
    for (MobID::T i = 0; i < MobID::kNumMobs; ++i)
        id_list[i] = i;
    std::sort(id_list, id_list + MobID::kNumMobs, [](MobID::T a, MobID::T b) {
        if (MOB_DATA[a].rarity < MOB_DATA[b].rarity) return true;
        if (MOB_DATA[b].rarity < MOB_DATA[a].rarity) return false;
        return strcmp(MOB_DATA[a].name, MOB_DATA[b].name) <= 0;
    });

    for (MobID::T i = 0; i < MobID::kNumMobs; ++i) 
        elt->add_child(make_mob_card(id_list[i]));

    return new Ui::ScrollContainer(elt, 300);
}

Element *Ui::make_mob_gallery() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kMobGallery); }),
        make_scroll()
    }, 15, 10, { 
        .fill = Chrome::kPanelDeep,
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            elt->y = Game::alive() ? 170.f : 84.f;
            ctx.translate(0, (elt->animation - 1) * elt->height);
        },
        .should_render = [](){
            if (Ui::panel_open != Panel::kMobs) return false;
            return Game::should_render_title_ui()
                || (Game::should_render_game_ui() && Game::alive());
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 84;
    if (Ui::Panel::mob_gallery == nullptr)
        Ui::Panel::mob_gallery = elt;
    else
        Ui::Panel::mob_gallery_alt = elt;
    return elt;
}
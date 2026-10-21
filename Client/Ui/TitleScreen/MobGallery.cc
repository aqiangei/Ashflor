#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/StaticData.hh>

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <vector>

using namespace Ui;

Element *Ui::mob_tooltips[MobID::kNumMobs] = {nullptr};

GalleryMob::GalleryMob(MobID::T id, float w) : 
    Element(w,w,{ .fill=RARITY_COLORS[MOB_DATA[id].rarity], .stroke_hsv=1, .line_width=3, .round_radius=6, .v_justify=Style::Top }), id(id) {}

static GalleryMob *s_mobile_mob_tip = nullptr;

void GalleryMob::on_render(Renderer &ctx) {
    // Hold-to-inspect: clear sticky tip once the finger lifts (FocusLost may
    // have fired earlier from scroll jitter while the touch was still down).
    if (Input::is_mobile && tip_touch != (uint32_t) -1
        && !Input::touches.contains(tip_touch)) {
        tip_touch = (uint32_t) -1;
        rendering_tooltip = 0;
        if (s_mobile_mob_tip == this) s_mobile_mob_tip = nullptr;
    }
    bool seen = Game::seen_mobs[id] != 0;
    style.fill = seen ? RARITY_COLORS[MOB_DATA[id].rarity] : 0xffaeaa3d;
    style.stroke_hsv = seen ? 1.f : 0.f;
    style.line_width = seen ? 3.f : 0.f;
    Element::on_render(ctx);
    if (!seen) return;
    {
    RenderContext icon(&ctx);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, style.round_radius);
    ctx.clip();
    struct MobData const &data = MOB_DATA[id];
    if (id != MobID::kDigger)
        ctx.rotate(-3*M_PI/4);
    if (id == MobID::kBeetle || id == MobID::kMassiveBeetle || id == MobID::kFlowerGuardian)
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
}

void GalleryMob::on_event(uint8_t event) {
    if (!Game::seen_mobs[id]) {
        rendering_tooltip = 0;
        tip_touch = (uint32_t) -1;
        if (s_mobile_mob_tip == this) s_mobile_mob_tip = nullptr;
        return;
    }
    // Mobile: keep the tip up while the finger is still down. Scroll jitter
    // clears Element::touch_id before kFocusLost, so track tip_touch separately.
    if (Input::is_mobile) {
        if (event == kMouseDown || event == kMouseHover) {
            if (s_mobile_mob_tip != nullptr && s_mobile_mob_tip != this) {
                s_mobile_mob_tip->rendering_tooltip = 0;
                s_mobile_mob_tip->tip_touch = (uint32_t) -1;
            }
            s_mobile_mob_tip = this;
            tip_touch = touch_id;
            rendering_tooltip = 1;
            tooltip = Ui::mob_tooltips[id];
            return;
        }
        if (event == kFocusLost || event == kClick) {
            if (event == kFocusLost && tip_touch != (uint32_t) -1
                && Input::touches.contains(tip_touch)) {
                rendering_tooltip = 1;
                tooltip = Ui::mob_tooltips[id];
                return;
            }
            tip_touch = (uint32_t) -1;
            rendering_tooltip = 0;
            if (s_mobile_mob_tip == this) s_mobile_mob_tip = nullptr;
            return;
        }
    }
    if (event != kFocusLost) {
        rendering_tooltip = 1;
        tooltip = Ui::mob_tooltips[id];
    } else
        rendering_tooltip = 0;
}

static Element *make_mob_drops(MobID::T id, float petal_size = 40) {
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
        PetalID::T drop = data.drops[j];
        if (drop == PetalID::kNone || drop >= PetalID::kNumPetals) continue;
        float chance = j < drop_chances.size() ? drop_chances[j] : 0.f;
        elt->add_child(new Ui::VContainer({
            // always_show: bestiary should list drops even before the petal is owned
            new GalleryPetal(drop, petal_size, 1),
            new StaticText(12, format_pct(chance * 100))
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

static void make_mob_tooltip(MobID::T id) {
    std::vector<Ui::Element *> children = {
        new Ui::DynamicText(20, [id](){ return I18n::mob_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [id](){ return I18n::rarity_name(MOB_DATA[id].rarity); }, { .fill = RARITY_COLORS[MOB_DATA[id].rarity], .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        new Ui::DynamicText(12, [id](){ return I18n::mob_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        make_mob_stat_container(id),
    };
    if (MOB_DATA[id].drops.size() > 0) {
        children.push_back(new Ui::Element(0, 8));
        children.push_back(new Ui::DynamicText(12, [](){
            return I18n::use_chinese ? std::string("掉落") : std::string("Drops");
        }, { .fill = 0xffffffff, .h_justify = Style::Left }));
        children.push_back(make_mob_drops(id, 36));
    }
    Element *tooltip = new Ui::VContainer(children, 5, 2);
    tooltip->style.fill = 0xb01c2418;
    tooltip->style.round_radius = 12;
    tooltip->refactor();
    Ui::mob_tooltips[id] = tooltip;
}

void Ui::make_mob_tooltips() {
    for (MobID::T i = 0; i < MobID::kNumMobs; ++i)
        make_mob_tooltip(i);
}

static Element *make_mob_card(MobID::T id) {
    Element *elt = new Ui::VContainer({
        new Ui::Element(300,0),
        new Ui::HFlexContainer(
            new Ui::VContainer({
                new Ui::DynamicText(18, [id](){ return I18n::mob_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
                new Ui::DynamicText(14, [id](){ return I18n::rarity_name(MOB_DATA[id].rarity); }, { .fill = RARITY_COLORS[MOB_DATA[id].rarity], .h_justify = Style::Left }),
                new Ui::Element(0,2),
                new Ui::DynamicText(14, [id](){ return I18n::mob_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left })
            }, 0, 5),
            new GalleryMob(id, 60),
            10, 10
        ),
        new Ui::Element(0,10),
        make_mob_stat_container(id),
        new Ui::Element(0,10),
        make_mob_drops(id, 45)
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
    // Desktop was 110×1050 — oversized on 1080p; keep mobile compact.
    float const kCell = Input::is_mobile ? 46.f : 72.f;
    constexpr uint16_t kEmpty = 0xffffu;
    struct Family {
        char const *name;
        std::array<uint16_t, RarityID::kNumRarities> ids;
    };
    std::vector<Family> families;
    for (uint16_t id = 0; id < MobID::kNumMobs; ++id) {
        char const *name = MOB_DATA[id].name;
        uint8_t rarity = MOB_DATA[id].rarity;
        if (rarity >= RarityID::kNumRarities) continue;
        Family *target = nullptr;
        for (Family &family : families)
            if (std::strcmp(family.name, name) == 0 && family.ids[rarity] == kEmpty) {
                target = &family;
                break;
            }
        if (!target) {
            Family family{name, {}};
            family.ids.fill(kEmpty);
            families.push_back(family);
            target = &families.back();
        }
        target->ids[rarity] = id;
    }
    std::vector<Element *> rows;
    for (Family const &family : families) {
        std::vector<Element *> cards;
        for (uint16_t id : family.ids)
            cards.push_back(id == kEmpty
                ? static_cast<Element *>(new Element(kCell, kCell,
                    { .fill = 0xffaeaa3d, .round_radius = 5.f }))
                : static_cast<Element *>(new GalleryMob((MobID::T)id, kCell)));
        rows.push_back(new Ui::HContainer(cards, 0, 5,
            { .h_justify = Style::Left, .no_animation = 1 }));
    }
    return new Ui::ScrollContainer(new Ui::VContainer(rows, 0, 5,
        { .h_justify = Style::Left, .no_animation = 1 }),
        Input::is_mobile ? 360.f : 400.f);
}

class MobGalleryHeader final : public Ui::Container {
public:
    MobGalleryHeader() : Ui::Container({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kMobGallery); },
            { .fill = Chrome::kTitle }),
        new Ui::Button(34, 34,
            new Ui::DynamicText(22, [](){ return std::string{"×"}; }),
            [](Element *, uint8_t e){ if (e == Ui::kClick) Ui::panel_open = Panel::kNone; },
            nullptr, { .fill = Chrome::kBtnDanger, .round_radius = 6 })
    }, Input::is_mobile ? 474.f : 720.f, 38, { .no_animation = 1 }) {}
    void refactor() override {
        for (Element *child : children) child->refactor();
        width = Input::is_mobile ? 474.f : 720.f;
        height = 38;
        children[0]->x = 0;
        children[1]->x = (width - children[1]->width) / 2 - 2;
    }
};

Element *Ui::make_mob_gallery() {
    Element *elt = new Ui::VContainer({
        new MobGalleryHeader(),
        make_scroll()
    }, 12, 10, {
        .fill = 0xffd8d94c,
        .line_width = Chrome::kRim,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            elt->y = Game::alive() ? 85.f : 84.f;
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

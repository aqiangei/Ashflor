#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/StaticData.hh>
#include <Client/Assets/Assets.hh>

#include <Shared/Craft.hh>

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

using namespace Ui;

GalleryPetal::GalleryPetal(PetalID::T id, float w, uint8_t always_show) : 
    Element(w,w,{ .fill = Chrome::kPanelInset, .round_radius = w/12 , .h_justify = Style::Left }),
    id(id), always_show(always_show) {}

void GalleryPetal::on_render(Renderer &ctx) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) {
        style.fill = 0xffaeaa3d;
        Element::on_render(ctx);
        return;
    }
    uint8_t known = always_show || Game::seen_petals[id];
    if (!known) {
        style.fill = 0xffaeaa3d;
        Element::on_render(ctx);
        return;
    }
    // Uncollected drops in the bestiary still show the petal, slightly dimmed.
    if (always_show && !Game::seen_petals[id])
        ctx.set_global_alpha(0.55f);
    ctx.scale(width / 60);
    draw_loadout_background(ctx, id);
    if (always_show && !Game::seen_petals[id])
        ctx.set_global_alpha(1.f);
}

void GalleryPetal::on_event(uint8_t event) {
    if (event != kFocusLost && id != PetalID::kNone && id < PetalID::kNumPetals
        && (always_show || Game::seen_petals[id])) {
        rendering_tooltip = 1;
        // Gallery: show leveled stats when this Unique has an account level.
        if (Craft::can_level_petal(id) && Craft::petal_level_get(Game::petal_levels, id) > 0
            && Ui::UiLoadout::petal_tooltips_leveled[id] != nullptr)
            tooltip = Ui::UiLoadout::petal_tooltips_leveled[id];
        else
            tooltip = Ui::UiLoadout::petal_tooltips[id];
    } else
        rendering_tooltip = 0;
}

PetalsCollectedIndicator::PetalsCollectedIndicator(float w) : Element(w,w,{}) {}

void PetalsCollectedIndicator::on_render(Renderer &ctx) {
    uint32_t colct = 0;
    uint32_t totct = 0;
    for (PetalID::T i = PetalID::kBasic; i < PetalID::kNumPetals; ++i) {
        colct += Game::seen_petals[i] != 0;
        ++totct;
    }
    ctx.set_fill(0x80000000);
    ctx.begin_path();
    ctx.arc(0,0,width/2);
    ctx.fill();
    ctx.set_fill(0xc0eeeeee);
    ctx.begin_path();
    ctx.move_to(0,0);
    ctx.partial_arc(0,0,width/2*0.8,-M_PI/2,-M_PI/2+2*M_PI*colct/totct,0);
    ctx.close_path();
    ctx.fill();
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
    for (uint16_t id = PetalID::kBasic; id < PetalID::kNumPetals; ++id) {
        char const *name = PETAL_DATA[id].name;
        uint8_t rarity = PETAL_DATA[id].rarity;
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
                : static_cast<Element *>(new GalleryPetal((PetalID::T)id, kCell)));
        rows.push_back(new Ui::HContainer(cards, 0, 5,
            { .h_justify = Style::Left, .no_animation = 1 }));
    }
    return new Ui::ScrollContainer(new Ui::VContainer(rows, 0, 5,
        { .h_justify = Style::Left, .no_animation = 1 }),
        Input::is_mobile ? 360.f : 400.f);
}

class PetalGalleryHeader final : public Ui::Container {
public:
    PetalGalleryHeader() : Ui::Container({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kPetalGallery); },
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

Element *Ui::make_petal_gallery() {
    Element *elt = new Ui::VContainer({
        new PetalGalleryHeader(),
        make_scroll()
    }, 12, 10, {
        .fill = Chrome::kPanelDeep,
        .line_width = Chrome::kRim,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate(0, (elt->animation - 1) * elt->height);
        },
        .should_render = [](){
            if (Ui::panel_open != Panel::kPetals) return false;
            return Game::should_render_title_ui()
                || (Game::should_render_game_ui() && Game::alive());
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 84;
    if (Ui::Panel::petal_gallery == nullptr)
        Ui::Panel::petal_gallery = elt;
    else
        Ui::Panel::petal_gallery_alt = elt;
    return elt;
}

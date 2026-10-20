#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>
#include <Client/Assets/Assets.hh>

#include <algorithm>
#include <cstring>

using namespace Ui;

GalleryPetal::GalleryPetal(PetalID::T id, float w) : 
    Element(w,w,{ .fill = Chrome::kPanelInset, .round_radius = w/12 , .h_justify = Style::Left }), id(id) {}

void GalleryPetal::on_render(Renderer &ctx) {
    if (!Game::seen_petals[id]) {
        Element::on_render(ctx);
        ctx.draw_text("?", { .fill = 0xffeeeeee, .size = width / 2, .stroke_scale = 0});
    } else {
        ctx.scale(width / 60);
        draw_loadout_background(ctx, id);
    }
}

void GalleryPetal::on_event(uint8_t event) {
    if (event != kFocusLost && id != PetalID::kNone && Game::seen_petals[id]) {
        rendering_tooltip = 1;
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
    std::vector<Element *> rows;

    PetalID::T id_list[PetalID::kNumPetals];
    for (PetalID::T i = 0; i < PetalID::kNumPetals; ++i)
        id_list[i] = i;
    std::sort(id_list, id_list + PetalID::kNumPetals, [](PetalID::T a, PetalID::T b){
        if (a == PetalID::kNone) return true;
        if (b == PetalID::kNone) return false;
        if (PETAL_DATA[a].rarity < PETAL_DATA[b].rarity) return true;
        if (PETAL_DATA[a].rarity > PETAL_DATA[b].rarity) return false;
        return strcmp(PETAL_DATA[a].name, PETAL_DATA[b].name) <= 0;
    });
    for (PetalID::T i = PetalID::kBasic; i < PetalID::kNumPetals;) {
        std::vector<Element *> cards;
        for (uint8_t j = 0; j < 9 && i < PetalID::kNumPetals; ++j, ++i) {
            cards.push_back(new GalleryPetal(id_list[i], 64));
        }
        rows.push_back(new Ui::HContainer(cards, 0, 5,
            { .h_justify = Style::Left, .no_animation = 1 }));
    }
    return new Ui::ScrollContainer(new Ui::VContainer(rows, 0, 5,
        { .h_justify = Style::Left, .no_animation = 1 }), 470);
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
    }, 616, 38, { .no_animation = 1 }) {}
    void refactor() override {
        for (Element *child : children) child->refactor();
        width = 616;
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
            return Ui::panel_open == Panel::kPetals && Game::should_render_title_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 84;
    Ui::Panel::petal_gallery = elt;
    return elt;
}

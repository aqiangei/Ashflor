#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Ui/Container.hh>
#include <Shared/Entity.hh>
#include <Shared/Talent.hh>

#include <cmath>
#include <format>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

class RotationWheel final : public Element {
public:
    RotationWheel() : Element(72, 72, { .no_animation = 1 }) {
        style.should_render = [](){ return Game::alive(); };
    }
    void on_render(Renderer &ctx) override {
        float r = width * 0.42f;
        float t = (float) Game::rot_speed_setting / (float) Talent::kRotSettingMax;
        ctx.set_fill(0xc0222222);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.fill();
        ctx.set_stroke(0xff66c2ff);
        ctx.set_line_width(7);
        ctx.begin_path();
        ctx.partial_arc(0, 0, r * 0.78f, -M_PI / 2, -M_PI / 2 + 2 * M_PI * t, 0);
        ctx.stroke();
        // Spinning tick mark
        float ang = (float) (Game::timestamp * 0.004 * (0.6 + t * 1.4));
        ctx.set_stroke(0xffffffff);
        ctx.set_line_width(3);
        ctx.round_line_cap();
        ctx.begin_path();
        ctx.move_to(cosf(ang) * r * 0.25f, sinf(ang) * r * 0.25f);
        ctx.line_to(cosf(ang) * r * 0.72f, sinf(ang) * r * 0.72f);
        ctx.stroke();
        std::string label = std::format("{}", (int) Game::rot_speed_setting);
        ctx.draw_text(label.c_str(), { .size = 16 });
        ctx.translate(0, height * 0.42f);
        ctx.draw_text("Q·E", { .size = 11, .fill = 0xffdddddd });
    }
};

Element *Ui::make_rotation_wheel() {
    return new RotationWheel();
}

UiLoadoutSlot *Ui::UiLoadout::petal_backgrounds[2 * MAX_SLOT_COUNT + 1] = {nullptr};

UiLoadoutSlot::UiLoadoutSlot(uint8_t pos) : Element(70, 70, { .fill = 0xffeeeeee, .round_radius = 0 }) {
    // Secondary row: smaller slots, still 1:1 under primary
    if (pos >= MAX_SLOT_COUNT && pos < 2 * MAX_SLOT_COUNT)
        width = height = 48;
    style.line_width = width / 12;
    style.round_radius = width / 20;
    style.should_render = [=](){
        if (pos < MAX_SLOT_COUNT)
            return pos < Game::loadout_count;
        if (pos < 2 * MAX_SLOT_COUNT)
            return (pos - MAX_SLOT_COUNT) < Game::loadout_count;
        return true; // delete slot
    };
    position = pos;
    Ui::UiLoadout::petal_backgrounds[pos] = this;
}

void UiLoadoutSlot::on_render(Renderer &ctx) {
    Element::on_render(ctx);
    if (Game::slot_indicator_opacity < 0.01) return;
    if (position < Game::loadout_count) {
        ctx.set_global_alpha(Game::slot_indicator_opacity);
        ctx.translate(0, -height / 2 - 15);
        char str[4] = {'[', SLOT_KEYBINDS[position], ']', '\0'};
        ctx.draw_text(&str[0], { .size = 16 });
    } else if (position == 2 * MAX_SLOT_COUNT) {
        ctx.set_global_alpha(Game::slot_indicator_opacity);
        ctx.translate(0, -height / 2 - 14);
        ctx.draw_text("[T]", { .size = 14 });
        ctx.translate(0, height / 2 + 14);
    }
}

UiDeleteSlot::UiDeleteSlot() : UiLoadoutSlot(2 * MAX_SLOT_COUNT) {
    width = height = 56;
    style.fill = 0xffcf8888;
    style.line_width = width / 12;
    style.round_radius = width / 20;
    delete_text_opacity.set(0);
    style.animate = [&](Element *, Renderer &) {
        delete_text_opacity.set(Ui::UiLoadout::num_petals_selected != 0 || Ui::UiLoadout::selected_with_keys != MAX_SLOT_COUNT);
        delete_text_opacity.step(Ui::lerp_amount);
    };
}

void UiDeleteSlot::on_render(Renderer &ctx) {
    UiLoadoutSlot::on_render(ctx);
    if ((float) delete_text_opacity > 0.01) {
        ctx.set_global_alpha((float) delete_text_opacity);
        ctx.draw_text(I18n::tr(I18n::kDelete), { .size = height / 4});
    }
}

Element *Ui::make_loadout_backgrounds() {
    // Primary row + trash glued to its right
    Element *primary_row = new Ui::HContainer({
        new Ui::HContainer(
            Ui::make_range(0, MAX_SLOT_COUNT, [](uint32_t i){
                return (Element *) (new Ui::UiLoadoutSlot((uint8_t) i));
            }),
            0, 12, { .layer = 1 }
        ),
        new Ui::UiDeleteSlot()
    }, 5, 10, { .layer = 1 });

    // Smaller secondary slots, centered under primary (same column gap via pad)
    // Primary slot pitch = 70+12=82; secondary 48 + gap → center under each
    // Use same outer structure width: pad secondary so columns align under primary centers
    float primary_pitch = 70.f + 12.f;
    float secondary_size = 48.f;
    float secondary_gap = primary_pitch - secondary_size; // 34
    Element *secondary_row = new Ui::HContainer(
        Ui::make_range(MAX_SLOT_COUNT, 2 * MAX_SLOT_COUNT, [](uint32_t i){
            return (Element *) (new Ui::UiLoadoutSlot((uint8_t) i));
        }),
        5 + (70.f - 48.f) / 2.f, // indent so first secondary centers under first primary
        secondary_gap,
        { .layer = 1 }
    );

    Element *slots_col = new Ui::VContainer(
        {
            primary_row,
            secondary_row,
            new Ui::InputFreeze(),
            new Ui::Element(0, 34, {
                .should_render = [](){ return Input::keyboard_movement || Input::is_mobile; }
            })
        }, 0, 4, {}
    );

    Element *base = new Ui::HContainer(
        { slots_col, make_rotation_wheel() },
        0, 14,
        { .should_render = [](){ return Game::alive(); } }
    );
    base->style.v_justify = Style::Bottom;
    base->style.animate = [](Element *elt, Renderer &ctx) {
        ctx.translate(0, (1 - elt->animation) * elt->height * 2);
    };
    return base;
}

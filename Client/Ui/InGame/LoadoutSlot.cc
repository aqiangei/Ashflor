#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Render/Renderer.hh>
#include <Client/StaticData.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/Extern.hh>
#include <Shared/Entity.hh>
#include <Shared/LoadoutPreset.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <format>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

class RotationWheel final : public Element {
    uint32_t hold_touch = (uint32_t)-1;
    uint8_t dragging = 0;

    void apply_pointer(float px, float py) {
        float dx = (px - screen_x) / Ui::scale;
        float dy = (py - screen_y) / Ui::scale;
        float dist = std::sqrt(dx * dx + dy * dy);
        float r = width * 0.42f;
        if (dist < r * 0.18f) return; // dead zone: keep current
        // Angle from top (−π/2), clockwise → 0..1 matching partial_arc fill
        float ang = std::atan2(dy, dx);
        float t = (ang + (float) M_PI / 2.f) / (float)(2.0 * M_PI);
        if (t < 0.f) t += 1.f;
        if (t >= 1.f) t -= 1.f;
        Game::set_rot_speed_from_frac(t);
    }

    void poll_hold_adjust() {
        float px = 0, py = 0;
        uint8_t active = 0;
        if (Input::is_mobile) {
            auto it = Input::touches.find(hold_touch);
            if (it != Input::touches.end()) {
                px = it->second.x;
                py = it->second.y;
                active = 1;
            } else {
                hold_touch = (uint32_t)-1;
                dragging = 0;
            }
        } else if (dragging && BitMath::at(Input::mouse_buttons_state, Input::LeftMouse)) {
            px = Input::mouse_x;
            py = Input::mouse_y;
            active = 1;
        } else {
            dragging = 0;
        }
        if (active)
            apply_pointer(px, py);
    }

public:
    RotationWheel() : Element(Input::is_mobile ? 96 : 72, Input::is_mobile ? 96 : 72, { .no_animation = 1 }) {
        style.should_render = [](){ return Game::alive(); };
    }

    void on_event(uint8_t e) override {
        if (e == kMouseDown) {
            dragging = 1;
            if (Input::is_mobile)
                hold_touch = touch_id;
            else {
                apply_pointer(Input::mouse_x, Input::mouse_y);
            }
        } else if (e == kFocusLost || e == kClick) {
            dragging = 0;
            hold_touch = (uint32_t)-1;
        }
    }

    void on_render(Renderer &ctx) override {
        poll_hold_adjust();

        float r = width * 0.42f;
        float t = Game::rot_speed_f / (float) Talent::kRotSettingMax;
        float hint = Game::rot_hint_opacity;
        float pulse = 1.f + 0.08f * hint * std::sin((float) Game::timestamp * 0.012f);

        // Base disc
        ctx.set_fill(0xc0222222);
        ctx.begin_path();
        ctx.arc(0, 0, r * pulse);
        ctx.fill();

        // Track ring
        ctx.set_stroke(0x66ffffff);
        ctx.set_line_width(6);
        ctx.begin_path();
        ctx.arc(0, 0, r * 0.78f);
        ctx.stroke();

        // Filled arc = current spin (smooth)
        ctx.set_stroke(hint > 0.05f ? 0xff7ad4ff : 0xff66c2ff);
        ctx.set_line_width(7 + 3 * hint);
        ctx.begin_path();
        ctx.partial_arc(0, 0, r * 0.78f, -M_PI / 2, -M_PI / 2 + 2 * M_PI * t, 0);
        ctx.stroke();

        // Outer hint glow while adjusting
        if (hint > 0.02f) {
            ctx.set_global_alpha(hint * 0.55f);
            ctx.set_stroke(0xffa8e6ff);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, r * 1.05f);
            ctx.stroke();
            ctx.set_global_alpha(1.f);
        }

        // Spinning tick — angular speed tracks setting (matches petal spin feel)
        float spin = (float) (Game::timestamp * 0.0035 * (0.35 + t * 1.85));
        ctx.set_stroke(0xffffffff);
        ctx.set_line_width(3);
        ctx.round_line_cap();
        ctx.begin_path();
        ctx.move_to(cosf(spin) * r * 0.22f, sinf(spin) * r * 0.22f);
        ctx.line_to(cosf(spin) * r * 0.72f, sinf(spin) * r * 0.72f);
        ctx.stroke();

        std::string label = std::format("{:.0f}", Game::rot_speed_f);
        ctx.draw_text(label.c_str(), { .size = Input::is_mobile ? 18.f : 16.f });
        ctx.translate(0, height * 0.40f);
        if (Input::is_mobile)
            ctx.draw_text(I18n::use_chinese ? "拖环" : "Drag", { .fill = 0xffdddddd, .size = 11 });
        else
            ctx.draw_text("Q·E", { .fill = 0xffdddddd, .size = 11 });
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
    style.line_width = 0;
    style.round_radius = width * 0.22f;
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

void UiLoadoutSlot::on_event(uint8_t event) {
    // Empty primaries have no petal widget — pick mode lands here.
    if (event != kClick && event != kMouseDown) return;
    if (!Game::mobile_l_preset_pick) return;
    if (position >= Game::loadout_count) return;
    Game::try_mobile_l_preset_slot(position);
}

void UiLoadoutSlot::on_render(Renderer &ctx) {
    // Rarity-tinted slot (zorr / florr_clone loadout bar)
    uint32_t saved_fill = style.fill;
    if (position < 2 * MAX_SLOT_COUNT) {
        // position is dynamic UI index; secondary lives at [n,2n) in cache
        uint8_t static_i = loadout_dynamic_to_static(position);
        PetalID::T pid = PetalID::kNone;
        if (static_i < 2 * MAX_SLOT_COUNT)
            pid = Game::cached_loadout[static_i];
        if (pid != PetalID::kNone && pid < PetalID::kNumPetals) {
            uint8_t r = PETAL_DATA[pid].rarity;
            if (r < RarityID::kNumRarities)
                style.fill = RARITY_COLORS[r];
        }
    }
    Element::on_render(ctx);
    style.fill = saved_fill;
    // Hold K/L: translucent first petal of that bank's preset on each primary slot
    // (identifier only — does not change equipped petals).
    if (Game::preset_ghost_bank != 0 && position < Game::loadout_count) {
        PetalID::T ghost = Game::preset_ghost_petal(position);
        if (ghost != PetalID::kNone && ghost < PetalID::kNumPetals) {
            ctx.set_global_alpha(0.38f);
            {
                RenderContext rc(&ctx);
                float s = width / 60.f;
                if (s < 0.35f) s = 0.35f;
                ctx.scale(s);
                draw_static_petal(ghost, ctx);
            }
            ctx.set_global_alpha(1.f);
            // Soft ring so empty slots still read as "preview"
            ctx.set_stroke(Game::preset_ghost_bank == 1 ? 0xa0a8d8ff : 0xa0ffe08a);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.round_rect(-width / 2 - 1, -height / 2 - 1, width + 2, height + 2,
                style.round_radius + 1);
            ctx.stroke();
        }
    }
    // Hover highlight while dragging a petal over this slot
    if (Ui::UiLoadout::num_petals_selected != 0) {
        float dist = std::max(fabsf(Input::mouse_x - screen_x), fabsf(Input::mouse_y - screen_y));
        if (dist < (width + 8) * Ui::scale / 2) {
            ctx.set_global_alpha(0.18f);
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, style.round_radius);
            ctx.fill();
            ctx.set_global_alpha(1.f);
        }
    }
    // L-pick: gold ring on primary slots
    if (Game::mobile_l_preset_pick && position < Game::loadout_count) {
        ctx.set_stroke(0xffffe08a);
        ctx.set_line_width(3.5f);
        ctx.begin_path();
        ctx.round_rect(-width / 2 - 2, -height / 2 - 2, width + 4, height + 4, style.round_radius + 2);
        ctx.stroke();
    }
    if (Game::mobile_l_preset_pick && position < Game::loadout_count) {
        ctx.set_global_alpha(1.f);
        ctx.translate(0, -height / 2 - 16);
        // L1..L9, L0 (matches digit keybinds)
        char const *lab = position == 9 ? "L0" : nullptr;
        std::string lab_buf;
        if (lab == nullptr) {
            lab_buf = std::format("L{}", position + 1);
            lab = lab_buf.c_str();
        }
        ctx.draw_text(lab, { .fill = 0xffffe08a, .size = 15 });
        return;
    }
    if (Game::slot_indicator_opacity < 0.01) return;
    if (position < Game::loadout_count) {
        ctx.set_global_alpha(Game::slot_indicator_opacity);
        ctx.translate(0, -height / 2 - 15);
        char str[4] = {'[', SLOT_KEYBINDS[position], ']', '\0'};
        ctx.draw_text(&str[0], { .size = 16 });
    } else if (position == 2 * MAX_SLOT_COUNT) {
        // Drop-on-ground slot (T still unequips selected secondary)
        ctx.set_global_alpha(Game::slot_indicator_opacity);
        ctx.translate(0, -height / 2 - 14);
        ctx.draw_text("[D]", { .size = 14 });
        ctx.translate(0, height / 2 + 14);
    }
}

UiDeleteSlot::UiDeleteSlot() : UiLoadoutSlot(2 * MAX_SLOT_COUNT) {
    width = height = 56;
    style.fill = 0xffcf8888;
    style.line_width = 0;
    style.round_radius = width * 0.22f;
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

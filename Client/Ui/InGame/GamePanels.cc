#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Render/Renderer.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/StaticData.hh>

#include <Helpers/Bits.hh>

#include <Shared/Binary.hh>
#include <Shared/Craft.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Ui;

static bool petal_equipped(PetalID::T id) {
    if (id == PetalID::kNone) return false;
    for (uint32_t i = 0; i < Game::loadout_count; ++i)
        if (Game::cached_loadout[i] == id) return true;
    return false;
}

// Layout: primary [0,n) · secondary 副槽 [n, 2n) · pure bag [2n, n+MAX)
// Bag panel only shows pure bag — never secondary.

static uint8_t bag_pure_count() {
    if (Game::loadout_count >= MAX_SLOT_COUNT) return 0;
    return (uint8_t) (MAX_SLOT_COUNT - Game::loadout_count);
}

static uint8_t bag_slot_static(uint8_t bag_index) {
    return (uint8_t) (2 * Game::loadout_count + bag_index);
}

static PetalID::T bag_slot_id(uint8_t bag_index) {
    uint8_t sp = bag_slot_static(bag_index);
    if (sp >= Game::loadout_count + MAX_SLOT_COUNT || sp >= 2 * MAX_SLOT_COUNT)
        return PetalID::kNone;
    return Game::cached_loadout[sp];
}

// Count identical petals in pure backpack only (exclude secondary)
static uint32_t bag_stack_count(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    uint32_t n = 0;
    uint8_t count = bag_pure_count();
    for (uint8_t i = 0; i < count; ++i) {
        if (bag_slot_id(i) == id) ++n;
    }
    return n;
}

// Only the first bag index of a stack is shown (duplicates collapse)
static uint8_t bag_is_stack_head(uint8_t bag_index) {
    PetalID::T id = bag_slot_id(bag_index);
    if (id == PetalID::kNone) return 0;
    for (uint8_t i = 0; i < bag_index; ++i) {
        if (bag_slot_id(i) == id) return 0;
    }
    return 1;
}

// Bag grid cell — click = equip to bottom bar; drag = drop onto bottom equip slots
class InventoryBagSlot final : public Element {
    uint8_t bag_index; // 0 .. bag_pure_count()-1 → static 2*loadout_count + bag_index
    uint8_t dragging;
    uint32_t drag_touch;
    float press_x, press_y;
public:
    InventoryBagSlot(uint8_t idx)
        : Element(64, 72, { .no_animation = 1 }),
          bag_index(idx), dragging(0), drag_touch((uint32_t)-1),
          press_x(0), press_y(0) {
        style.should_render = [idx](){
            return Game::alive() && idx < bag_pure_count() && bag_is_stack_head(idx);
        };
        style.animate = [this](Element *, Renderer &) {
            if (!dragging) return;
            float mx = Input::mouse_x;
            float my = Input::mouse_y;
            uint8_t released = 0;
            if (Input::is_mobile) {
                auto iter = Input::touches.find(drag_touch);
                if (iter == Input::touches.end())
                    released = drag_touch != (uint32_t)-1;
                else { mx = iter->second.x; my = iter->second.y; }
            } else {
                released = BitMath::at(Input::mouse_buttons_released, Input::LeftMouse);
            }
            uint8_t static_pos = bag_slot_static(bag_index);
            if (static_pos >= Game::loadout_count + MAX_SLOT_COUNT
                || static_pos >= 2 * MAX_SLOT_COUNT) { dragging = 0; return; }
            UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[static_pos];
            float dist = std::hypot(mx - press_x, my - press_y);
            constexpr float kClickSlop = 14.f;
            if (released) {
                dragging = 0;
                drag_touch = (uint32_t)-1;
                if (dist < kClickSlop) {
                    if (p != nullptr && p->selected) {
                        --Ui::UiLoadout::num_petals_selected;
                        p->selected = 0;
                        p->style.layer = 0;
                        p->persistent_touch_id = (uint32_t)-1;
                    }
                    // Primary L→R, then secondary L→R; fly if equipped, silent if full
                    ui_auto_equip_from_bag(static_pos, screen_x, screen_y);
                    return;
                }
                if (p != nullptr && p->selected) {
                    uint8_t tgt = find_loadout_drop_target(mx, my);
                    if (tgt != (uint8_t)-1) {
                        if (tgt == 2 * MAX_SLOT_COUNT)
                            ui_delete_petal(static_pos);
                        else {
                            uint8_t st = loadout_dynamic_to_static(tgt);
                            if (st != static_pos)
                                ui_swap_petals(st, static_pos);
                        }
                    }
                    if (p->selected) {
                        --Ui::UiLoadout::num_petals_selected;
                        p->selected = 0;
                    }
                    p->style.layer = 0;
                    p->persistent_touch_id = (uint32_t)-1;
                }
                return;
            }
            if (dist >= kClickSlop && p != nullptr
                && Game::cached_loadout[static_pos] != PetalID::kNone) {
                if (!p->selected)
                    begin_petal_drag(static_pos, drag_touch);
                p->style.layer = 1;
                p->x = (mx - Ui::window_width / 2) / Ui::scale;
                p->y = (my - Ui::window_height / 2) / Ui::scale;
            }
        };
    }
    void refactor() override {
        visible = style.should_render();
        width = 64.f;
        height = 72.f;
    }
    uint8_t slot_static() const {
        return bag_slot_static(bag_index);
    }
    PetalID::T id() const {
        return bag_slot_id(bag_index);
    }
    void on_render(Renderer &ctx) override {
        PetalID::T pid = id();
        ctx.set_fill(0xff3ecf6e);
        ctx.set_stroke(0xff2a9a4a);
        ctx.set_line_width(1.5f);
        ctx.begin_path();
        ctx.round_rect(-width / 2, -height / 2, width, height, 4);
        ctx.fill();
        ctx.stroke();
        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) return;
        if (dragging) ctx.set_global_alpha(0.4f);
        {
            RenderContext c(&ctx);
            ctx.translate(0, -8);
            // Half previous bag icon size (was 96/60)
            ctx.scale(48.f / 60.f);
            draw_loadout_background(ctx, pid);
        }
        uint32_t stack = bag_stack_count(pid);
        if (stack > 1) {
            std::string badge = std::format("x{}", stack);
            float bx = width / 2 - 12;
            float by = -height / 2 + 10;
            ctx.set_fill(0xe0000000);
            ctx.begin_path();
            ctx.round_rect(bx - 11, by - 8, 22, 16, 4);
            ctx.fill();
            {
                RenderContext c(&ctx);
                ctx.translate(bx, by);
                ctx.center_text_align();
                ctx.center_text_baseline();
                ctx.draw_text(badge.c_str(), { .fill = 0xffffffff, .size = 11, .stroke_scale = 0 });
            }
        }
        ctx.translate(0, height / 2 - 10);
        ctx.set_fill(0xffffffff);
        ctx.draw_text(PETAL_DATA[pid].name, { .size = 10 });
        ctx.translate(0, -(height / 2 - 10));
    }
    void on_event(uint8_t event) override {
        PetalID::T pid = id();
        if (event == kMouseDown && pid != PetalID::kNone) {
            dragging = 1;
            drag_touch = touch_id;
            press_x = Input::mouse_x;
            press_y = Input::mouse_y;
            if (Input::is_mobile) {
                auto iter = Input::touches.find(touch_id);
                if (iter != Input::touches.end()) {
                    press_x = iter->second.x;
                    press_y = iter->second.y;
                }
            }
            rendering_tooltip = 0;
            return;
        }
        if (event != kFocusLost && pid != PetalID::kNone && pid < PetalID::kNumPetals) {
            rendering_tooltip = 1;
            tooltip = Ui::UiLoadout::petal_tooltips[pid];
        } else {
            rendering_tooltip = 0;
        }
    }
};

class SpecialPetalCard final : public Element {
    PetalID::T petal_id;
public:
    SpecialPetalCard(PetalID::T id) : Element(64, 64, { .no_animation = 1 }), petal_id(id) {}
    void on_render(Renderer &ctx) override {
        ctx.scale(width / 60.f);
        draw_loadout_background(ctx, petal_id);
        if (petal_equipped(petal_id)) {
            ctx.scale(60.f / width);
            ctx.set_stroke(0xff3ecf8e);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.round_rect(-width / 2 - 2, -height / 2 - 2, width + 4, height + 4, 5);
            ctx.stroke();
        }
    }
    void on_event(uint8_t event) override {
        if (event == kMouseDown) {
            // Drag the first owned copy of this special petal
            uint32_t total = Game::loadout_count + MAX_SLOT_COUNT;
            for (uint32_t i = 0; i < total && i < 2 * MAX_SLOT_COUNT; ++i) {
                if (Game::cached_loadout[i] == petal_id) {
                    begin_petal_drag((uint8_t) i, touch_id);
                    rendering_tooltip = 0;
                    return;
                }
            }
        }
        if (event != kFocusLost && petal_id < PetalID::kNumPetals) {
            rendering_tooltip = 1;
            tooltip = Ui::UiLoadout::petal_tooltips[petal_id];
        } else {
            rendering_tooltip = 0;
        }
    }
};

// Drawer: slides up from above the bottom-left buttons
static Element *make_drawer(std::vector<Element *> children, uint8_t panel_id) {
    Element *elt = new Ui::VContainer(children, 14, 6, {
        .fill = 0xd0181818,
        .line_width = 4,
        .round_radius = 8,
        .animate = [](Element *elt, Renderer &ctx){
            // Slide up (closed: fully below)
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 12.f));
        },
        .should_render = [panel_id](){
            return Game::game_panel == panel_id
                && Game::alive()
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = 56; // right of vertical side buttons
    elt->y = -12;
    if (panel_id < 5) Ui::game_panel_roots[panel_id] = elt;
    return elt;
}

static constexpr float kBagPanelW = 420.f;
static constexpr float kBagScrollH = 360.f;

static Element *make_bag_scroll() {
    // Half-size cells → denser 5-col grid; wheel always scrolls while bag open
    std::vector<Element *> rows;
    constexpr uint8_t kCols = 5;
    uint8_t row_count = (MAX_SLOT_COUNT + kCols - 1) / kCols;
    for (uint8_t row = 0; row < row_count; ++row) {
        std::vector<Element *> cells;
        for (uint8_t col = 0; col < kCols; ++col) {
            uint8_t idx = (uint8_t) (row * kCols + col);
            if (idx < MAX_SLOT_COUNT)
                cells.push_back(new InventoryBagSlot(idx));
        }
        rows.push_back(new Ui::HContainer(cells, 0, 6, { .no_animation = 1 }));
    }
    Element *grid = new Ui::VContainer(rows, 0, 6, { .no_animation = 1 });
    auto *scroll = new Ui::ScrollContainer(grid, kBagScrollH);
    scroll->always_capture_wheel = 1;
    return scroll;
}

static void on_inventory_btn(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelInventory);
}
static void on_talents_btn(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelTalents);
}
static void on_craft_btn(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelCraft);
}
static void on_special_btn(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::toggle_game_panel(Game::kPanelSpecial);
}
static bool inventory_active() { return Game::game_panel == Game::kPanelInventory; }
static bool talents_active() { return Game::game_panel == Game::kPanelTalents; }
static bool craft_active() { return Game::game_panel == Game::kPanelCraft; }
static bool special_active() { return Game::game_panel == Game::kPanelSpecial; }

#define MAKE_SPECIAL_BUY(NAME, ID) \
static void on_buy_##NAME(Element *, uint8_t e) { \
    if (e == Ui::kClick) Game::craft_buy_special(PetalID::ID); \
}
MAKE_SPECIAL_BUY(antennae, kAntennae)
MAKE_SPECIAL_BUY(accelerator, kAccelerator)
MAKE_SPECIAL_BUY(magnet, kMagnet)

static void on_craft_once(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    if (Game::craft_tray_id == PetalID::kNone || Game::craft_tray_count < Craft::kCraftBatch)
        return;
    Game::craft_petals(CraftMode::kCraftGamble, Game::craft_tray_id);
}
static void on_craft_all(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    PetalID::T id = Game::craft_tray_id;
    if (id == PetalID::kNone) return;
    Game::craft_petals(CraftMode::kCraftGambleAll, id);
    Game::craft_tray_clear();
}
static void on_close_craft(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::close_game_panel();
}

static uint8_t shift_alt_held() {
    return Input::keys_held.contains('\x10') && Input::keys_held.contains((char) 18);
}

// Top pentagon tray
class CraftTrayView final : public Element {
public:
    CraftTrayView() : Element(220, 150, { .no_animation = 1 }) {}
    void on_render(Renderer &ctx) override {
        float cx = 0, cy = 8;
        float rad = 48;
        float slot = 36;
        for (uint32_t i = 0; i < Craft::kCraftBatch; ++i) {
            float ang = (float) (-M_PI / 2 + i * (2 * M_PI / Craft::kCraftBatch));
            float x = cx + std::cos(ang) * rad;
            float y = cy + std::sin(ang) * rad;
            ctx.set_fill(0xffc4b59a);
            ctx.begin_path();
            ctx.round_rect(x - slot / 2, y - slot / 2, slot, slot, 4);
            ctx.fill();
            if (Game::craft_tray_id != PetalID::kNone && Game::craft_tray_count > i) {
                RenderContext guard(&ctx);
                ctx.translate(x, y);
                ctx.scale(0.55f);
                draw_loadout_background(ctx, Game::craft_tray_id);
            }
        }
        if (Game::craft_tray_count > Craft::kCraftBatch) {
            std::string t = std::format("x{}", Game::craft_tray_count);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff222222);
            ctx.set_text_size(14);
            ctx.set_line_width(1.6f);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.stroke_text(t.c_str());
            ctx.fill_text(t.c_str());
        }
    }
};

// Material cell: click = +5, hold accelerates, Shift+Alt = dump all
class CraftMatCell final : public Element {
    PetalID::T petal_id;
    float hold_timer = 0;
    float hold_interval = 0.28f;
    uint8_t holding = 0;
public:
    CraftMatCell(PetalID::T id)
        : Element(52, 58, {
            // Only petals you actually own (bag stacks + material bank)
            .should_render = [id](){
                return Game::alive()
                    && id != PetalID::kNone
                    && id < PetalID::kNumPetals
                    && Game::craft_owned(id) > 0;
            },
            .no_animation = 1
        }), petal_id(id) {
        style.animate = [this](Element *, Renderer &){
            if (!holding) return;
            if (!(focus_state == kMouseDown)) {
                holding = 0;
                hold_timer = 0;
                hold_interval = 0.28f;
                return;
            }
            hold_timer += (float) (Ui::dt / 1000.0);
            while (hold_timer >= hold_interval) {
                hold_timer -= hold_interval;
                Game::craft_tray_add(petal_id, Craft::kCraftBatch);
                hold_interval = std::max(0.04f, hold_interval * 0.82f);
                if (Game::craft_available(petal_id) == 0) {
                    holding = 0;
                    break;
                }
            }
        };
    }
    void on_render(Renderer &ctx) override {
        if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) {
            ctx.set_fill(0xffb7a88c);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 3);
            ctx.fill();
            return;
        }
        // Remaining not yet in tray (= bag + bank − tray)
        uint32_t shown = Game::craft_available(petal_id);
        uint8_t selected = (Game::craft_tray_id == petal_id);
        {
            RenderContext guard(&ctx);
            ctx.scale(0.72f);
            draw_loadout_background(ctx, petal_id);
        }
        if (selected) {
            ctx.set_stroke(0xff00e5ff);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 3);
            ctx.stroke();
        }
        {
            RenderContext t(&ctx);
            std::string qty = std::format("x{}", shown);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(11);
            ctx.set_line_width(1.4f);
            ctx.translate(-width / 2 + 3, -height / 2 + 9);
            ctx.stroke_text(qty.c_str());
            ctx.fill_text(qty.c_str());
        }
        {
            RenderContext t(&ctx);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(9);
            ctx.set_line_width(1.2f);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.translate(0, height / 2 - 8);
            char const *nm = I18n::petal_name(petal_id);
            ctx.stroke_text(nm);
            ctx.fill_text(nm);
        }
    }
    void on_event(uint8_t event) override {
        if (petal_id == PetalID::kNone) return;
        if (event == kMouseDown) {
            if (shift_alt_held()) {
                Game::craft_tray_add(petal_id, Game::craft_available(petal_id));
                holding = 0;
            } else {
                Game::craft_tray_add(petal_id, Craft::kCraftBatch);
                holding = 1;
                hold_timer = 0;
                hold_interval = 0.28f;
            }
        } else if (event == kMouseUp || event == kFocusLost) {
            holding = 0;
            hold_timer = 0;
            hold_interval = 0.28f;
        }
        if (event != kFocusLost && petal_id < PetalID::kNumPetals) {
            rendering_tooltip = 1;
            tooltip = Ui::UiLoadout::petal_tooltips[petal_id];
        } else {
            rendering_tooltip = 0;
        }
    }
};

static Element *make_craft_material_grid() {
    // One cell per petal id; invisible when owned==0 so layout collapses to real inventory
    std::vector<PetalID::T> ids;
    for (PetalID::T id = 1; id < PetalID::kNumPetals; ++id)
        ids.push_back(id);
    std::stable_sort(ids.begin(), ids.end(), [](PetalID::T a, PetalID::T b){
        uint8_t ra = PETAL_DATA[a].rarity, rb = PETAL_DATA[b].rarity;
        if (ra != rb) return ra < rb;
        return std::strcmp(PETAL_DATA[a].name, PETAL_DATA[b].name) < 0;
    });
    constexpr uint8_t kCols = 5;
    std::vector<Element *> vrows;
    for (size_t i = 0; i < ids.size(); i += kCols) {
        std::vector<Element *> cells;
        std::vector<PetalID::T> row_ids;
        for (uint8_t c = 0; c < kCols && i + c < ids.size(); ++c) {
            row_ids.push_back(ids[i + c]);
            cells.push_back(new CraftMatCell(ids[i + c]));
        }
        vrows.push_back(new Ui::HContainer(cells, 0, 4, {
            .should_render = [row_ids](){
                for (PetalID::T id : row_ids)
                    if (Game::craft_owned(id) > 0) return true;
                return false;
            },
            .h_justify = Style::Left,
            .no_animation = 1
        }));
    }
    Element *grid = new Ui::VContainer(vrows, 0, 4, { .h_justify = Style::Left, .no_animation = 1 });
    return new Ui::ScrollContainer(grid, 210);
}

// IconPark geometry (viewBox 0..48), white glyphs for side buttons:
// backpack / star / merge / gift — https://iconpark.oceanengine.com/
static void iconpark_setup(Renderer &ctx, float button_w) {
    float s = button_w / 48.f;
    ctx.scale(s);
    ctx.translate(-24.f, -24.f);
    ctx.set_fill(0xffffffff);
    ctx.set_stroke(0xffffffff);
    ctx.set_line_width(3.5f);
    ctx.round_line_cap();
    ctx.round_line_join();
}

static Element *iconpark_side_icon(void (*draw)(Renderer &)) {
    return new Ui::StaticIcon([draw](Element *elt, Renderer &ctx) {
        iconpark_setup(ctx, elt->width);
        draw(ctx);
    }, 26, 26);
}

static void iconpark_backpack(Renderer &ctx) {
    // straps
    ctx.begin_path();
    ctx.move_to(19, 9.6f);
    ctx.line_to(19, 4);
    ctx.line_to(13, 4);
    ctx.line_to(13, 14);
    ctx.move_to(29, 9.6f);
    ctx.line_to(29, 4);
    ctx.line_to(35, 4);
    ctx.line_to(35, 14);
    ctx.stroke();
    // body
    ctx.begin_path();
    ctx.round_rect(11, 10, 26, 34, 4);
    ctx.fill();
    ctx.stroke();
    // side pockets
    ctx.begin_path();
    ctx.move_to(11, 29);
    ctx.line_to(5, 29);
    ctx.line_to(5, 39);
    ctx.line_to(11, 39);
    ctx.move_to(37, 29);
    ctx.line_to(43, 29);
    ctx.line_to(43, 39);
    ctx.line_to(37, 39);
    ctx.stroke();
    // zipper pocket (darker cut on white fill — use stroke inset)
    ctx.set_stroke(0xff5a9fdb);
    ctx.set_line_width(2.8f);
    ctx.begin_path();
    ctx.move_to(17, 23);
    ctx.line_to(31, 23);
    ctx.move_to(28, 23);
    ctx.line_to(28, 27);
    ctx.stroke();
}

static void iconpark_star(Renderer &ctx) {
    float const pts[][2] = {
        {24.f, 5.f}, {17.89f, 17.48f}, {4.f, 19.49f}, {14.06f, 29.33f},
        {11.65f, 43.f}, {24.f, 36.42f}, {36.35f, 43.f}, {33.96f, 29.33f},
        {44.f, 19.49f}, {30.19f, 17.48f}
    };
    ctx.begin_path();
    ctx.move_to(pts[0][0], pts[0][1]);
    for (int i = 1; i < 10; ++i)
        ctx.line_to(pts[i][0], pts[i][1]);
    ctx.close_path();
    ctx.fill();
}

static void iconpark_merge(Renderer &ctx) {
    // Two diamonds = rotated squares (IconPark merge)
    auto diamond = [&](float cx, float cy, float half) {
        ctx.begin_path();
        ctx.move_to(cx, cy - half);
        ctx.line_to(cx + half, cy);
        ctx.line_to(cx, cy + half);
        ctx.line_to(cx - half, cy);
        ctx.close_path();
        ctx.stroke();
    };
    diamond(18.f, 30.f, 11.f);
    diamond(30.f, 18.f, 11.f);
}

static void iconpark_gift(Renderer &ctx) {
    ctx.begin_path();
    ctx.round_rect(7, 20, 34, 24, 2);
    ctx.fill();
    ctx.begin_path();
    ctx.round_rect(4, 12, 40, 8, 1);
    ctx.fill();
    // Ribbon (IconPark colors[2]) — darker so it reads on white fill
    ctx.set_stroke(0xffb87418);
    ctx.set_line_width(3.2f);
    ctx.begin_path();
    ctx.move_to(24, 44);
    ctx.line_to(24, 12);
    ctx.stroke();
    ctx.set_stroke(0xffffffff);
    ctx.set_line_width(3.5f);
    ctx.begin_path();
    ctx.move_to(16, 4);
    ctx.line_to(24, 12);
    ctx.line_to(32, 4);
    ctx.stroke();
}

Element *Ui::make_game_side_buttons() {
    // Vertical column against left wall (bottom) — IconPark glyphs
    Element *elt = new Ui::VContainer({
        new Ui::Button(44, 44,
            iconpark_side_icon(iconpark_backpack),
            on_inventory_btn, inventory_active,
            { .fill = 0xff5a9fdb, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            iconpark_side_icon(iconpark_star),
            on_talents_btn, talents_active,
            { .fill = 0xff9b6bdb, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            iconpark_side_icon(iconpark_merge),
            on_craft_btn, craft_active,
            { .fill = 0xff1dd129, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            iconpark_side_icon(iconpark_gift),
            on_special_btn, special_active,
            { .fill = 0xffe0a030, .line_width = 3, .round_radius = 8 })
    }, 6, 6, {
        .should_render = [](){
            return Game::alive() && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = 4;
    elt->y = -8;
    Ui::game_side_buttons_root = elt;
    return elt;
}

static void on_close_inventory(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::close_game_panel();
}

Element *Ui::make_inventory_panel() {
    // Fixed-width panel; bag grid scrolls vertically inside
    Element *body = new Ui::VContainer({
        new Ui::Element(kBagPanelW, 1), // lock panel width
        new Ui::HFlexContainer(
            new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kPanelInventory); }, {
                .fill = 0xffffffff
            }),
            new Ui::Button(32, 32,
                new Ui::DynamicText(16, [](){ return std::string{"X"}; }),
                on_close_inventory, nullptr,
                { .fill = 0xffe03a3a, .line_width = 3, .round_radius = 6 }),
            0, 8, {}
        ),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? std::format("钱币 {}　点击装备 · 同种堆叠 · 装备拖出回背包", Game::coins)
                : std::format("Coins {}  click=equip · stacks · drag out unequips", Game::coins);
        }, { .fill = 0xffffd24a }),
        make_bag_scroll(),
        new Ui::DynamicText(11, [](){
            uint32_t bank = 0, bag = 0;
            for (PetalID::T id = 1; id < PetalID::kNumPetals; ++id) {
                bank += Game::craft_bank[id];
                bag += Game::craft_bag_count(id);
            }
            return I18n::use_chinese
                ? std::format("材料库 {} · 可合成背包 {}（C）", bank, bag)
                : std::format("bank {} · craftable bag {} (C)", bank, bag);
        }, { .fill = 0xffdddddd })
    }, 14, 8, {
        .fill = 0xe0286a9e,
        .line_width = 5,
        .round_radius = 10,
        .animate = [](Element *elt, Renderer &ctx){
            // Keep fixed outer size so scroll area does not resize the chrome
            elt->width = kBagPanelW + 28.f;
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 20.f));
        },
        .should_render = [](){
            return Game::game_panel == Game::kPanelInventory
                && Game::alive()
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    body->x = 56;
    body->y = -12;
    Ui::game_panel_roots[Game::kPanelInventory] = body;
    return body;
}

static uint32_t talent_player_level() {
    if (Game::alive()) {
        uint32_t lv = score_to_level(Game::score);
        return lv < 1 ? 1 : lv;
    }
    return Game::respawn_level < 1 ? 1 : Game::respawn_level;
}

static void on_talent_hp(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::talent_spend(TalentID::kHealth);
}
static void on_talent_rot(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::talent_spend(TalentID::kRotSpeed);
}

Element *Ui::make_talents_panel() {
    return make_drawer({
        new Ui::DynamicText(20, [](){ return I18n::tr(I18n::kPanelTalents); }, {
            .fill = 0xffffffff, .h_justify = Style::Left
        }),
        new Ui::DynamicText(13, [](){
            uint32_t lv = talent_player_level();
            uint32_t free = Talent::points_free(lv, Game::talent_hp, Game::talent_rot);
            uint8_t slots = Talent::max_rank(lv);
            return I18n::use_chinese
                ? std::format("可用点数 {}　槽位上限 {}（每 10 级 +1）", free, slots)
                : std::format("{} pts · max rank {} (unlock +1 / 10 lv)", free, slots);
        }, { .fill = 0xfff0d878, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){
            return I18n::use_chinese
                ? "每级 +1 点，每 10 级额外 +10 点。点击加点。"
                : "+1 pt/level, +10 pts every 10 levels. Click to invest.";
        }, { .fill = 0xffbbbbbb, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(150, 56,
                new Ui::VContainer({
                    new Ui::DynamicText(14, [](){
                        return I18n::use_chinese ? "体魄" : "Vitality";
                    }, { .fill = 0xffffffff }),
                    new Ui::DynamicText(12, [](){
                        return std::format("Lv{}  +{:.0f}HP",
                            (int) Game::talent_hp,
                            Talent::health_bonus(Game::talent_hp));
                    }, { .fill = 0xffa8e6a8 })
                }, 0, 2, {}),
                on_talent_hp, nullptr,
                { .fill = 0xff3a6b4a, .line_width = 3, .round_radius = 6 }),
            new Ui::Button(150, 56,
                new Ui::VContainer({
                    new Ui::DynamicText(14, [](){
                        return I18n::use_chinese ? "转速" : "Spin";
                    }, { .fill = 0xffffffff }),
                    new Ui::DynamicText(12, [](){
                        return std::format("Lv{}  +{:.1f}rad/s",
                            (int) Game::talent_rot,
                            Talent::rot_bonus(Game::talent_rot));
                    }, { .fill = 0xffa8d4e6 })
                }, 0, 2, {}),
                on_talent_rot, nullptr,
                { .fill = 0xff3a5a7a, .line_width = 3, .round_radius = 6 })
        }, 0, 10, {}),
        new Ui::DynamicText(11, [](){
            uint32_t lv = talent_player_level();
            uint8_t nh = (uint8_t) (Game::talent_hp + 1);
            uint8_t nr = (uint8_t) (Game::talent_rot + 1);
            uint8_t max_r = Talent::max_rank(lv);
            std::string a = Game::talent_hp >= max_r
                ? (I18n::use_chinese ? "体魄已满" : "Vitality max")
                : (I18n::use_chinese
                    ? std::format("体魄下级需 {} 点", Talent::rank_cost(nh))
                    : std::format("Vitality next: {} pts", Talent::rank_cost(nh)));
            std::string b = Game::talent_rot >= max_r
                ? (I18n::use_chinese ? "转速已满" : "Spin max")
                : (I18n::use_chinese
                    ? std::format("转速下级需 {} 点", Talent::rank_cost(nr))
                    : std::format("Spin next: {} pts", Talent::rank_cost(nr)));
            return a + "　·　" + b;
        }, { .fill = 0xffcccccc, .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "局内用 Q / E 调转速圆轮（槽右侧）"
                : "In-game: Q/E adjust the spin wheel (right of slots)";
        }, { .fill = 0xff999999, .h_justify = Style::Left })
    }, Game::kPanelTalents);
}

Element *Ui::make_craft_panel() {
    Element *elt = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kPanelCraft); }, {
                .fill = 0xffffffff
            }),
            new Ui::HContainer({
                new Ui::Button(88, 28,
                    new Ui::DynamicText(12, [](){
                        return I18n::use_chinese ? "全部合成" : "Craft All";
                    }),
                    on_craft_all, nullptr,
                    { .fill = 0xff555555, .line_width = 3, .round_radius = 4 }),
                new Ui::Button(32, 28,
                    new Ui::DynamicText(14, [](){ return std::string{"X"}; }),
                    on_close_craft, nullptr,
                    { .fill = 0xffe03a3a, .line_width = 3, .round_radius = 4 })
            }, 0, 6, {}),
            0, 8, {}
        ),
        new Ui::HContainer({
            new CraftTrayView(),
            new Ui::VContainer({
                new Ui::Button(96, 36,
                    new Ui::DynamicText(16, [](){
                        return I18n::use_chinese ? "合成" : "Craft";
                    }),
                    on_craft_once, nullptr,
                    { .fill = 0xff444444, .line_width = 3, .round_radius = 4 }),
                new Ui::DynamicText(11, [](){
                    if (Game::craft_tray_id == PetalID::kNone)
                        return I18n::use_chinese
                            ? std::string{"点击下方花瓣放入合成区（每次+5）"}
                            : std::string{"Click petals below (+5 each)"};
                    uint8_t r = PETAL_DATA[Game::craft_tray_id].rarity;
                    float p = Craft::success_chance(r) * 100.f;
                    return I18n::use_chinese
                        ? std::format("五合一→{} {:.0f}% · Shift+Alt放满",
                            I18n::rarity_name((uint8_t) (r + 1)), p)
                        : std::format("5→{} {:.0f}% · Shift+Alt=all",
                            I18n::rarity_name((uint8_t) (r + 1)), p);
                }, { .fill = 0xff222222, .h_justify = Style::Left })
            }, 8, 8, { .h_justify = Style::Left })
        }, 0, 12, { .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "仅显示背包/材料库已有花瓣 · 五个相同合成更高一级 · Shift+Alt 放满"
                : "Only owned petals (bag+bank) · 5→next rarity · Shift+Alt=all";
        }, { .fill = 0xff333333, .h_justify = Style::Left }),
        make_craft_material_grid(),
        new Ui::DynamicText(12, [](){ return Game::craft_status; }, {
            .fill = 0xff553300, .h_justify = Style::Left
        })
    }, 14, 8, {
        .fill = 0xf0d9c9a8,
        .line_width = 5,
        .round_radius = 10,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 12.f));
        },
        .should_render = [](){
            return Game::game_panel == Game::kPanelCraft
                && Game::alive()
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = 56;
    elt->y = -12;
    Ui::game_panel_roots[Game::kPanelCraft] = elt;
    return elt;
}

Element *Ui::make_special_craft_panel() {
    return make_drawer({
        new Ui::DynamicText(20, [](){ return I18n::tr(I18n::kPanelSpecial); }, {
            .fill = 0xffffffff, .h_justify = Style::Left
        }),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese
                ? std::format("钱币：{}", Game::coins)
                : std::format("Coins: {}", Game::coins);
        }, { .fill = 0xffffd24a, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){
            return I18n::use_chinese
                ? "特殊花瓣商店（买完进背包；绿框=已装备主手）"
                : "Special shop (buys → bag; green = equipped)";
        }, { .fill = 0xffcccccc, .h_justify = Style::Left }),
        new Ui::HContainer({
            new SpecialPetalCard(PetalID::kAntennae),
            new SpecialPetalCard(PetalID::kAccelerator),
            new SpecialPetalCard(PetalID::kMagnet)
        }, 0, 10, { .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(110, 28,
                new Ui::DynamicText(11, [](){
                    return I18n::use_chinese
                        ? std::format("触角·{}币", Craft::buy_special_cost(PetalID::kAntennae))
                        : std::format("Ant·{}", Craft::buy_special_cost(PetalID::kAntennae));
                }),
                on_buy_antennae, nullptr,
                { .fill = 0xff9b6bdb, .line_width = 3, .round_radius = 4 }),
            new Ui::Button(110, 28,
                new Ui::DynamicText(11, [](){
                    return I18n::use_chinese
                        ? std::format("加速·{}币", Craft::buy_special_cost(PetalID::kAccelerator))
                        : std::format("Accel·{}", Craft::buy_special_cost(PetalID::kAccelerator));
                }),
                on_buy_accelerator, nullptr,
                { .fill = 0xff5a9fdb, .line_width = 3, .round_radius = 4 }),
            new Ui::Button(110, 28,
                new Ui::DynamicText(11, [](){
                    return I18n::use_chinese
                        ? std::format("磁铁·{}币", Craft::buy_special_cost(PetalID::kMagnet))
                        : std::format("Mag·{}", Craft::buy_special_cost(PetalID::kMagnet));
                }),
                on_buy_magnet, nullptr,
                { .fill = 0xffe0a030, .line_width = 3, .round_radius = 4 })
        }, 0, 6, { .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){
            return I18n::use_chinese
                ? std::format("装备中：触角 {}　加速 {}　磁铁 {}",
                    petal_equipped(PetalID::kAntennae) ? "开" : "关",
                    petal_equipped(PetalID::kAccelerator) ? "开" : "关",
                    petal_equipped(PetalID::kMagnet) ? "开" : "关")
                : std::format("Equipped: Ant {} · Accel {} · Mag {}",
                    petal_equipped(PetalID::kAntennae) ? "ON" : "off",
                    petal_equipped(PetalID::kAccelerator) ? "ON" : "off",
                    petal_equipped(PetalID::kMagnet) ? "ON" : "off");
        }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "从背包拖到主手栏生效。也可在 C 制作里用钱币买高等级花瓣。"
                : "Drag bag → primary to equip. Or buy high rarities in C craft.";
        }, { .fill = 0xffbbbbbb, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){ return Game::craft_status; }, {
            .fill = 0xffffe0a0, .h_justify = Style::Left
        })
    }, Game::kPanelSpecial);
}

#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/StaticIcon.hh>
#include <Client/Ui/StaticText.hh>
#include <Client/Ui/TextInput.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Render/Renderer.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/StaticData.hh>

#include <Helpers/Bits.hh>

#include <Shared/Binary.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <array>
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

// Layout: primary [0,n) · inventory [n,n+MAX), whose first n entries
// are also shown in the secondary quick-swap row.

static std::string bag_search_text;
static Element *bag_cells[PetalID::kNumPetals] = {};

static uint8_t bag_ui_open() {
    return Game::game_panel == Game::kPanelInventory
        && Game::should_render_game_ui()
        && (Game::alive() || Game::on_game_screen);
}

// The backpack presents slot petals and stacked petals as one collection.
static uint32_t bag_stack_count(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    uint32_t n = Game::craft_bank[id];
    for (uint8_t i = 0; i < MAX_SLOT_COUNT; ++i)
        if (Game::cached_loadout[Game::loadout_count + i] == id) ++n;
    return n;
}

static uint8_t bag_slot_static(PetalID::T id) {
    for (uint8_t i = 0; i < MAX_SLOT_COUNT; ++i)
        if (Game::cached_loadout[Game::loadout_count + i] == id)
            return (uint8_t)(Game::loadout_count + i);
    return (uint8_t)-1;
}

static uint8_t bag_name_matches(PetalID::T id) {
    if (bag_search_text.empty()) return 1;
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    char const *nm = I18n::petal_name(id);
    if (nm == nullptr) nm = PETAL_DATA[id].name;
    // Case-insensitive substring
    std::string hay = nm;
    std::string needle = bag_search_text;
    for (char &c : hay) if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
    for (char &c : needle) if (c >= 'A' && c <= 'Z') c = (char) (c - 'A' + 'a');
    return hay.find(needle) != std::string::npos;
}

// Backpack order: rarity blocks high→low (Unique → … → Common). Within a
// rarity, sort by name. Same flower at different rarities must NOT sit
// together — they belong in separate rarity blocks.
static std::vector<PetalID::T> const &bag_sorted_ids() {
    static std::vector<PetalID::T> const order = []{
        std::vector<PetalID::T> ids;
        ids.reserve(PetalID::kNumPetals);
        for (uint32_t id = 1; id < PetalID::kNumPetals; ++id)
            ids.push_back((PetalID::T) id);
        std::stable_sort(ids.begin(), ids.end(), [](PetalID::T a, PetalID::T b){
            uint8_t ra = PETAL_DATA[a].rarity, rb = PETAL_DATA[b].rarity;
            if (ra != rb) return ra > rb; // high tier → low tier
            char const *na = PETAL_DATA[a].name;
            char const *nb = PETAL_DATA[b].name;
            if (na != nullptr && nb != nullptr) {
                int c = std::strcmp(na, nb);
                if (c != 0) return c < 0;
            }
            return a < b;
        });
        return ids;
    }();
    return order;
}

static PetalID::T bag_display_id(uint16_t idx) {
    if (!Game::loadout_count) return PetalID::kNone;
    for (PetalID::T id : bag_sorted_ids()) {
        if (!bag_stack_count(id) || !bag_name_matches(id)) continue;
        if (idx-- == 0) return id;
    }
    return PetalID::kNone;
}

static uint8_t bag_slot_visible(uint16_t idx) {
    return bag_ui_open() && bag_display_id(idx) != PetalID::kNone;
}

// Bag cell: press shows tooltip; move past slop → drag; tiny release → equip.
class InventoryBagSlot final : public Element {
    uint16_t bag_index; // index in the filtered stack grid
    uint8_t holding;   // finger/mouse down on this cell
    uint8_t dragging;  // past slop — ghost owned by UiLoadoutPetal
    uint32_t drag_touch;
    float press_x, press_y;
public:
    InventoryBagSlot(uint16_t idx)
        : Element(90, 90, { .no_animation = 1 }),
          bag_index(idx), holding(0), dragging(0), drag_touch((uint32_t)-1),
          press_x(0), press_y(0) {
        style.should_render = [idx](){ return bag_slot_visible(idx); };
        bag_cells[idx] = this;
    }
    void refactor() override {
        visible = style.should_render();
        // Larger cells on mobile so stack/expand reflow stays tappable
        width = Input::is_mobile ? 78.f : 90.f;
        height = Input::is_mobile ? 84.f : 90.f;
    }
    void poll_events(ScreenEvent const &event) override {
        if (!Input::is_mobile) {
            Element::poll_events(event);
            return;
        }
        float const ow = width, oh = height;
        width = ow + 12.f;
        height = oh + 12.f;
        Element::poll_events(event);
        width = ow;
        height = oh;
    }
    uint8_t slot_static() const {
        return bag_slot_static(id());
    }
    PetalID::T id() const {
        return bag_display_id(bag_index);
    }
    void on_render(Renderer &ctx) override {
        if (Input::is_mobile && holding) {
            bool active = Input::touches.contains(drag_touch);
            dragging = active && Ui::UiLoadout::num_petals_selected != 0;
            rendering_tooltip = active && !dragging;
            if (!active) {
                holding = 0;
                drag_touch = (uint32_t)-1;
            }
        }
        PetalID::T pid = id();
        uint32_t fill = Chrome::kPanelInset;
        uint8_t rarity = 0;
        if (pid != PetalID::kNone && pid < PetalID::kNumPetals) {
            rarity = PETAL_DATA[pid].rarity;
            if (rarity < RarityID::kNumRarities) fill = RARITY_COLORS[rarity];
        }
        // Flat rarity face (no muddy HSV rim) so the tier color reads clearly.
        {
            float hw = width / 2.f, hh = height / 2.f;
            float rad = Chrome::kRadius;
            ctx.set_fill(0x38000000);
            ctx.begin_path();
            ctx.round_rect(-hw + 2.f, -hh + 3.f, width, height, rad);
            ctx.fill();
            ctx.set_fill(fill);
            ctx.begin_path();
            ctx.round_rect(-hw, -hh, width, height, rad);
            ctx.fill();
            ctx.set_stroke(0x66000000);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.round_rect(-hw + 1.f, -hh + 1.f, width - 2.f, height - 2.f, rad - 1.f);
            ctx.stroke();
        }
        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) return;
        if (dragging) ctx.set_global_alpha(0.4f);
        {
            RenderContext c(&ctx);
            ctx.translate(0, -9);
            ctx.scale(1.15f);
            if (PETAL_DATA[pid].radius > 20)
                ctx.scale(20.f / PETAL_DATA[pid].radius);
            draw_static_petal(pid, ctx);
        }
        // Rarity tag at the top so the tier is readable without hovering.
        {
            RenderContext c(&ctx);
            ctx.translate(0, -height / 2 + 11);
            ctx.set_text_size(11);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xaa222222);
            ctx.set_line_width(1.4f);
            ctx.stroke_text(I18n::rarity_name(rarity));
            ctx.fill_text(I18n::rarity_name(rarity));
        }
        uint32_t stack = bag_stack_count(pid);
        if (stack > 1) {
            std::string badge = std::format("x{}", stack);
            float bx = width / 2 - 12;
            float by = -height / 2 + 10;
            ctx.set_fill(0xe0000000);
            ctx.begin_path();
            ctx.round_rect(bx - 12, by - 9, 24, 18, 8);
            ctx.fill();
            {
                RenderContext c(&ctx);
                ctx.translate(bx, by);
                ctx.center_text_align();
                ctx.center_text_baseline();
                ctx.draw_text(badge.c_str(), {
                    .fill = Chrome::kGold, .size = 13, .stroke_scale = 0.1f
                });
            }
        }
        ctx.translate(0, height / 2 - 15);
        ctx.draw_text(I18n::petal_name(pid), {
            .fill = Chrome::kTitle, .size = 16, .stroke_scale = 0.1f
        });
        ctx.translate(0, -(height / 2 - 15));
    }
    void on_event(uint8_t event) override {
        PetalID::T pid = id();
        if (event == kMouseDown && pid != PetalID::kNone && pid < PetalID::kNumPetals) {
            // Mobile repeats kMouseDown every held frame — only arm once.
            if (!holding) {
                holding = 1;
                dragging = 0;
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
                if (slot_static() != (uint8_t)-1)
                    begin_bag_gesture(slot_static(), press_x, press_y, drag_touch);
            }
            rendering_tooltip = 1;
            tooltip = Ui::UiLoadout::petal_tooltips[pid];
            return;
        }
        if (event == kClick) {
            // Slot copies equip via bag gesture; stack-only copies equip
            // straight from the backpack (same collection the panel shows).
            if (slot_static() == (uint8_t)-1 && pid != PetalID::kNone
                && !Recipe::is_bag_only(pid))
                Game::withdraw_bank_petal(pid);
            holding = 0;
            dragging = 0;
            drag_touch = (uint32_t)-1;
            rendering_tooltip = 0;
            return;
        }
        if (event == kFocusLost) {
            // Moving outside the cell does not end its gesture or tooltip.
            if (holding && Input::is_mobile && Input::touches.contains(drag_touch)) {
                if (Ui::UiLoadout::num_petals_selected != 0)
                    rendering_tooltip = 0;
                return;
            }
            holding = 0;
            dragging = 0;
            drag_touch = (uint32_t)-1;
            rendering_tooltip = 0;
            return;
        }
        if (pid != PetalID::kNone && pid < PetalID::kNumPetals) {
            if (!holding) {
                rendering_tooltip = 1;
                tooltip = Ui::UiLoadout::petal_tooltips[pid];
            }
        } else {
            rendering_tooltip = 0;
        }
    }
};

uint8_t Ui::bag_drop_target(float x, float y) {
    if (!bag_ui_open()) return (uint8_t)-1;
    for (uint16_t i = 0; i < PetalID::kNumPetals; ++i) {
        Element *cell = bag_cells[i];
        if (cell == nullptr || !cell->visible || !bag_slot_visible(i)) continue;
        float half_w = cell->width * Ui::scale / 2;
        float half_h = cell->height * Ui::scale / 2;
        if (fabsf(x - cell->screen_x) <= half_w
            && fabsf(y - cell->screen_y) <= half_h)
            return bag_slot_static(bag_display_id(i));
    }
    return (uint8_t)-1;
}

class SpecialPetalCard final : public Element {
    PetalID::T petal_id;
public:
    SpecialPetalCard(PetalID::T id) : Element(64, 64, { .no_animation = 1 }), petal_id(id) {}
    void on_render(Renderer &ctx) override {
        ctx.scale(width / 60.f);
        draw_loadout_background(ctx, petal_id);
        if (petal_equipped(petal_id)) {
            ctx.scale(60.f / width);
            ctx.set_stroke(0xaa3ecf8e);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.round_rect(-width / 2 - 2, -height / 2 - 2, width + 4, height + 4, 10);
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
    Element *elt = new Ui::VContainer(children, 18, 12, {
        .fill = Chrome::kPanelDeep,
        .line_width = 0,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 16.f));
        },
        .should_render = [panel_id](){
            return Game::game_panel == panel_id
                && (Game::alive()
                    || ((panel_id == Game::kPanelInventory
                        || panel_id == Game::kPanelTalents
                        || panel_id == Game::kPanelPresets
                        || panel_id == Game::kPanelCraft
                        || panel_id == Game::kPanelSpecial)
                        && Game::on_game_screen))
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    // Sit clear of side plaques (humanized: not jammed into loadout)
    elt->x = Input::is_mobile ? 92.f : 72.f;
    elt->y = -20;
    if (panel_id < 7) Ui::game_panel_roots[panel_id] = elt;
    return elt;
}

static constexpr float kBagPanelW = 540.f;
static constexpr float kBagScrollH = 620.f;
static constexpr float kBagScrollHMobile = 390.f;

class BagGridRow final : public Ui::HContainer {
    uint16_t first;
public:
    BagGridRow(std::vector<Element *> const &cells, uint16_t idx)
        : Ui::HContainer(cells, 0, 6, { .no_animation = 1 }), first(idx) {
        style.should_render = [idx](){ return bag_display_id(idx) != PetalID::kNone; };
    }
    void refactor() override {
        visible = style.should_render();
        Ui::HContainer::refactor();
    }
};

// Fixed viewport: panel height must not grow/shrink with petal count.
// Overflow scrolls inside this clip; underfill leaves empty space below.
class BagScroll final : public Ui::ScrollContainer {
    float viewport_h;
public:
    BagScroll(Element *content, float h)
        : Ui::ScrollContainer(content, h), viewport_h(h) {}
    void refactor() override {
        Ui::ScrollContainer::refactor();
        height = viewport_h;
    }
};

static Element *make_bag_scroll() {
    // Mobile: 4 cols + taller cells; desktop keeps denser 5-col grid
    std::vector<Element *> rows;
    uint8_t const kCols = Input::is_mobile ? 4 : 5;
    float const scroll_h = Input::is_mobile ? kBagScrollHMobile : kBagScrollH;
    uint16_t row_count = (PetalID::kNumPetals - 1 + kCols - 1) / kCols;
    for (uint16_t row = 0; row < row_count; ++row) {
        std::vector<Element *> cells;
        for (uint8_t col = 0; col < kCols; ++col) {
            uint16_t idx = (uint16_t) (row * kCols + col);
            if (idx < PetalID::kNumPetals - 1)
                cells.push_back(new InventoryBagSlot(idx));
        }
        rows.push_back(new BagGridRow(cells, (uint16_t)(row * kCols)));
    }
    Element *grid = new Ui::VContainer(rows, 0, 6, { .no_animation = 1 });
    auto *scroll = new BagScroll(grid, scroll_h);
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
static uint8_t preset_save_mode = 0;
static bool inventory_active() { return Game::game_panel == Game::kPanelInventory; }
static bool talents_active() { return Game::game_panel == Game::kPanelTalents; }
static bool craft_active() { return Game::game_panel == Game::kPanelCraft; }
static bool special_active() { return Game::game_panel == Game::kPanelSpecial; }

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

// Large pentagon work area, matching the warm crafting board.
class CraftTrayView final : public Element {
public:
    CraftTrayView() : Element(350, 280, { .no_animation = 1 }) {}
    void on_render(Renderer &ctx) override {
        float cx = 0, cy = 8;
        float rad = 88;
        float slot = 60;
        for (uint32_t i = 0; i < Craft::kCraftBatch; ++i) {
            float ang = (float) (-M_PI / 2 + i * (2 * M_PI / Craft::kCraftBatch));
            float x = cx + std::cos(ang) * rad;
            float y = cy + std::sin(ang) * rad;
            ctx.set_fill(Chrome::kPanelInset);
            ctx.begin_path();
            ctx.round_rect(x - slot / 2, y - slot / 2, slot, slot, 7);
            ctx.fill();
            if (Game::craft_tray_id != PetalID::kNone && Game::craft_tray_count > i) {
                RenderContext guard(&ctx);
                ctx.translate(x, y);
                ctx.scale(0.92f);
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
        : Element(64, 66, {
            // Always occupy the column so rarity columns line up across rows.
            // Death screen keeps craft UI open (same as bag / specials).
            .should_render = [](){
                return Game::alive() || Game::on_game_screen;
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
        uint32_t owned = (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals)
            ? 0 : Game::craft_owned(petal_id);
        if (owned == 0) {
            ctx.set_fill(Chrome::kPanelInset);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 6);
            ctx.fill();
            return;
        }
        // Remaining not yet in tray (= bag + bank − tray)
        uint32_t shown = Game::craft_available(petal_id);
        uint8_t selected = (Game::craft_tray_id == petal_id);
        {
            RenderContext guard(&ctx);
            ctx.scale(0.9f);
            draw_loadout_background(ctx, petal_id);
        }
        if (selected) {
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 10);
            ctx.stroke();
        }
        {
            RenderContext t(&ctx);
            std::string qty = std::format("x{}", shown);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(14);
            ctx.set_line_width(1.4f);
            ctx.translate(-width / 2 + 3, -height / 2 + 12);
            ctx.stroke_text(qty.c_str());
            ctx.fill_text(qty.c_str());
        }
        {
            RenderContext t(&ctx);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(12);
            ctx.set_line_width(1.2f);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.translate(0, height / 2 - 10);
            char const *nm = I18n::petal_name(petal_id);
            ctx.stroke_text(nm);
            ctx.fill_text(nm);
        }
    }
    void on_event(uint8_t event) override {
        if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
        if (Game::craft_owned(petal_id) == 0) return;
        if (event == kMouseDown) {
            if (Game::craft_available(petal_id) == 0) return;
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

class CraftMaterialGrid final : public Ui::ScrollContainer {
public:
    explicit CraftMaterialGrid(Element *content) : Ui::ScrollContainer(content, 316) {}
    void refactor() override {
        Ui::ScrollContainer::refactor();
        // Keep the full board visible on short desktop windows and phones.
        float visible_height = (float)(Ui::window_height / Ui::scale);
        height = std::clamp(visible_height - 430.f, 140.f, 290.f);
    }
};

static Element *make_craft_material_grid() {
    // One row per petal name. Columns are rarities left → right (Common…Unique).
    // Missing rarities stay as empty cells so every row is the same length.
    struct Family {
        char const *name;
        std::array<PetalID::T, RarityID::kNumRarities> ids;
    };
    std::vector<Family> families;
    for (uint32_t raw = 1; raw < (uint32_t) PetalID::kNumPetals; ++raw) {
        PetalID::T id = (PetalID::T) raw;
        if (Recipe::is_bag_only(id)) continue;
        char const *name = PETAL_DATA[id].name;
        uint8_t r = PETAL_DATA[id].rarity;
        if (name == nullptr || r >= RarityID::kNumRarities) continue;
        Family *fam = nullptr;
        for (Family &f : families) {
            if (std::strcmp(f.name, name) == 0) { fam = &f; break; }
        }
        if (fam == nullptr) {
            families.push_back(Family{ name, {} });
            fam = &families.back();
            fam->ids.fill(PetalID::kNone);
        }
        fam->ids[r] = id;
    }
    std::stable_sort(families.begin(), families.end(), [](Family const &a, Family const &b){
        return std::strcmp(a.name, b.name) < 0;
    });
    std::vector<Element *> vrows;
    for (Family const &fam : families) {
        std::array<PetalID::T, RarityID::kNumRarities> ids = fam.ids;
        std::vector<Element *> cells;
        for (uint8_t r = 0; r < RarityID::kNumRarities; ++r)
            cells.push_back(new CraftMatCell(ids[r]));
        cells.push_back(new CraftMatCell(PetalID::kNone));
        vrows.push_back(new Ui::HContainer(cells, 0, 4, {
            .should_render = [ids](){
                for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
                    PetalID::T id = ids[r];
                    if (id != PetalID::kNone && Game::craft_owned(id) > 0) return true;
                }
                return false;
            },
            .h_justify = Style::Left,
            .no_animation = 1
        }));
    }
    Element *grid = new Ui::VContainer(vrows, 0, 4, { .h_justify = Style::Left, .no_animation = 1 });
    return new CraftMaterialGrid(grid);
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
    // Vertical column against left wall (bottom) — IconPark glyphs.
    // Size bumped further on mobile by Button::apply_mobile_touch_size (44→64).
    float const gap = Input::is_mobile ? 10.f : 6.f;
    // Stone plaques — muted earth tones, not candy bubbles
    Element *elt = new Ui::VContainer({
        new Ui::Button(48, 48,
            iconpark_side_icon(iconpark_backpack),
            on_inventory_btn, inventory_active,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            iconpark_side_icon(iconpark_star),
            on_talents_btn, talents_active,
            { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            iconpark_side_icon(iconpark_merge),
            on_craft_btn, craft_active,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            iconpark_side_icon(iconpark_gift),
            on_special_btn, special_active,
            { .fill = Chrome::kBtnClay, .line_width = 0, .round_radius = Chrome::kRadius })
    }, gap, gap, {
        .should_render = [](){
            return Game::should_render_game_ui()
                && (Game::alive() || Game::on_game_screen);
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = Input::is_mobile ? 10.f : 8.f;
    elt->y = -24; // lift clear of loadout bar
    Ui::game_side_buttons_root = elt;
    return elt;
}

static void on_close_inventory(Element *, uint8_t e) {
    if (e == Ui::kClick) {
        preset_save_mode = 0;
        Game::close_game_panel();
    }
}
static void on_compact_bag(Element *, uint8_t e) {
    if (e == Ui::kClick) ui_compact_backpack();
}

Element *Ui::make_inventory_panel() {
    Element *body = new Ui::VContainer({
        new Ui::Element(kBagPanelW, 1),
        new Ui::HFlexContainer(
            new Ui::DynamicText(29, [](){ return I18n::use_chinese ? "背包" : "Backpack"; }, {
                .fill = Chrome::kTitle
            }),
            new Ui::Button(36, 36,
                new Ui::DynamicText(18, [](){ return std::string{"X"}; }),
                on_close_inventory, nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 10, {}
        ),
        new Ui::TextInput(bag_search_text, Input::is_mobile ? 330.f : 480.f, 42, 48, {
            .fill = 0xfff5f5f5, .line_width = 3, .round_radius = 3,
            .h_justify = Style::Left, .no_animation = 1
        }),
        make_bag_scroll(),
    }, 18, 10, {
        .fill = 0xff5e9fd2,
        .line_width = Chrome::kRim,
        .round_radius = Chrome::kRadius,
        // Mobile: snap open (slide lerp felt like “scale lag” on bag expand)
        .animate = [](Element *elt, Renderer &ctx){
            elt->width = (Input::is_mobile ? 360.f : kBagPanelW) + 32.f;
            if (!Input::is_mobile)
                ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 24.f));
            float mx = Input::mouse_x, my = Input::mouse_y;
            if (Input::is_mobile && Ui::UiLoadout::num_petals_selected != 0) {
                for (auto const &[id, touch] : Input::touches) {
                    mx = touch.x;
                    my = touch.y;
                    break;
                }
            }
            if (Ui::UiLoadout::num_petals_selected != 0
                && loadout_drag_over_bag(mx, my)) {
                ctx.set_global_alpha(0.4f + 0.12f * (float) sin(Game::timestamp / 140.0));
                ctx.set_stroke(0x88ffffff);
                ctx.set_line_width(3);
                ctx.begin_path();
                ctx.round_rect(-elt->width / 2, -elt->height / 2, elt->width, elt->height, Chrome::kRadius);
                ctx.stroke();
                ctx.set_global_alpha(1.f);
            }
        },
        .should_render = [](){ return bag_ui_open(); },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom,
        .no_animation = Input::is_mobile ? (uint8_t) 1 : (uint8_t) 0
    });
    body->x = Input::is_mobile ? 92.f : 72.f;
    body->y = -20;
    Ui::game_panel_roots[Game::kPanelInventory] = body;
    return body;
}

class PresetButton final : public Ui::Button {
    uint8_t index;
    double press_at;
    uint8_t long_fired;
    static constexpr double kLongMs = 420.0;
public:
    explicit PresetButton(uint8_t i) : Ui::Button(Input::is_mobile ? 58.f : 52.f,
        Input::is_mobile ? 54.f : 48.f,
        new Ui::DynamicText(14, [i](){
            uint8_t number = (uint8_t) (i % 10 + 1);
            char const *mark = "";
            if (Game::active_loadout_preset == i) mark = ">";
            else if (Game::loadout_presets[i].slot_count) mark = "*";
            return std::format("{}{}{}", i < 10 ? "K" : "L",
                number == 10 ? "0" : std::to_string(number), mark);
        }),
        [](Element *, uint8_t){}, nullptr,
        { .fill = i < 10 ? Chrome::kBtnSlate : Chrome::kBtnMoss,
          .line_width = 0, .round_radius = Chrome::kRadius }),
        index(i), press_at(0), long_fired(0) {}

    void on_render(Renderer &ctx) override {
        // Hold progress ring while saving
        if (focus_state == Ui::kMouseDown && !long_fired
            && Game::timestamp - press_at >= kLongMs) {
            long_fired = 1;
            Game::save_loadout_preset(index);
            preset_save_mode = 0;
        }
        Button::on_render(ctx);
        if (Game::active_loadout_preset == index) {
            ctx.set_stroke(Chrome::kGold);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, Chrome::kRadius);
            ctx.stroke();
        }
    }

    void on_event(uint8_t event) override {
        if (event == Ui::kMouseDown) {
            press_at = Game::timestamp;
            long_fired = 0;
            return;
        }
        if (event != Ui::kClick) return;
        if (long_fired) {
            long_fired = 0;
            return;
        }
        if (preset_save_mode) {
            Game::save_loadout_preset(index);
            preset_save_mode = 0;
        } else {
            // Missing petals → lower same-name rarity or empty; switch anyway.
            Game::apply_loadout_preset(index);
        }
    }
};

Element *Ui::make_presets_panel() {
    std::vector<Element *> rows;
    for (uint8_t row = 0; row < 4; ++row) {
        std::vector<Element *> buttons;
        for (uint8_t col = 0; col < 5; ++col)
            buttons.push_back(new PresetButton((uint8_t) (row * 5 + col)));
        rows.push_back(new Ui::HContainer(buttons, 0, 6, { .no_animation = 1 }));
    }
    return make_drawer({
        new Ui::Element(344, 1),
        new Ui::HFlexContainer(
            new Ui::DynamicText(24, [](){
                return I18n::use_chinese ? std::string{"配装方案"} : std::string{"Loadout presets"};
            }, { .fill = Chrome::kTitle }),
            new Ui::Button(36, 36,
                new Ui::DynamicText(18, [](){ return std::string{"X"}; }),
                on_close_inventory, nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 10, {}
        ),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese
                ? std::string{"K+1~0：方案 1~10　L+1~0：方案 11~20"}
                : std::string{"K+1..0: 1..10   L+1..0: 11..20"};
        }, { .fill = Chrome::kMuted }),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese
                ? std::string{"点按切换　·　长按保存　·　缺花瓣降级或留空"}
                : std::string{"Tap=apply · Hold=save · Missing→lower/empty"};
        }, { .fill = Chrome::kMuted }),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? std::string{"手机 R：点按换主副槽　·　长按循环下一方案"}
                : std::string{"Mobile R: tap swap-all · hold next preset"};
        }, { .fill = Chrome::kMuted }),
        new Ui::VContainer(rows, 0, 6, { .no_animation = 1 }),
        new Ui::Button(180, 44,
            new Ui::DynamicText(16, [](){
                if (preset_save_mode)
                    return I18n::use_chinese ? std::string{"选择要保存的槽位"} : std::string{"Choose a slot to save"};
                return I18n::use_chinese ? std::string{"保存当前配装"} : std::string{"Save current loadout"};
            }),
            [](Element *, uint8_t e){ if (e == Ui::kClick) preset_save_mode = !preset_save_mode; },
            nullptr,
            { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }
        ),
        new Ui::DynamicText(14, [](){
            if (!Game::account_logged_in)
                return I18n::use_chinese ? std::string{"登录账号后跨设备同步方案和天赋"}
                    : std::string{"Sign in to sync presets and talents"};
            return Game::loadout_preset_status;
        }, { .fill = Chrome::kGold })
    }, Game::kPanelPresets);
}

static uint32_t talent_player_level() {
    if (Game::alive()) {
        uint32_t lv = score_to_level(Game::score);
        return lv < 1 ? 1 : lv;
    }
    if (Game::simulation.ent_exists(Game::camera_id))
        return Game::simulation.get_ent(Game::camera_id).get_respawn_level();
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
        new Ui::DynamicText(24, [](){ return I18n::tr(I18n::kPanelTalents); }, {
            .fill = Chrome::kTitle, .h_justify = Style::Left
        }),
        new Ui::DynamicText(15, [](){
            uint32_t lv = talent_player_level();
            uint32_t free = Talent::points_free(lv, Game::talent_hp, Game::talent_rot);
            uint8_t rank_limit = Talent::max_rank(lv);
            return I18n::use_chinese
                ? std::format("等级 {}　·　可用点数 {}　·　单项上限 {} 阶", lv, free, rank_limit)
                : std::format("Level {} · {} pts · rank limit {}", lv, free, rank_limit);
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese
                ? "每升1级得1点；1级可加首阶，每10级解锁下一阶。"
                : "+1 pt per level; first rank at 1, next every 10 levels.";
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(158, 60,
                new Ui::VContainer({
                    new Ui::DynamicText(16, [](){
                        return I18n::use_chinese ? "体魄" : "Vitality";
                    }, { .fill = Chrome::kTitle }),
                    new Ui::DynamicText(14, [](){
                        return std::format("Lv{}  +{:.0f}HP",
                            (int) Game::talent_hp,
                            Talent::health_bonus(Game::talent_hp));
                    }, { .fill = Chrome::kBody })
                }, 0, 3, {}),
                on_talent_hp, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(158, 60,
                new Ui::VContainer({
                    new Ui::DynamicText(16, [](){
                        return I18n::use_chinese ? "转速" : "Spin";
                    }, { .fill = Chrome::kTitle }),
                    new Ui::DynamicText(14, [](){
                        return std::format("Lv{}  +{:.1f}rad/s",
                            (int) Game::talent_rot,
                            Talent::rot_bonus(Game::talent_rot));
                    }, { .fill = Chrome::kBody })
                }, 0, 3, {}),
                on_talent_rot, nullptr,
                { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius })
        }, 0, 12, {}),
        new Ui::DynamicText(13, [](){
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
        }, { .fill = Chrome::kBody, .h_justify = Style::Left }),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? "局内：按住 Q/E 调速，或拖动槽右侧圆轮"
                : "In-game: hold Q/E or drag the spin wheel";
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [](){ return Game::talent_status; }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        })
    }, Game::kPanelTalents);
}

class CraftHeader final : public Ui::Container {
public:
    CraftHeader() : Ui::Container({
        new Ui::DynamicText(27, [](){ return I18n::tr(I18n::kPanelCraft); },
            { .fill = Chrome::kTitle }),
        new Ui::HContainer({
            new Ui::Button(94, 34,
                new Ui::DynamicText(14, [](){ return I18n::use_chinese ? "全部合成" : "Craft All"; }),
                on_craft_all, nullptr,
                { .fill = Chrome::kBtnStone, .round_radius = 7 }),
            new Ui::Button(36, 34,
                new Ui::DynamicText(23, [](){ return std::string{"×"}; }),
                on_close_craft, nullptr,
                { .fill = Chrome::kBtnDanger, .round_radius = 7 })
        }, 0, 10, {})
    }, 575, 44, { .no_animation = 1 }) {}
    void refactor() override {
        for (Element *child : children) child->refactor();
        width = 575;
        height = 44;
        children[0]->x = 0;
        children[0]->y = 0;
        children[1]->x = (width - children[1]->width) / 2 - 3;
        children[1]->y = 0;
    }
};

Element *Ui::make_craft_panel() {
    Element *elt = new Ui::VContainer({
        new CraftHeader(),
        new Ui::HContainer({
            new CraftTrayView(),
            new Ui::VContainer({
                new Ui::Button(112, 48,
                    new Ui::DynamicText(19, [](){ return I18n::use_chinese ? "合成" : "Craft"; }),
                    on_craft_once, nullptr,
                    { .fill = Chrome::kBtnStone, .round_radius = 8 }),
                new Ui::DynamicText(15, [](){
                    return I18n::use_chinese ? "将五个相同的花瓣" : "Place five matching petals";
                }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
                new Ui::DynamicText(15, [](){
                    return I18n::use_chinese ? "合成为更高稀有度" : "to craft a higher rarity";
                }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
                new Ui::DynamicText(13, [](){
                    if (Game::craft_tray_id == PetalID::kNone) return std::string{};
                    uint8_t r = PETAL_DATA[Game::craft_tray_id].rarity;
                    if (r + 1 >= RarityID::kNumRarities)
                        return I18n::use_chinese ? std::string{"已达最高稀有度"} : std::string{"Max rarity"};
                    return std::format("{} {:.0f}%", I18n::rarity_name((uint8_t)(r + 1)),
                                       Craft::success_chance(r) * 100.f);
                }, { .fill = Chrome::kBody, .h_justify = Style::Left })
            }, 0, 7, { .h_justify = Style::Left })
        }, 0, 14, { .h_justify = Style::Left }),
        make_craft_material_grid(),
        new Ui::DynamicText(14, [](){ return Game::craft_status; }, {
            .fill = Chrome::kBody, .h_justify = Style::Left
        })
    }, 18, 10, {
        .fill = Chrome::kPanelDeep,
        .line_width = Chrome::kRim,
        .round_radius = 9,
        .animate = [](Element *elt, Renderer &ctx){
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 16.f));
        },
        .should_render = [](){
            return Game::game_panel == Game::kPanelCraft
                && (Game::alive() || Game::on_game_screen)
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    elt->x = Input::is_mobile ? 82.f : 100.f;
    elt->y = -18.f;
    Ui::game_panel_roots[Game::kPanelCraft] = elt;
    return elt;
}

static std::string bonus_clock(uint32_t sec) {
    return std::format("{:02}:{:02}:{:02}", sec / 3600, (sec / 60) % 60, sec % 60);
}

static uint32_t visible_bonus_seconds(bool daily) {
    uint32_t today = Game::daily_bonus_seconds;
    uint32_t banked = Game::offline_bonus_seconds;
    // Only preview burn while the toggle is ON (matches server spend rules).
    if (Game::bonus_enabled && Game::alive() && Game::account_logged_in
        && Game::timestamp > Game::bonus_synced_at) {
        uint32_t elapsed = (uint32_t)((Game::timestamp - Game::bonus_synced_at) / 1000.0);
        uint32_t from_daily = std::min(elapsed, today);
        today -= from_daily;
        elapsed -= from_daily;
        banked -= std::min(elapsed, banked);
    }
    return daily ? today : banked;
}

class BonusToggleButton final : public Ui::ToggleButton {
public:
    BonusToggleButton() : ToggleButton(36, &Game::bonus_enabled) {}
    void on_event(uint8_t event) override {
        ToggleButton::on_event(event);
        if (event == kClick) Game::set_bonus_enabled(Game::bonus_enabled);
    }
};

Element *Ui::make_boost_panel() {
    Element *elt = make_drawer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(22, [](){
                return I18n::use_chinese ? "三倍掉落加成" : "Triple drop bonus";
            }, { .fill = Chrome::kTitle }),
            new Ui::Button(34, 34,
                new Ui::DynamicText(21, [](){ return std::string{"×"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) Game::close_game_panel(); },
                nullptr, { .fill = Chrome::kBtnDanger, .round_radius = 6 }),
            0, 12),
        new Ui::HContainer({
            new BonusToggleButton(),
            new Ui::DynamicText(18, [](){
                if (!Game::account_logged_in)
                    return I18n::use_chinese ? "登录后可开关" : "Sign in to toggle";
                return Game::bonus_enabled
                    ? (I18n::use_chinese ? "加成已开启" : "Bonus ON")
                    : (I18n::use_chinese ? "加成已关闭" : "Bonus OFF");
            }, { .fill = Chrome::kTitle })
        }, 0, 12, { .h_justify = Style::Left }),
        new Ui::VContainer({
            new Ui::DynamicText(15, [](){ return I18n::use_chinese ? "每日加成" : "Daily bonus"; },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(25, [](){ return bonus_clock(visible_bonus_seconds(true)); },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "每天 1 小时 · 北京时间零点刷新" : "One hour daily · resets at Beijing midnight";
            }, { .fill = Chrome::kBody, .h_justify = Style::Left })
        }, 12, 7, { .fill = 0xff73c77a, .round_radius = 7, .h_justify = Style::Left }),
        new Ui::VContainer({
            new Ui::DynamicText(15, [](){ return I18n::use_chinese ? "离线加成" : "Offline bonus"; },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(25, [](){ return bonus_clock(visible_bonus_seconds(false)); },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "离线 10 分钟积 1 分钟 · 最多存 3 小时" : "10 minutes away earns 1 minute · cap 3 hours";
            }, { .fill = Chrome::kBody, .h_justify = Style::Left })
        }, 12, 7, { .fill = 0xff73c77a, .round_radius = 7, .h_justify = Style::Left }),
        new Ui::DynamicText(13, [](){
            if (!Game::account_logged_in)
                return I18n::use_chinese ? "登录账号后获得加成" : "Sign in to earn bonus time";
            return Game::bonus_enabled
                ? (I18n::use_chinese
                    ? "开启中：消耗时间并三倍掉落（先每日后离线）"
                    : "ON: burns time and triples drops (daily then offline)")
                : (I18n::use_chinese
                    ? "已关闭：不消耗时间，也不会三倍掉落"
                    : "OFF: timers pause and drops stay normal");
        }, { .fill = Chrome::kTitle, .h_justify = Style::Left })
    }, Game::kPanelBoost);
    elt->style.fill = 0xff5db56e;
    elt->style.line_width = Chrome::kRim;
    elt->style.v_justify = Style::Top;
    elt->x = 12.f;
    elt->y = 72.f;
    return elt;
}

// Assembly workshop (V): A + B = Result [hammer], bottom-left drawer like backpack.
static uint8_t recipe_filter_tier = 0; // 0 base · 3 special · 255 all
static uint16_t recipe_selected = 0;

static uint8_t recipe_can_craft(uint16_t idx) {
    if (idx >= Recipe::recipe_count()) return 0;
    Recipe::Def const &rec = Recipe::recipe_at(idx);
    for (uint8_t i = 0; i < rec.ingredient_count; ++i)
        if (Game::craft_owned(rec.ingredients[i].id) < rec.ingredients[i].count)
            return 0;
    return 1;
}

class RecipePetalChip final : public Element {
    PetalID::T petal_id;
    uint16_t need;
    uint8_t show_need;
public:
    RecipePetalChip(PetalID::T id, uint16_t n, uint8_t with_need)
        : Element(72, 84, { .no_animation = 1 }),
          petal_id(id), need(n), show_need(with_need) {}
    void on_render(Renderer &ctx) override {
        uint32_t fill = Chrome::kPanelInset;
        uint8_t rarity = 0;
        if (petal_id != PetalID::kNone && petal_id < PetalID::kNumPetals) {
            rarity = PETAL_DATA[petal_id].rarity;
            if (rarity < RarityID::kNumRarities) fill = RARITY_COLORS[rarity];
        }
        uint32_t owned = (petal_id < PetalID::kNumPetals) ? Game::craft_owned(petal_id) : 0;
        if (show_need && owned < need) fill = 0xff5a5a5a;
        Chrome::draw_plate(ctx, width, height, fill, Chrome::kRadius, 0.f);
        if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
        {
            RenderContext c(&ctx);
            ctx.translate(0, -12);
            ctx.scale(1.05f);
            if (PETAL_DATA[petal_id].radius > 18)
                ctx.scale(18.f / PETAL_DATA[petal_id].radius);
            draw_static_petal(petal_id, ctx);
        }
        ctx.translate(0, height / 2 - 28);
        ctx.draw_text(I18n::petal_name(petal_id), {
            .fill = Chrome::kTitle, .size = 12, .stroke_scale = 0.1f
        });
        if (show_need) {
            ctx.translate(0, 16);
            uint32_t col = owned >= need ? 0xff7CFF8A : 0xffff6b6b;
            ctx.draw_text(std::format("x{} / x{}", need, owned).c_str(), {
                .fill = col, .size = 12, .stroke_scale = 0.1f
            });
            ctx.translate(0, -16);
        } else {
            ctx.translate(0, 16);
            ctx.draw_text("x1", {
                .fill = Chrome::kGold, .size = 12, .stroke_scale = 0.1f
            });
            ctx.translate(0, -16);
        }
        ctx.translate(0, -(height / 2 - 28));
    }
};

class RecipeRow final : public Element {
    uint16_t recipe_index;
public:
    explicit RecipeRow(uint16_t idx)
        : Element(510, 96, { .no_animation = 1 }), recipe_index(idx) {
        style.should_render = [idx](){
            if (Game::game_panel != Game::kPanelSpecial) return 0;
            if (!Game::should_render_game_ui()) return 0;
            if (idx >= Recipe::recipe_count()) return 0;
            if (recipe_filter_tier != 255
                && Recipe::recipe_at(idx).tier != recipe_filter_tier)
                return 0;
            return 1;
        };
    }
    void refactor() override {
        visible = style.should_render();
        width = Input::is_mobile ? 330.f : 510.f;
        height = 96.f;
    }
    void on_render(Renderer &ctx) override {
        if (recipe_index >= Recipe::recipe_count()) return;
        Recipe::Def const &rec = Recipe::recipe_at(recipe_index);
        uint8_t can = recipe_can_craft(recipe_index);
        uint8_t sel = recipe_selected == recipe_index;
        Chrome::draw_plate(ctx, width, height,
            sel ? 0xff3ecf8e : Chrome::kPanelFace, Chrome::kRadius, 0.f);
        if (sel) {
            ctx.set_stroke(0xff7CFF8A);
            ctx.set_line_width(3.f);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, Chrome::kRadius);
            ctx.stroke();
        }
        float x = -width / 2 + 46;
        auto draw_chip = [&](PetalID::T id, uint16_t need, uint8_t show_need) {
            RenderContext c(&ctx);
            ctx.translate(x, 0);
            RecipePetalChip chip(id, need, show_need);
            chip.width = 68;
            chip.height = 80;
            chip.on_render(ctx);
            x += 78;
        };
        if (rec.ingredient_count >= 1)
            draw_chip(rec.ingredients[0].id, rec.ingredients[0].count, 1);
        if (rec.ingredient_count >= 2) {
            {
                RenderContext c(&ctx);
                ctx.translate(x - 6, 0);
                ctx.draw_text("+", { .fill = 0xffffffff, .size = 28, .stroke_scale = 0.08f });
            }
            x += 18;
            draw_chip(rec.ingredients[1].id, rec.ingredients[1].count, 1);
        }
        {
            RenderContext c(&ctx);
            ctx.translate(x - 4, 0);
            ctx.draw_text("=", { .fill = 0xffffffff, .size = 28, .stroke_scale = 0.08f });
        }
        x += 20;
        draw_chip(rec.result, 1, 0);
        if (rec.ingredient_count > 2) {
            RenderContext c(&ctx);
            ctx.translate(x + 10, height / 2 - 14);
            std::string extra = I18n::use_chinese ? "还需要 " : "Also ";
            for (uint8_t i = 2; i < rec.ingredient_count; ++i) {
                if (i > 2) extra += ", ";
                extra += I18n::petal_name(rec.ingredients[i].id);
                extra += std::format("×{}", rec.ingredients[i].count);
            }
            ctx.draw_text(extra.c_str(), {
                .fill = can ? Chrome::kGold : 0xffffaaaa, .size = 11, .stroke_scale = 0.1f
            });
        }
        // Hammer craft button
        {
            RenderContext c(&ctx);
            float bx = width / 2 - 42;
            ctx.translate(bx, 0);
            Chrome::draw_plate(ctx, 56, 56,
                can ? Chrome::kBtnMoss : Chrome::kBtnIron, Chrome::kRadius, 0.f);
            ctx.set_fill(0xffffe066);
            ctx.begin_path();
            ctx.move_to(-10, 8);
            ctx.line_to(12, -10);
            ctx.line_to(16, -6);
            ctx.line_to(-6, 12);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffc98418);
            ctx.begin_path();
            ctx.round_rect(-4, 6, 8, 16, 2);
            ctx.fill();
        }
    }
    void on_event(uint8_t event) override {
        if (event == kClick) {
            recipe_selected = recipe_index;
            if (recipe_can_craft(recipe_index))
                Game::craft_recipe(recipe_index);
        }
    }
};

static Element *make_recipe_scroll() {
    std::vector<Element *> rows;
    uint16_t n = Recipe::recipe_count();
    for (uint16_t i = 0; i < n; ++i)
        rows.push_back(new RecipeRow(i));
    Element *list = new Ui::VContainer(rows, 0, 8, { .no_animation = 1 });
    float scroll_h = Input::is_mobile ? 360.f : 520.f;
    auto *scroll = new BagScroll(list, scroll_h);
    scroll->always_capture_wheel = 1;
    return scroll;
}

static void on_recipe_tier_base(Element *, uint8_t e) {
    if (e == Ui::kClick) recipe_filter_tier = 0;
}
static void on_recipe_tier_cross(Element *, uint8_t e) {
    if (e == Ui::kClick) recipe_filter_tier = 1;
}
static void on_recipe_tier_legend(Element *, uint8_t e) {
    if (e == Ui::kClick) recipe_filter_tier = 2;
}
static void on_recipe_tier_special(Element *, uint8_t e) {
    if (e == Ui::kClick) recipe_filter_tier = 3;
}
static void on_recipe_tier_all(Element *, uint8_t e) {
    if (e == Ui::kClick) recipe_filter_tier = 255;
}

Element *Ui::make_special_craft_panel() {
    return make_drawer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(26, [](){
                return I18n::use_chinese ? "组装" : "Assemble";
            }, { .fill = Chrome::kTitle }),
            new Ui::Button(36, 36,
                new Ui::DynamicText(18, [](){ return std::string{"X"}; }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::close_game_panel();
                }, nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 10, {}
        ),
        new Ui::DynamicText(14, [](){
            uint32_t n = Game::craft_owned(PetalID::kCoin);
            return I18n::use_chinese
                ? std::format("钱币花瓣：{}　原料与成品都在背包里", n)
                : std::format("Coin petals: {} · mats & results live in bag", n);
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(72, 36,
                new Ui::DynamicText(13, [](){ return I18n::use_chinese ? "基础" : "Base"; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_base,
                [](){ return recipe_filter_tier != 0; },
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(72, 36,
                new Ui::DynamicText(13, [](){ return I18n::use_chinese ? "跨图" : "Cross"; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_cross,
                [](){ return recipe_filter_tier != 1; },
                { .fill = Chrome::kBtnClay, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(72, 36,
                new Ui::DynamicText(13, [](){ return I18n::use_chinese ? "传说" : "Legend"; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_legend,
                [](){ return recipe_filter_tier != 2; },
                { .fill = Chrome::kBtnStone, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(72, 36,
                new Ui::DynamicText(13, [](){ return I18n::use_chinese ? "特殊" : "Shop"; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_special,
                [](){ return recipe_filter_tier != 3; },
                { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(72, 36,
                new Ui::DynamicText(13, [](){ return I18n::use_chinese ? "全部" : "All"; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_all,
                [](){ return recipe_filter_tier != 255; },
                { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }),
        }, 0, 6, { .h_justify = Style::Left }),
        make_recipe_scroll(),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? "材料充足时点锤头合成，成品进入背包。"
                : "Tap the hammer when ready — result goes to bag.";
        }, { .fill = Chrome::kMuted, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [](){ return Game::craft_status; }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        })
    }, Game::kPanelSpecial);
}

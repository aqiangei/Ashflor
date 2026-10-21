#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/HudIcons.hh>
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
#include <Helpers/Math.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
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
    // Bank stacks must still list when loadout_count is briefly 0 (title/death sync).
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
        // Same sticky-blank trap as guild rows: layout runs before render.
        // Keep animation in sync so a newly filled stack cell is not clipped
        // at width 0 until a full page refresh recreates the tree.
        uint8_t want = style.should_render ? style.should_render() : 1;
        visible = want;
        animation.set(want);
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
            dragging = active && (Ui::UiLoadout::num_petals_selected != 0
                || Ui::bag_gesture_ghost_petal() != PetalID::kNone);
            rendering_tooltip = active && !dragging;
            if (!active) {
                holding = 0;
                drag_touch = (uint32_t)-1;
            }
        } else if (!Input::is_mobile && holding) {
            dragging = Ui::bag_gesture_ghost_petal() == id()
                || (Ui::UiLoadout::num_petals_selected != 0
                    && slot_static() != (uint8_t) -1);
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
            std::string badge = format_stack(stack);
            float const sz = 13.f;
            ctx.set_text_size(sz);
            float tw = (float) ctx.get_text_size(badge.c_str());
            float bw = std::max(24.f, tw + 10.f);
            float bx = width / 2 - bw * 0.5f;
            float by = -height / 2 + 10;
            ctx.set_fill(0xe0000000);
            ctx.begin_path();
            ctx.round_rect(bx - bw * 0.5f, by - 9, bw, 18, 8);
            ctx.fill();
            {
                RenderContext c(&ctx);
                ctx.translate(bx, by);
                ctx.center_text_align();
                ctx.center_text_baseline();
                ctx.draw_text(badge.c_str(), {
                    .fill = Chrome::kGold, .size = sz, .stroke_scale = 0.1f
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
                uint8_t slot = slot_static();
                if (slot != (uint8_t) -1)
                    begin_bag_gesture(slot, press_x, press_y, drag_touch);
                else if (Game::craft_bank[pid] > 0 && !Recipe::is_bag_only(pid))
                    begin_bag_gesture((uint8_t) -1, press_x, press_y, drag_touch, pid);
            }
            rendering_tooltip = 1;
            tooltip = Ui::UiLoadout::petal_tooltips[pid];
            return;
        }
        if (event == kClick) {
            // Equip / drop is owned by finish_bag_gesture (tap + drag-to-slot).
            holding = 0;
            dragging = 0;
            drag_touch = (uint32_t)-1;
            rendering_tooltip = 0;
            return;
        }
        if (event == kFocusLost) {
            // Leaving the cell must not cancel an in-flight bag→slot drag.
            if (holding && Ui::bag_gesture_busy()) {
                if (Ui::UiLoadout::num_petals_selected != 0
                    || Ui::bag_gesture_ghost_petal() != PetalID::kNone)
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
            // Hard stop on runaway width (HFlex stretch ↔ VContainer feedback).
            float const cap = Input::is_mobile ? 480.f : 760.f;
            if (elt->width > cap) elt->width = cap;
            ctx.translate(0, (1.f - (float) elt->animation) * (elt->height + 16.f));
        },
        .should_render = [panel_id](){
            return Game::game_panel == panel_id
                && (Game::alive()
                    || ((panel_id == Game::kPanelInventory
                        || panel_id == Game::kPanelTalents
                        || panel_id == Game::kPanelPresets
                        || panel_id == Game::kPanelCraft
                        || panel_id == Game::kPanelSpecial
                        || panel_id == Game::kPanelShop)
                        && Game::on_game_screen))
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Left,
        .v_justify = Style::Bottom
    });
    // Sit clear of side plaques (humanized: not jammed into loadout)
    elt->x = Input::is_mobile ? 92.f : 72.f;
    elt->y = -20;
    if (panel_id < Game::kPanelCount) Ui::game_panel_roots[panel_id] = elt;
    return elt;
}

static constexpr float kBagPanelW = 600.f;
static constexpr float kBagPanelWMobile = 460.f;
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
        uint8_t want = style.should_render ? style.should_render() : 1;
        visible = want;
        animation.set(want);
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
static void shop_snap_rarity_to_tab();
static void on_shop_btn(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    Game::toggle_game_panel(Game::kPanelShop);
    if (Game::game_panel == Game::kPanelShop)
        shop_snap_rarity_to_tab();
}
static uint8_t preset_save_mode = 0;
static bool inventory_active() { return Game::game_panel == Game::kPanelInventory; }
static bool talents_active() { return Game::game_panel == Game::kPanelTalents; }
static bool craft_active() { return Game::game_panel == Game::kPanelCraft; }
static bool special_active() { return Game::game_panel == Game::kPanelSpecial; }
static bool shop_active() { return Game::game_panel == Game::kPanelShop; }

// ── Craft motion FX (place / take-back / win / miss) ─────────────────────
namespace CraftFx {
enum Kind : uint8_t { kToTray = 0, kFromTray = 1, kToBag = 2, kMissBurst = 3 };
struct Fly {
    PetalID::T id = PetalID::kNone;
    float x0 = 0, y0 = 0, x1 = 0, y1 = 0; // screen px
    float t = 0; // 0..1
    float dur = 280.f; // ms
    Kind kind = kToTray;
    float spin = 0.f;
};
struct Speck {
    float x = 0, y = 0, vx = 0, vy = 0;
    float life = 0, max_life = 1.f;
    uint32_t color = 0xffffffff;
    float size = 3.f;
};

static constexpr uint32_t kMaxFlies = 14;
static constexpr uint32_t kMaxSpecks = 48;
static Fly flies[kMaxFlies];
static uint32_t fly_n = 0;
static Speck specks[kMaxSpecks];
static uint32_t speck_n = 0;
static float tray_pulse = 0.f;
static float tray_shake = 0.f;
static float tray_spin = 0.f; // radians, during craft absorb
static float win_flash = 0.f;
static Element *tray_view = nullptr;
static float mat_sx[PetalID::kNumPetals] = {};
static float mat_sy[PetalID::kNumPetals] = {};
static uint8_t mat_seen[PetalID::kNumPetals] = {};

static float ease_out(float t) {
    float u = 1.f - t;
    return 1.f - u * u * u;
}
static float ease_in_out(float t) {
    return t < 0.5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) * 0.5f;
}

static void spawn_fly(PetalID::T id, float x0, float y0, float x1, float y1,
                      Kind kind, float dur_ms = 280.f) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    if (fly_n >= kMaxFlies) {
        // Drop oldest
        for (uint32_t i = 1; i < fly_n; ++i) flies[i - 1] = flies[i];
        --fly_n;
    }
    Fly &f = flies[fly_n++];
    f.id = id;
    f.x0 = x0; f.y0 = y0; f.x1 = x1; f.y1 = y1;
    f.t = 0.f;
    f.dur = dur_ms;
    f.kind = kind;
    f.spin = (kind == kMissBurst) ? (frand() * 2.f - 1.f) * 3.f : 0.f;
}

static void spawn_speck(float x, float y, uint32_t color) {
    if (speck_n >= kMaxSpecks) {
        for (uint32_t i = 1; i < speck_n; ++i) specks[i - 1] = specks[i];
        --speck_n;
    }
    Speck &s = specks[speck_n++];
    float ang = frand() * (float) (2.0 * M_PI);
    float spd = 40.f + frand() * 120.f;
    s.x = x; s.y = y;
    s.vx = std::cos(ang) * spd;
    s.vy = std::sin(ang) * spd - 30.f;
    s.life = 0.f;
    s.max_life = 0.35f + frand() * 0.35f;
    s.color = color;
    s.size = 2.f + frand() * 3.5f;
}

static void tray_local_slot(uint32_t i, float &lx, float &ly) {
    float ang = (float) (-M_PI / 2 + i * (2 * M_PI / Craft::kCraftBatch));
    lx = std::cos(ang) * 88.f;
    ly = 8.f + std::sin(ang) * 88.f;
}

static uint8_t tray_slot_screen(uint32_t i, float &sx, float &sy) {
    if (tray_view == nullptr || !tray_view->visible) return 0;
    float lx, ly;
    tray_local_slot(i, lx, ly);
    sx = tray_view->screen_x + lx * (float) Ui::scale;
    sy = tray_view->screen_y + ly * (float) Ui::scale;
    return 1;
}

static uint8_t tray_center_screen(float &sx, float &sy) {
    if (tray_view == nullptr || !tray_view->visible) return 0;
    sx = tray_view->screen_x;
    sy = tray_view->screen_y + 8.f * (float) Ui::scale;
    return 1;
}

static uint8_t bag_target_screen(float &sx, float &sy) {
    Element *aim = Ui::game_side_buttons_root;
    if (aim == nullptr || !aim->visible) {
        sx = 48.f * (float) Ui::scale;
        sy = (float) Ui::window_height - 120.f * (float) Ui::scale;
        return 1;
    }
    sx = aim->screen_x;
    sy = aim->screen_y - 40.f * (float) Ui::scale;
    return 1;
}

static void remember_mat(PetalID::T id, float sx, float sy) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    mat_sx[id] = sx;
    mat_sy[id] = sy;
    mat_seen[id] = 1;
}

static uint8_t mat_screen(PetalID::T id, float &sx, float &sy) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals || !mat_seen[id])
        return 0;
    sx = mat_sx[id];
    sy = mat_sy[id];
    return 1;
}

static void fly_to_tray(PetalID::T id, float from_sx, float from_sy) {
    uint32_t slot = Game::craft_tray_count;
    if (slot > 0) --slot; // just-added last visible slot
    if (slot >= Craft::kCraftBatch) slot = Craft::kCraftBatch - 1;
    float tx, ty;
    if (!tray_slot_screen(slot, tx, ty))
        if (!tray_center_screen(tx, ty)) return;
    spawn_fly(id, from_sx, from_sy, tx, ty, kToTray, 260.f);
    tray_pulse = 1.f;
}

static void fly_from_tray(PetalID::T id, uint32_t slot_hint) {
    float sx, sy, tx, ty;
    if (slot_hint >= Craft::kCraftBatch) slot_hint = Craft::kCraftBatch - 1;
    if (!tray_slot_screen(slot_hint, sx, sy))
        if (!tray_center_screen(sx, sy)) return;
    if (!mat_screen(id, tx, ty)) {
        bag_target_screen(tx, ty);
    }
    spawn_fly(id, sx, sy, tx, ty, kFromTray, 260.f);
}

static void begin_craft_absorb() {
    tray_spin = 0.f;
    tray_pulse = 1.f;
}

static void play_result(uint8_t result, PetalID::T petal) {
    float cx, cy;
    if (!tray_center_screen(cx, cy)) {
        cx = (float) Ui::window_width * 0.35f;
        cy = (float) Ui::window_height * 0.55f;
    }
    if (result == CraftResultCode::kCraftWin
        || result == CraftResultCode::kCraftExchanged
        || result == CraftResultCode::kCraftBought) {
        win_flash = 1.f;
        tray_pulse = 1.f;
        PetalID::T show = (petal != PetalID::kNone && petal < PetalID::kNumPetals)
            ? petal : Game::craft_tray_id;
        if (show == PetalID::kNone) show = petal;
        // Fly toward the higher-rarity result cell in the craft grid, not the bag.
        float tx, ty;
        if (!mat_screen(show, tx, ty))
            bag_target_screen(tx, ty);
        spawn_fly(show, cx, cy, tx, ty, kToBag, 420.f);
        uint32_t col = 0xffffe763;
        if (show < PetalID::kNumPetals) {
            uint8_t r = PETAL_DATA[show].rarity;
            if (r < RarityID::kNumRarities) col = RARITY_COLORS[r] | 0xff000000;
        }
        for (uint32_t i = 0; i < 18; ++i)
            spawn_speck(cx, cy, col);
    } else if (result == CraftResultCode::kCraftMiss) {
        tray_shake = 1.f;
        for (uint32_t i = 0; i < Craft::kCraftBatch; ++i) {
            float sx, sy;
            if (!tray_slot_screen(i, sx, sy)) { sx = cx; sy = cy; }
            float ang = frand() * (float) (2.0 * M_PI);
            float dist = 50.f + frand() * 90.f;
            spawn_fly(PetalID::kBasic, sx, sy,
                      sx + std::cos(ang) * dist, sy + std::sin(ang) * dist,
                      kMissBurst, 320.f);
        }
        for (uint32_t i = 0; i < 14; ++i)
            spawn_speck(cx, cy, 0xff886655);
    }
}

static void tick(float dt_ms) {
    if (tray_pulse > 0.f) {
        tray_pulse -= dt_ms / 320.f;
        if (tray_pulse < 0.f) tray_pulse = 0.f;
    }
    if (tray_shake > 0.f) {
        tray_shake -= dt_ms / 380.f;
        if (tray_shake < 0.f) tray_shake = 0.f;
    }
    if (win_flash > 0.f) {
        win_flash -= dt_ms / 500.f;
        if (win_flash < 0.f) win_flash = 0.f;
    }
    if (Game::game_panel == Game::kPanelCraft
        && Game::craft_tray_id == PetalID::kNone
        && tray_spin < 6.5f && tray_pulse > 0.2f)
        tray_spin += dt_ms * 0.012f;

    for (uint32_t i = 0; i < fly_n; ) {
        Fly &f = flies[i];
        f.t += dt_ms / f.dur;
        if (f.t >= 1.f) {
            flies[i] = flies[--fly_n];
            continue;
        }
        ++i;
    }
    float dt = dt_ms / 1000.f;
    for (uint32_t i = 0; i < speck_n; ) {
        Speck &s = specks[i];
        s.life += dt;
        s.x += s.vx * dt;
        s.y += s.vy * dt;
        s.vy += 220.f * dt;
        if (s.life >= s.max_life) {
            specks[i] = specks[--speck_n];
            continue;
        }
        ++i;
    }
}

static void render_overlay(Renderer &ctx) {
    // Specks in screen space (window coords via reset like tooltips)
    for (uint32_t i = 0; i < speck_n; ++i) {
        Speck const &s = specks[i];
        float a = 1.f - s.life / s.max_life;
        float ux = (s.x - (float) Ui::window_width * 0.5f) / (float) Ui::scale;
        float uy = (s.y - (float) Ui::window_height * 0.5f) / (float) Ui::scale;
        RenderContext g(&ctx);
        ctx.translate(ux, uy);
        ctx.set_global_alpha(a);
        ctx.set_fill(s.color);
        ctx.begin_path();
        ctx.arc(0, 0, s.size);
        ctx.fill();
    }
    for (uint32_t i = 0; i < fly_n; ++i) {
        Fly const &f = flies[i];
        float e = (f.kind == kToBag) ? ease_in_out(f.t) : ease_out(f.t);
        float x = f.x0 + (f.x1 - f.x0) * e;
        float y = f.y0 + (f.y1 - f.y0) * e;
        // Arc lift so place/take-back reads as a toss
        float arc = std::sin(f.t * (float) M_PI) * (f.kind == kMissBurst ? 12.f : 36.f);
        y -= arc;
        float ux = (x - (float) Ui::window_width * 0.5f) / (float) Ui::scale;
        float uy = (y - (float) Ui::window_height * 0.5f) / (float) Ui::scale;
        float alpha = 1.f;
        float scale = 0.85f;
        if (f.kind == kToTray) {
            scale = 0.95f - 0.2f * e;
            alpha = 0.35f + 0.65f * (1.f - e * 0.3f);
        } else if (f.kind == kFromTray) {
            scale = 0.75f + 0.2f * e;
        } else if (f.kind == kToBag) {
            scale = 1.05f - 0.35f * e;
            alpha = 1.f - e * 0.15f;
        } else { // miss
            scale = 0.7f * (1.f - e);
            alpha = 1.f - e;
        }
        if (f.id == PetalID::kNone || f.id >= PetalID::kNumPetals) continue;
        if (f.kind == kMissBurst && f.id == PetalID::kBasic) {
            // Ash puff — no petal art
            RenderContext g(&ctx);
            ctx.translate(ux, uy);
            ctx.set_global_alpha(alpha * 0.7f);
            ctx.set_fill(0xff6a5040);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f * (1.f - e) + 2.f);
            ctx.fill();
            continue;
        }
        RenderContext g(&ctx);
        ctx.translate(ux, uy);
        ctx.rotate(f.spin * f.t);
        ctx.set_global_alpha(alpha);
        ctx.scale(scale);
        draw_loadout_background(ctx, f.id, 1.f, 0);
    }
    if (win_flash > 0.05f && tray_view != nullptr && tray_view->visible) {
        float ux = (tray_view->screen_x - (float) Ui::window_width * 0.5f)
            / (float) Ui::scale;
        float uy = (tray_view->screen_y - (float) Ui::window_height * 0.5f)
            / (float) Ui::scale;
        RenderContext g(&ctx);
        ctx.translate(ux, uy);
        ctx.set_global_alpha(win_flash * 0.45f);
        ctx.set_fill(0xffffe763);
        ctx.begin_path();
        ctx.arc(0, 8.f, 70.f * (1.2f - win_flash * 0.4f));
        ctx.fill();
    }
}
} // namespace CraftFx

static void on_craft_once(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    if (Game::craft_tray_id == PetalID::kNone || Game::craft_tray_count < Craft::kCraftBatch)
        return;
    PetalID::T id = Game::craft_tray_id;
    uint32_t batches = Game::craft_tray_count / Craft::kCraftBatch;
    if (batches > 200) batches = 200;
    uint32_t used = batches * Craft::kCraftBatch;
    CraftFx::begin_craft_absorb();
    Game::craft_petals(CraftMode::kCraftGamble, id, (uint16_t) batches);
    if (Game::craft_tray_count <= used) Game::craft_tray_clear();
    else Game::craft_tray_count -= used;
}
static void on_craft_all(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    PetalID::T id = Game::craft_tray_id;
    if (id == PetalID::kNone) return;
    CraftFx::begin_craft_absorb();
    Game::craft_petals(CraftMode::kCraftGambleAll, id, 200);
    Game::craft_tray_clear();
}
static void on_close_craft(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::close_game_panel();
}

static uint8_t shift_alt_held() {
    return Input::keys_held.contains('\x10') && Input::keys_held.contains((char) 18);
}

static void craft_tray_add_animated(PetalID::T id, uint32_t amount, float from_sx, float from_sy) {
    if (id == PetalID::kNone || amount == 0) return;
    // Different petal already on the tray → kick the old stack back to the mat row.
    if (Game::craft_tray_id != PetalID::kNone
        && Game::craft_tray_id != id
        && Game::craft_tray_count > 0) {
        PetalID::T old = Game::craft_tray_id;
        uint32_t n = Game::craft_tray_count;
        uint32_t slots = n < Craft::kCraftBatch ? n : Craft::kCraftBatch;
        for (uint32_t i = 0; i < slots; ++i)
            CraftFx::fly_from_tray(old, i);
        Game::craft_tray_clear();
    }
    uint32_t before = (Game::craft_tray_id == id) ? Game::craft_tray_count : 0;
    Game::craft_tray_add(id, amount);
    if (Game::craft_tray_count > before)
        CraftFx::fly_to_tray(id, from_sx, from_sy);
}

static void craft_tray_take_back() {
    PetalID::T id = Game::craft_tray_id;
    if (id == PetalID::kNone || Game::craft_tray_count == 0) return;
    uint32_t n = Game::craft_tray_count;
    uint32_t take = Craft::kCraftBatch;
    if (take > n) take = n;
    uint32_t slot = n > 0 ? (n - 1) : 0;
    if (slot >= Craft::kCraftBatch) slot = Craft::kCraftBatch - 1;
    CraftFx::fly_from_tray(id, slot);
    Game::craft_tray_remove(take);
}

// Large pentagon work area, matching the warm crafting board.
class CraftTrayView final : public Element {
    float clear_anim = 0.f;
    PetalID::T ghost_id = PetalID::kNone;
    uint32_t ghost_count = 0;
public:
    CraftTrayView() : Element(Input::is_mobile ? 300.f : 400.f, Input::is_mobile ? 260.f : 300.f, {
        .animate = [this](Element *, Renderer &) {
            CraftFx::tray_view = this;
            if (Game::craft_tray_id != PetalID::kNone && Game::craft_tray_count > 0) {
                ghost_id = Game::craft_tray_id;
                ghost_count = Game::craft_tray_count;
                clear_anim = 0.f;
                return;
            }
            if (ghost_id == PetalID::kNone) return;
            clear_anim += (float)(Ui::dt / 280.0);
            if (clear_anim >= 1.f) {
                clear_anim = 0.f;
                ghost_id = PetalID::kNone;
                ghost_count = 0;
            }
        },
        .no_animation = 1
    }) {
        CraftFx::tray_view = this;
    }
    void on_render(Renderer &ctx) override {
        float shake_x = 0.f, shake_y = 0.f;
        if (CraftFx::tray_shake > 0.f) {
            float s = CraftFx::tray_shake;
            shake_x = std::sin(Game::timestamp * 0.06) * 7.f * s;
            shake_y = std::cos(Game::timestamp * 0.08) * 4.f * s;
        }
        float pulse = CraftFx::tray_pulse;
        float cx = shake_x, cy = 8.f + shake_y;
        float rad = 88.f * (1.f + pulse * 0.06f);
        float slot = 60.f;
        PetalID::T show_id = Game::craft_tray_id;
        uint32_t show_n = Game::craft_tray_count;
        float fade = 1.f;
        if ((show_id == PetalID::kNone || show_n == 0) && ghost_id != PetalID::kNone) {
            show_id = ghost_id;
            show_n = ghost_count;
            fade = 1.f - clear_anim;
        }
        // Soft board glow while pulsing / winning
        if (pulse > 0.05f || CraftFx::win_flash > 0.05f) {
            float g = std::max(pulse, CraftFx::win_flash);
            ctx.set_global_alpha(g * 0.25f);
            ctx.set_fill(0xffffe763);
            ctx.begin_path();
            ctx.arc(cx, cy, rad + 36.f);
            ctx.fill();
            ctx.set_global_alpha(1.f);
        }
        float spin = (show_n == 0 && fade < 1.f) ? clear_anim * 1.2f : 0.f;
        for (uint32_t i = 0; i < Craft::kCraftBatch; ++i) {
            float ang = (float) (-M_PI / 2 + i * (2 * M_PI / Craft::kCraftBatch)) + spin;
            float x = cx + std::cos(ang) * rad;
            float y = cy + std::sin(ang) * rad;
            ctx.set_fill(Chrome::kPanelInset);
            ctx.begin_path();
            ctx.round_rect(x - slot / 2, y - slot / 2, slot, slot, 7);
            ctx.fill();
            if (show_id != PetalID::kNone && show_n > i) {
                RenderContext guard(&ctx);
                ctx.translate(x, y);
                float pop = 1.f + pulse * 0.12f * std::sin((float) i * 1.1f + (float) Game::timestamp * 0.02f);
                float s = 0.92f * (0.55f + 0.45f * fade) * pop;
                ctx.scale(s);
                ctx.set_global_alpha(fade);
                draw_loadout_background(ctx, show_id, 1.f, 0);
            }
        }
        if (show_n > Craft::kCraftBatch && fade > 0.05f) {
            std::string t = format_stack(show_n);
            ctx.set_global_alpha(fade);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff222222);
            ctx.set_text_size(14);
            ctx.set_line_width(1.6f);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.stroke_text(t.c_str());
            ctx.fill_text(t.c_str());
            ctx.set_global_alpha(1.f);
        }
        // Hint: click tray to take back
        if (show_id != PetalID::kNone && show_n > 0 && fade > 0.9f) {
            ctx.set_global_alpha(0.55f);
            ctx.set_fill(Chrome::kMuted);
            ctx.set_text_size(12);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.fill_text(I18n::use_chinese ? "点击收回" : "Click to return");
            ctx.set_global_alpha(1.f);
        }
    }
    void on_event(uint8_t event) override {
        // kClick only — kMouseDown+kClick would take back twice.
        if (event == kClick
            && Game::craft_tray_id != PetalID::kNone
            && Game::craft_tray_count > 0)
            craft_tray_take_back();
    }
};

// Material cell: click = +5, hold accelerates, Shift+Alt = dump all
// Same English name + rarity can collide (Dahlia vs RoseRare, etc.).
// Only return an id the player actually owns — never a catalog placeholder.
static PetalID::T resolve_craft_family_petal(char const *name, uint8_t rarity) {
    if (name == nullptr || rarity >= RarityID::kNumRarities) return PetalID::kNone;
    PetalID::T best_owned = PetalID::kNone;
    uint32_t best_owned_n = 0;
    for (uint32_t raw = 1; raw < (uint32_t) PetalID::kNumPetals; ++raw) {
        PetalID::T id = (PetalID::T) raw;
        if (Recipe::is_bag_only(id)) continue;
        if (PETAL_DATA[id].rarity != rarity) continue;
        if (PETAL_DATA[id].name == nullptr) continue;
        if (std::strcmp(PETAL_DATA[id].name, name) != 0) continue;
        uint32_t n = Game::craft_owned(id);
        if (n == 0) continue;
        if (n > best_owned_n || (n == best_owned_n && id > best_owned)) {
            best_owned = id;
            best_owned_n = n;
        }
    }
    return best_owned;
}

// Catalog id for this name+rarity (for win-fly landing on empty next-tier cells).
static PetalID::T resolve_craft_family_slot(char const *name, uint8_t rarity) {
    if (name == nullptr || rarity >= RarityID::kNumRarities) return PetalID::kNone;
    PetalID::T best_any = PetalID::kNone;
    for (uint32_t raw = 1; raw < (uint32_t) PetalID::kNumPetals; ++raw) {
        PetalID::T id = (PetalID::T) raw;
        if (Recipe::is_bag_only(id)) continue;
        if (PETAL_DATA[id].rarity != rarity) continue;
        if (PETAL_DATA[id].name == nullptr) continue;
        if (std::strcmp(PETAL_DATA[id].name, name) != 0) continue;
        if (best_any == PetalID::kNone || id > best_any) best_any = id;
    }
    return best_any;
}

class CraftMatCell final : public Element {
    char const *family_name;
    uint8_t rarity;
    float hold_timer = 0;
    float hold_interval = 0.28f;
    uint8_t holding = 0;
    float empty_anim = 0.f; // 0 = empty plate; never start as a ghost catalog icon
    PetalID::T last_shown_id = PetalID::kNone;

    PetalID::T current_id() const {
        if (family_name == nullptr) return PetalID::kNone;
        PetalID::T id = resolve_craft_family_petal(family_name, rarity);
        if (id == PetalID::kNone || Game::craft_owned(id) == 0) return PetalID::kNone;
        return id;
    }
public:
    CraftMatCell(char const *name, uint8_t r)
        : Element(64, 66, {
            // Death screen keeps craft UI open (same as bag / specials).
            .should_render = [](){
                return Game::alive() || Game::on_game_screen;
            },
            .no_animation = 1
        }), family_name(name), rarity(r) {
        style.animate = [this](Element *, Renderer &){
            PetalID::T petal_id = current_id();
            // Fly target for the next-rarity column even when still empty.
            PetalID::T slot_id = resolve_craft_family_slot(family_name, rarity);
            if (slot_id != PetalID::kNone && screen_x > 1.f)
                CraftFx::remember_mat(slot_id, screen_x, screen_y);
            uint32_t avail = (petal_id == PetalID::kNone) ? 0 : Game::craft_available(petal_id);
            // Hide icon when unowned or all copies are on the tray.
            float target = (petal_id != PetalID::kNone && avail > 0) ? 1.f : 0.f;
            if (petal_id != PetalID::kNone && avail > 0) last_shown_id = petal_id;
            float step = (float)(Ui::dt / 180.0);
            if (empty_anim < target) empty_anim = std::min(target, empty_anim + step);
            else if (empty_anim > target) empty_anim = std::max(target, empty_anim - step);
            if (empty_anim <= 0.f) last_shown_id = PetalID::kNone;

            if (!holding) return;
            if (!(focus_state == kMouseDown)) {
                holding = 0;
                hold_timer = 0;
                hold_interval = 0.28f;
                return;
            }
            if (petal_id == PetalID::kNone || avail == 0) {
                holding = 0;
                return;
            }
            hold_timer += (float) (Ui::dt / 1000.0);
            while (hold_timer >= hold_interval) {
                hold_timer -= hold_interval;
                craft_tray_add_animated(petal_id, Craft::kCraftBatch, screen_x, screen_y);
                hold_interval = std::max(0.04f, hold_interval * 0.82f);
                if (Game::craft_available(petal_id) == 0) {
                    holding = 0;
                    break;
                }
            }
        };
    }
    void on_render(Renderer &ctx) override {
        PetalID::T petal_id = current_id();
        if (petal_id != PetalID::kNone)
            CraftFx::remember_mat(petal_id, screen_x, screen_y);
        uint32_t shown = (petal_id == PetalID::kNone) ? 0 : Game::craft_available(petal_id);
        // Unowned rarity columns: blank plate only — never catalog art.
        if (petal_id == PetalID::kNone && last_shown_id == PetalID::kNone) {
            ctx.set_fill(Chrome::kPanelInset);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 6);
            ctx.fill();
            return;
        }
        // Empty / all-in-tray: plain plate (never "x0"); fade icon out via empty_anim.
        if (petal_id == PetalID::kNone || shown == 0 || empty_anim < 0.98f) {
            ctx.set_fill(Chrome::kPanelInset);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 6);
            ctx.fill();
        }
        PetalID::T draw_id = (shown > 0 && petal_id != PetalID::kNone) ? petal_id : last_shown_id;
        if (draw_id == PetalID::kNone || empty_anim <= 0.02f) return;
        uint8_t selected = (Game::craft_tray_id == petal_id && shown > 0);
        {
            RenderContext guard(&ctx);
            ctx.set_global_alpha(empty_anim);
            ctx.scale(0.9f * (0.6f + 0.4f * empty_anim));
            // Name drawn once below — skip built-in label to avoid duplicates.
            draw_loadout_background(ctx, draw_id, 1.f, 0);
        }
        if (selected) {
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, 10);
            ctx.stroke();
        }
        if (shown > 0 && empty_anim > 0.5f) {
            RenderContext t(&ctx);
            std::string qty = format_stack(shown);
            ctx.set_global_alpha(empty_anim);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(14);
            ctx.set_line_width(1.4f);
            ctx.translate(-width / 2 + 3, -height / 2 + 12);
            ctx.stroke_text(qty.c_str());
            ctx.fill_text(qty.c_str());
        }
        if (empty_anim > 0.5f) {
            RenderContext t(&ctx);
            ctx.set_global_alpha(empty_anim);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff111111);
            ctx.set_text_size(12);
            ctx.set_line_width(1.2f);
            ctx.center_text_align();
            ctx.center_text_baseline();
            ctx.translate(0, height / 2 - 10);
            char const *nm = I18n::petal_name(draw_id);
            ctx.stroke_text(nm);
            ctx.fill_text(nm);
        }
    }
    void on_event(uint8_t event) override {
        PetalID::T petal_id = current_id();
        if (petal_id == PetalID::kNone) return;
        if (event == kMouseDown) {
            if (Game::craft_available(petal_id) == 0) {
                Game::craft_status = I18n::use_chinese
                    ? "已全部放上托盘" : "All copies are on the tray";
                return;
            }
            if (!Craft::can_upgrade_petal(petal_id)) {
                Game::craft_status = I18n::use_chinese
                    ? "已达最高稀有度（独特）" : "Already at max rarity (Unique)";
                return;
            }
            CraftFx::remember_mat(petal_id, screen_x, screen_y);
            if (shift_alt_held()) {
                craft_tray_add_animated(petal_id, Game::craft_available(petal_id),
                                        screen_x, screen_y);
                holding = 0;
            } else {
                craft_tray_add_animated(petal_id, Craft::kCraftBatch, screen_x, screen_y);
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
    // Cells resolve the owned id at draw time (name collisions like Dahlia/RoseRare).
    std::vector<char const *> family_names;
    for (uint32_t raw = 1; raw < (uint32_t) PetalID::kNumPetals; ++raw) {
        PetalID::T id = (PetalID::T) raw;
        if (Recipe::is_bag_only(id)) continue;
        char const *name = PETAL_DATA[id].name;
        uint8_t r = PETAL_DATA[id].rarity;
        if (name == nullptr || r >= RarityID::kNumRarities) continue;
        uint8_t seen = 0;
        for (char const *n : family_names) {
            if (std::strcmp(n, name) == 0) { seen = 1; break; }
        }
        if (!seen) family_names.push_back(name);
    }
    std::stable_sort(family_names.begin(), family_names.end(), [](char const *a, char const *b){
        return std::strcmp(a, b) < 0;
    });
    std::vector<Element *> vrows;
    for (char const *name : family_names) {
        std::vector<Element *> cells;
        for (uint8_t r = 0; r < RarityID::kNumRarities; ++r)
            cells.push_back(new CraftMatCell(name, r));
        // Spacer keeps column alignment with older layout.
        cells.push_back(new CraftMatCell(nullptr, 0));
        // Always list every family × rarity column — empty plates for unowned
        // slots so the catalog is never blank.
        vrows.push_back(new Ui::HContainer(cells, 0, 4, {
            .h_justify = Style::Left,
            .no_animation = 1
        }));
    }
    Element *grid = new Ui::VContainer(vrows, 0, 4, { .h_justify = Style::Left, .no_animation = 1 });
    return new CraftMaterialGrid(grid);
}

Element *Ui::make_game_side_buttons() {
    // Vertical column against left wall (bottom) — shared 24×24 HUD glyphs.
    // Size bumped further on mobile by Button::apply_mobile_touch_size (44→64).
    float const gap = Input::is_mobile ? 10.f : 6.f;
    Element *elt = new Ui::VContainer({
        new Ui::Button(48, 48,
            make_hud_icon(hud_icon_inventory, 28.f),
            on_inventory_btn, inventory_active,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            make_hud_icon(hud_icon_talents, 28.f),
            on_talents_btn, talents_active,
            { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            make_hud_icon(hud_icon_craft, 28.f),
            on_craft_btn, craft_active,
            { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            make_hud_icon(hud_icon_special_craft, 28.f),
            on_special_btn, special_active,
            { .fill = Chrome::kBtnClay, .line_width = 0, .round_radius = Chrome::kRadius }),
        new Ui::Button(48, 48,
            make_hud_icon(hud_icon_shop, 28.f),
            on_shop_btn, shop_active,
            { .fill = Chrome::kBtnBronze, .line_width = 0, .round_radius = Chrome::kRadius })
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
        new Ui::TextInput(bag_search_text, Input::is_mobile ? 420.f : 540.f, 42, 48, {
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
            elt->width = (Input::is_mobile ? kBagPanelWMobile : kBagPanelW) + 32.f;
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
        new Ui::Element(Input::is_mobile ? 420.f : 480.f, 1),
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
                ? std::string{"手机 R：长按后点槽切 L 背包（当前→L1，其它首次空）"}
                : std::string{"Mobile R: hold then tap L bag (current→L1, others start empty)"};
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
                return I18n::use_chinese ? std::string{"登录账号后跨设备同步配装（天赋全账号共用）"}
                    : std::string{"Sign in to sync petal presets (talents are account-wide)"};
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

static char const *talent_name_zh(uint8_t id) {
    switch (id) {
        case TalentID::kHealth: return "体魄";
        case TalentID::kRotSpeed: return "转速";
        case TalentID::kAttackRange: return "射程";
        case TalentID::kCooldown: return "冷却";
        case TalentID::kMagnet: return "磁铁";
        case TalentID::kPetalArmor: return "耐久";
        case TalentID::kSlots: return "槽位";
        default: return "?";
    }
}
static char const *talent_name_en(uint8_t id) {
    switch (id) {
        case TalentID::kHealth: return "Vitality";
        case TalentID::kRotSpeed: return "Spin";
        case TalentID::kAttackRange: return "Range";
        case TalentID::kCooldown: return "CDR";
        case TalentID::kMagnet: return "Magnet";
        case TalentID::kPetalArmor: return "Armor";
        case TalentID::kSlots: return "Slots";
        default: return "?";
    }
}
static std::string talent_effect_text(uint8_t id, uint8_t rank) {
    switch (id) {
        case TalentID::kHealth:
            return std::format("+{:.0f} HP", Talent::health_bonus(rank));
        case TalentID::kRotSpeed:
            return std::format("+{:.1f} rad/s", Talent::rot_bonus(rank));
        case TalentID::kAttackRange:
            return std::format("+{:.0f} range", Talent::range_bonus(rank));
        case TalentID::kCooldown:
            return std::format("x{:.2f} CD", Talent::reload_mult(rank));
        case TalentID::kMagnet:
            return std::format("+{:.0f} pull", Talent::magnet_bonus(rank));
        case TalentID::kPetalArmor:
            return std::format("x{:.2f} petal HP", Talent::petal_health_mult(rank));
        case TalentID::kSlots:
            return std::format("{} slots", (int) Talent::slot_count(rank));
        default:
            return {};
    }
}

// ---- Talent panel: first click selects node; second click on same node upgrades ----
namespace TalentUi {
    constexpr uint32_t kCard       = 0xff9B7548;
    constexpr uint32_t kCardEdge   = 0xff765333;
    constexpr uint32_t kBranch     = 0xff684E35;
    constexpr uint32_t kActive     = 0xff72C65A;
    constexpr uint32_t kActiveStroke = 0xff3D8538;
    constexpr uint32_t kActiveHi   = 0xffA8E68A;
    constexpr uint32_t kTipRing    = 0xffF2D77C;
    constexpr uint32_t kLocked     = 0xff49463D;
    constexpr uint32_t kNextFill   = 0xffE8D48A;
    constexpr uint32_t kNextStroke = 0xffC4A84A;
    constexpr uint32_t kText       = 0xffFFF4DC;
    constexpr uint32_t kCapsule    = 0xcc5C4030;
    constexpr uint32_t kBtnReset   = 0xffE87973;
    constexpr float kNodeHit = 42.f;
    constexpr float kStemW = 12.f;
    constexpr float kBranchH = 68.f;
}

static uint8_t g_tal_sel_id = 0xFF;   // TalentID or 0xFF = none
static uint8_t g_tal_sel_rank = 0;    // 1..N when selected
static uint8_t g_tal_reset_ask = 0;
static double g_tal_fx_until = 0;
static uint8_t g_tal_fx_id = 0xFF;
static uint8_t g_tal_fx_rank = 0;

static void talent_select(uint8_t id, uint8_t rank) {
    g_tal_sel_id = id;
    g_tal_sel_rank = rank;
    g_tal_reset_ask = 0;
}

static char const *talent_name(uint8_t id) {
    return I18n::use_chinese ? talent_name_zh(id) : talent_name_en(id);
}

static uint8_t talent_is_slots(uint8_t id) { return id == TalentID::kSlots; }

static uint32_t talent_active_fill(uint8_t id, uint8_t rank) {
    if (talent_is_slots(id))
        return (RARITY_COLORS[Talent::rank_grade(rank) % RarityID::kNumRarities] & 0x00ffffff) | 0xff000000;
    static constexpr uint32_t colors[] = {
        0xffFF7894, 0xff75B7FF, 0xffFFBB5C,
        0xffA893F5, 0xff66D6AE, 0xff72CFE2
    };
    return colors[id];
}

static uint32_t talent_active_stroke(uint8_t id, uint8_t rank) {
    if (talent_is_slots(id))
        return RARITY_COLORS[Talent::rank_grade(rank) % RarityID::kNumRarities] | 0xff000000;
    return Renderer::HSV(talent_active_fill(id, rank), 0.68f);
}

static uint32_t talent_card_fill(uint8_t id) {
    static constexpr uint32_t colors[] = {
        0xff8D6074, 0xff5E7898, 0xff95744F,
        0xff74668F, 0xff517E72, 0xff587F87, 0xff8C704A
    };
    return colors[id];
}

// Second click applies selection:
//   rank > cur  → jump forward (spend cost_to_reach)
//   rank < cur  → jump back to that node (refund the difference)
//   rank == cur → refund one step (unlight the tip → cur-1)
static uint8_t talent_apply_target(uint8_t id, uint8_t rank) {
    if (id >= TalentID::kNum) return 0xFF;
    uint8_t cur = Game::talent_ranks[id];
    if (rank > cur) return rank;
    if (rank < cur) return rank;
    // tip re-click → step back one
    return cur > 0 ? (uint8_t) (cur - 1) : 0;
}

static bool talent_can_apply_to(uint8_t id, uint8_t rank) {
    if (id >= TalentID::kNum) return false;
    if (Game::talent_pending) return false;
    uint8_t target = talent_apply_target(id, rank);
    if (target == 0xFF) return false;
    uint8_t cur = Game::talent_ranks[id];
    if (target == cur) return false;
    uint32_t level = talent_player_level();
    return Talent::can_set_rank(id, target, level, Game::talent_ranks.data()) != 0;
}

static void talent_try_apply_selected() {
    uint8_t id = g_tal_sel_id;
    uint8_t sel = g_tal_sel_rank;
    if (id >= TalentID::kNum) return;
    uint8_t cur = Game::talent_ranks[id];
    uint8_t target = talent_apply_target(id, sel);
    uint32_t level = talent_player_level();
    if (!talent_can_apply_to(id, sel)) {
        if (Game::talent_pending) {
            Game::talent_status = I18n::use_chinese
                ? "天赋同步中…" : "Syncing talents…";
        } else if (target > cur
            && target > Talent::max_rank_for(id, level)) {
            Game::talent_status = I18n::use_chinese
                ? "当前等级尚未解锁该阶" : "Rank locked at this level";
        } else if (target > cur) {
            uint32_t need = Talent::cost_to_reach(cur, target);
            uint32_t have = Talent::points_free(level, Game::talent_ranks);
            Game::talent_status = I18n::use_chinese
                ? std::format("天赋点不足（需 {}，现有 {}）", need, have)
                : std::format("Not enough points (need {}, have {})", need, have);
        } else {
            Game::talent_status = I18n::use_chinese
                ? "无法退回" : "Cannot refund";
        }
        return;
    }
    Game::talent_set_rank(id, target);
    g_tal_fx_id = id;
    g_tal_fx_rank = target > 0 ? target : 1;
    g_tal_fx_until = Game::timestamp + 160.0;
    // Keep selection on the new tip / chosen node
    g_tal_sel_rank = target;
    Game::talent_status.clear();
}

static void on_talent_reset_ask(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    g_tal_reset_ask = 1;
}

static void on_talent_reset_cancel(Element *, uint8_t e) {
    if (e == Ui::kClick) g_tal_reset_ask = 0;
}

static void on_talent_reset_confirm(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    g_tal_reset_ask = 0;
    g_tal_sel_id = 0xFF;
    g_tal_sel_rank = 0;
    Game::talent_refund(0xFF);
}

class TalentInfoCapsule final : public Element {
    uint8_t kind; // 0=level 1=points 2=slots
public:
    explicit TalentInfoCapsule(uint8_t k)
        : Element(Input::is_mobile ? 108.f : 118.f, 34,
                  { .no_animation = 1 }), kind(k) {}
    void on_render(Renderer &ctx) override {
        uint32_t lv = talent_player_level();
        uint32_t free = Talent::points_free(lv, Game::talent_ranks);
        int slots = (int) Talent::slot_count_from_ranks(Game::talent_ranks);
        uint32_t face = TalentUi::kCapsule;
        uint32_t ink = TalentUi::kText;
        if (kind == 1) ink = Chrome::kGold;
        Chrome::draw_plate(ctx, width, height, face, 12.f, 2.5f);
        std::string label;
        if (kind == 0)
            label = I18n::use_chinese ? std::format("等级 {}", lv) : std::format("Lv {}", lv);
        else if (kind == 1)
            label = I18n::use_chinese ? std::format("可用点数 {}", free) : std::format("{} pts", free);
        else
            label = I18n::use_chinese ? std::format("槽位 {}", slots) : std::format("{} slots", slots);
        ctx.set_text_size(13.f);
        ctx.draw_text(label.c_str(), {
            .fill = ink, .size = 13.f, .stroke_scale = 0.1f
        });
    }
};

class TalentStem final : public Element {
    uint8_t talent_id;
    uint8_t after_rank;
public:
    TalentStem(uint8_t id, uint8_t after)
        : Element(TalentUi::kStemW, TalentUi::kBranchH, { .no_animation = 1 }),
          talent_id(id), after_rank(after) {}
    void on_render(Renderer &ctx) override {
        uint8_t cur = Game::talent_ranks[talent_id];
        uint8_t lit = cur > after_rank;
        uint32_t col = lit
            ? (talent_is_slots(talent_id)
                ? (RARITY_COLORS[Talent::rank_grade((uint8_t) (after_rank + 1)) % RarityID::kNumRarities] | 0xff000000)
                : talent_active_fill(talent_id, (uint8_t) (after_rank + 1)))
            : TalentUi::kBranch;
        ctx.round_line_cap();
        ctx.set_stroke(col);
        ctx.set_line_width(lit ? 5.f : 4.f);
        ctx.begin_path();
        ctx.move_to(-width / 2 + 1.f, 0);
        ctx.line_to(width / 2 - 1.f, 0);
        ctx.stroke();
    }
};

// Root node — 1st click selects; 2nd click refunds the whole branch to Lv0.
class TalentRootNode final : public Element {
    uint8_t talent_id;
public:
    explicit TalentRootNode(uint8_t id)
        : Element(TalentUi::kNodeHit, TalentUi::kBranchH, { .no_animation = 1 }),
          talent_id(id) {}
    void on_render(Renderer &ctx) override {
        uint8_t cur = Game::talent_ranks[talent_id];
        uint8_t selected = g_tal_sel_id == talent_id && g_tal_sel_rank == 0;
        float r = 11.f;
        uint32_t fill = cur > 0 ? talent_active_fill(talent_id, 1) : 0xffC9B896;
        uint32_t stroke = cur > 0 ? talent_active_stroke(talent_id, 1) : TalentUi::kTipRing;
        ctx.set_fill(fill);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.fill();
        ctx.set_stroke(stroke);
        ctx.set_line_width(2.4f);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.stroke();
        if (selected) {
            ctx.set_stroke(TalentUi::kTipRing);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 4.f);
            ctx.stroke();
        } else if (focus_state == kMouseHover || focus_state == kMouseDown) {
            ctx.set_stroke(0x66ffffff);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 3.5f);
            ctx.stroke();
        }
    }
    void on_event(uint8_t e) override {
        if (e != Ui::kClick) return;
        uint8_t already = g_tal_sel_id == talent_id && g_tal_sel_rank == 0;
        if (already) {
            talent_try_apply_selected();
            return;
        }
        talent_select(talent_id, 0);
        uint8_t cur = Game::talent_ranks[talent_id];
        Game::talent_status = cur > 0
            ? (I18n::use_chinese
                ? std::format("再次点击退回全部（返还 {} 点）",
                    Talent::cost_to_reach(0, cur))
                : std::format("Click again to clear (refund {} pts)",
                    Talent::cost_to_reach(0, cur)))
            : std::string{};
    }
};

// Rank node — 1st click selects; 2nd click: forward = upgrade, back = refund.
class TalentRankNode final : public Element {
    uint8_t talent_id;
    uint8_t rank;
public:
    TalentRankNode(uint8_t id, uint8_t r)
        : Element(TalentUi::kNodeHit, Input::is_mobile ? 46.f : TalentUi::kBranchH,
                  { .no_animation = 1 }),
          talent_id(id), rank(r) {}
    void on_render(Renderer &ctx) override {
        uint32_t level = talent_player_level();
        uint8_t cur = Game::talent_ranks[talent_id];
        uint8_t max_r = Talent::max_rank_for(talent_id, level);
        uint8_t locked = rank > max_r;
        uint8_t owned = cur >= rank;
        uint8_t tip = cur == rank;
        // Any higher unlocked rank is a jump target (not only cur+1)
        uint8_t jump = !owned && !locked && rank > cur;
        uint8_t jump_ok = jump
            && Talent::can_set_rank(talent_id, rank, level, Game::talent_ranks.data());
        uint8_t next = jump && rank == (uint8_t) (cur + 1);
        uint8_t selected = g_tal_sel_id == talent_id && g_tal_sel_rank == rank;
        uint8_t pulsing = g_tal_fx_id == talent_id && g_tal_fx_rank == rank
            && Game::timestamp < g_tal_fx_until;

        float r = tip ? 19.f : (jump_ok ? 17.f : 16.f);
        if (pulsing) {
            float t = (float) ((g_tal_fx_until - Game::timestamp) / 160.0);
            r *= 1.f + 0.12f * t;
        }
        if ((focus_state == kMouseHover || focus_state == kMouseDown) && !locked)
            r *= 1.06f;

        uint32_t fill;
        uint32_t stroke;
        float stroke_w;
        if (locked) {
            fill = 0xff65564E;
            stroke = 0xff2E2C28;
            stroke_w = 2.f;
        } else if (owned) {
            fill = talent_active_fill(talent_id, rank);
            stroke = tip ? TalentUi::kTipRing : talent_active_stroke(talent_id, rank);
            stroke_w = tip ? 3.2f : 2.6f;
        } else if (jump) {
            fill = jump_ok ? 0xffFFF0B5 : 0xff9B846A;
            stroke = jump_ok ? talent_active_fill(talent_id, rank) : 0xff6D5B4C;
            stroke_w = next ? 3.f : 2.6f;
        } else {
            fill = 0xff715D4D;
            stroke = TalentUi::kBranch;
            stroke_w = 2.f;
        }

        ctx.set_fill(fill);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.fill();
        if (!locked && owned && !talent_is_slots(talent_id)) {
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.arc(-r * 0.28f, -r * 0.30f, r * 0.38f);
            ctx.fill();
        }
        ctx.set_stroke(stroke);
        ctx.set_line_width(stroke_w);
        ctx.begin_path();
        ctx.arc(0, 0, r);
        ctx.stroke();
        if (jump && jump_ok) {
            ctx.set_stroke(TalentUi::kTipRing);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 3.f);
            ctx.stroke();
            if (next) {
                RenderContext g(&ctx);
                ctx.translate(r * 0.55f, -r * 0.75f);
                ctx.set_fill(talent_active_fill(talent_id, rank));
                ctx.begin_path();
                ctx.arc(0, 0, 7.f);
                ctx.fill();
                ctx.set_text_size(11.f);
                ctx.draw_text("+", {
                    .fill = 0xffffffff, .size = 11.f, .stroke_scale = 0.08f
                });
            }
            char buf[4];
            snprintf(buf, sizeof(buf), "%u", (unsigned) rank);
            ctx.set_text_size(13.f);
            ctx.draw_text(buf, {
                .fill = 0xff3A2E18, .size = 13.f, .stroke_scale = 0.1f
            });
        } else if (!locked) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%u", (unsigned) rank);
            ctx.set_text_size(13.f);
            ctx.draw_text(buf, {
                .fill = owned ? 0xffffffff : (jump ? 0xffFFF4DC : 0xff8A8070),
                .size = 13.f,
                .stroke_scale = 0.12f
            });
        }
        if (selected) {
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 5.f);
            ctx.stroke();
            ctx.set_stroke(TalentUi::kTipRing);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 7.5f);
            ctx.stroke();
        } else if (focus_state == kMouseHover || focus_state == kMouseDown) {
            ctx.set_stroke(0x55ffffff);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.arc(0, 0, r + 4.f);
            ctx.stroke();
        }
        if (focus_state == kMouseHover && !Input::is_mobile) {
            RenderContext tip_g(&ctx);
            ctx.translate(0, -r - 14.f);
            std::string tip_s;
            if (locked)
                tip_s = I18n::use_chinese ? "等级不足" : "Level locked";
            else if (owned) {
                if (rank < cur) {
                    uint32_t back = Talent::cost_to_reach(rank, cur);
                    tip_s = I18n::use_chinese
                        ? std::format("再点退回到此（还 {} 点）", back)
                        : std::format("Click again to refund {} pts", back);
                } else if (rank == cur && cur > 0) {
                    tip_s = I18n::use_chinese
                        ? std::format("再点退回一级（还 {} 点）", Talent::rank_cost(cur))
                        : std::format("Click again −1 (refund {} pts)", Talent::rank_cost(cur));
                } else
                    tip_s = talent_effect_text(talent_id, rank);
            } else if (jump) {
                uint32_t cost = Talent::cost_to_reach(cur, rank);
                tip_s = I18n::use_chinese
                    ? std::format("再点升到此阶（花 {} 点）", cost)
                    : std::format("Click again (spend {} pts)", cost);
            } else
                tip_s = I18n::use_chinese ? "需先点亮前置" : "Need prior rank";
            ctx.set_text_size(11.f);
            ctx.draw_text(tip_s.c_str(), {
                .fill = TalentUi::kText, .size = 11.f, .stroke_scale = 0.1f
            });
        }
    }
    void on_event(uint8_t e) override {
        if (e != Ui::kClick) return;
        uint8_t already = g_tal_sel_id == talent_id && g_tal_sel_rank == rank;
        if (already) {
            talent_try_apply_selected();
            return;
        }
        talent_select(talent_id, rank);
        uint32_t level = talent_player_level();
        uint8_t cur = Game::talent_ranks[talent_id];
        uint8_t target = talent_apply_target(talent_id, rank);
        if (rank > Talent::max_rank_for(talent_id, level) && rank > cur) {
            Game::talent_status = I18n::use_chinese
                ? "当前等级尚未解锁该阶" : "Rank locked at this level";
        } else if (target > cur) {
            uint32_t cost = Talent::cost_to_reach(cur, target);
            if (talent_can_apply_to(talent_id, rank)) {
                Game::talent_status = I18n::use_chinese
                    ? std::format("再次点击升到第{}级（花费 {} 点）", target, cost)
                    : std::format("Click again → rank {} (spend {} pts)", target, cost);
            } else {
                uint32_t have = Talent::points_free(level, Game::talent_ranks);
                Game::talent_status = I18n::use_chinese
                    ? std::format("升到第{}级需 {} 点（现有 {}）", target, cost, have)
                    : std::format("Rank {} needs {} pts (have {})", target, cost, have);
            }
        } else if (target < cur) {
            uint32_t back = Talent::cost_to_reach(target, cur);
            Game::talent_status = I18n::use_chinese
                ? std::format("再次点击退回到第{}级（返还 {} 点）", target, back)
                : std::format("Click again → rank {} (refund {} pts)", target, back);
        } else {
            Game::talent_status.clear();
        }
    }
};

class TalentBranchLabel final : public Element {
    uint8_t talent_id;
public:
    explicit TalentBranchLabel(uint8_t id)
        : Element(Input::is_mobile ? 260.f : 86.f,
                  Input::is_mobile ? 38.f : TalentUi::kBranchH,
                  { .no_animation = 1 }), talent_id(id) {}
    void on_render(Renderer &ctx) override {
        uint8_t cur = Game::talent_ranks[talent_id];
        if (Input::is_mobile) ctx.translate(-width * 0.5f + 36.f, -3.f);
        ctx.set_text_size(15.f);
        ctx.draw_text(talent_name(talent_id), {
            .fill = talent_active_fill(talent_id, 1), .size = 15.f, .stroke_scale = 0.12f
        });
        {
            RenderContext g(&ctx);
            ctx.translate(0, 16.f);
            std::string sub = cur > 0
                ? (I18n::use_chinese
                    ? std::format("Lv{} · {}", cur, talent_effect_text(talent_id, cur))
                    : std::format("Lv{} · {}", cur, talent_effect_text(talent_id, cur)))
                : (I18n::use_chinese ? std::string{"未加点"} : std::string{"Empty"});
            ctx.set_text_size(11.f);
            ctx.draw_text(sub.c_str(), {
                .fill = 0xffE8D4B0, .size = 11.f, .stroke_scale = 0.08f
            });
        }
    }
};

static Element *make_talent_branch(uint8_t id) {
    uint8_t max_nodes = id == TalentID::kSlots
        ? Talent::kMaxSlotRanks : Talent::kMaxGradeRank;
    if (Input::is_mobile) {
        std::vector<Element *> first, second;
        for (uint8_t r = 1; r <= max_nodes; ++r) {
            if (r <= 5) first.push_back(new TalentRankNode(id, r));
            else second.push_back(new TalentRankNode(id, r));
        }
        std::vector<Element *> contents = {
            new TalentBranchLabel(id),
            new Ui::HContainer(first, 0, 10, { .no_animation = 1 })
        };
        if (!second.empty())
            contents.push_back(new Ui::HContainer(second, 0, 10, { .no_animation = 1 }));
        return new Ui::VContainer(contents, 8, 0, {
            .fill = talent_card_fill(id),
            .round_radius = 16,
            .no_animation = 1
        });
    }
    std::vector<Element *> row;
    row.push_back(new TalentBranchLabel(id));
    row.push_back(new TalentRootNode(id));
    for (uint8_t r = 1; r <= max_nodes; ++r) {
        row.push_back(new TalentStem(id, (uint8_t) (r - 1)));
        row.push_back(new TalentRankNode(id, r));
    }
    return new Ui::HContainer(row, 8, 3, {
        .fill = talent_card_fill(id),
        .line_width = 0,
        .round_radius = 16,
        .no_animation = 1
    });
}

class TalentTreeScroll final : public Ui::ScrollContainer {
public:
    explicit TalentTreeScroll(Element *content) : Ui::ScrollContainer(content, 490.f) {
        always_capture_wheel = 1;
    }
    void refactor() override {
        float view_h = (float) (Ui::window_height / Ui::scale);
        height = std::clamp(view_h - (Input::is_mobile ? 330.f : 340.f), 220.f, 510.f);
        Ui::ScrollContainer::refactor();
    }
};

class TalentActionSummary final : public Element {
public:
    TalentActionSummary()
        : Element(Input::is_mobile ? 340.f : 420.f, 36,
                  { .no_animation = 1 }) {}
    void refactor() override {
        // Fixed content width — never follow parent (VContainer ↔ child loop).
        width = Input::is_mobile ? 340.f : 520.f;
    }
    void on_render(Renderer &ctx) override {
        std::string line;
        if (Input::is_mobile) {
            if (g_tal_sel_id >= TalentID::kNum)
                line = I18n::use_chinese
                    ? "点选→再点：往后升 / 往前退" : "Select→tap: forward / back";
            else {
                uint8_t cur = Game::talent_ranks[g_tal_sel_id];
                uint8_t target = talent_apply_target(g_tal_sel_id, g_tal_sel_rank);
                if (target > cur) {
                    uint32_t cost = Talent::cost_to_reach(cur, target);
                    line = I18n::use_chinese
                        ? std::format("{} · {}→{} · 花{}点 · 再点升",
                            talent_name(g_tal_sel_id), cur, target, cost)
                        : std::format("{} · {}→{} · {}pts · tap up",
                            talent_name(g_tal_sel_id), cur, target, cost);
                } else if (target < cur) {
                    uint32_t back = Talent::cost_to_reach(target, cur);
                    line = I18n::use_chinese
                        ? std::format("{} · {}→{} · 还{}点 · 再点退",
                            talent_name(g_tal_sel_id), cur, target, back)
                        : std::format("{} · {}→{} · +{}pts · tap back",
                            talent_name(g_tal_sel_id), cur, target, back);
                } else {
                    line = std::format("{}  Lv{}", talent_name(g_tal_sel_id), cur);
                }
            }
        } else if (g_tal_sel_id >= TalentID::kNum) {
            line = I18n::use_chinese
                ? "第一次选中；再点：往后升级，往前退回"
                : "Select once; again: forward upgrades, back refunds";
        } else {
            uint8_t cur = Game::talent_ranks[g_tal_sel_id];
            uint8_t target = talent_apply_target(g_tal_sel_id, g_tal_sel_rank);
            uint32_t level = talent_player_level();
            if (target > cur
                && g_tal_sel_rank > Talent::max_rank_for(g_tal_sel_id, level)) {
                line = I18n::use_chinese
                    ? std::format("选中：{} · 第{}级（未解锁）",
                        talent_name(g_tal_sel_id), g_tal_sel_rank)
                    : std::format("Selected: {} · rank {} (locked)",
                        talent_name(g_tal_sel_id), g_tal_sel_rank);
            } else if (target > cur) {
                uint32_t cost = Talent::cost_to_reach(cur, target);
                uint8_t ok = Talent::can_set_rank(
                    g_tal_sel_id, target, level, Game::talent_ranks.data());
                line = I18n::use_chinese
                    ? (ok
                        ? std::format("选中：{} · {} → {}（再点升级，花费 {} 点）",
                            talent_name(g_tal_sel_id), cur, target, cost)
                        : std::format("选中：{} · {} → {}（需 {} 点，不足）",
                            talent_name(g_tal_sel_id), cur, target, cost))
                    : (ok
                        ? std::format("Selected: {} · {} → {} (click to spend {} pts)",
                            talent_name(g_tal_sel_id), cur, target, cost)
                        : std::format("Selected: {} · {} → {} (need {} pts)",
                            talent_name(g_tal_sel_id), cur, target, cost));
            } else if (target < cur) {
                uint32_t back = Talent::cost_to_reach(target, cur);
                line = I18n::use_chinese
                    ? std::format("选中：{} · {} → {}（再点退回，返还 {} 点）",
                        talent_name(g_tal_sel_id), cur, target, back)
                    : std::format("Selected: {} · {} → {} (click to refund {} pts)",
                        talent_name(g_tal_sel_id), cur, target, back);
            } else {
                line = I18n::use_chinese
                    ? std::format("选中：{} · 当前 Lv{}", talent_name(g_tal_sel_id), cur)
                    : std::format("Selected: {} · Lv{}", talent_name(g_tal_sel_id), cur);
            }
        }
        ctx.set_text_size(13.f);
        ctx.draw_text(line.c_str(), {
            .fill = TalentUi::kText, .size = 13.f, .stroke_scale = 0.1f
        });
    }
};

Element *Ui::make_talents_panel() {
    std::vector<Element *> attr_branches;
    for (uint8_t id = 0; id < TalentID::kSlots; ++id)
        attr_branches.push_back(make_talent_branch(id));
    Element *tree = new TalentTreeScroll(new Ui::VContainer({
        new Ui::VContainer(attr_branches, 0, 6, { .no_animation = 1 }),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese ? std::string{"✦ 槽位分支"} : std::string{"✦ Slot branch"};
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        make_talent_branch(TalentID::kSlots)
    }, 0, 8, { .h_justify = Style::Left, .no_animation = 1 }));

    Element *normal_actions = new Ui::HContainer({
        new Ui::Button(108, 34,
            new Ui::DynamicText(13, [](){
                return I18n::use_chinese ? std::string{"重置全部"} : std::string{"Reset all"};
            }),
            on_talent_reset_ask, nullptr,
            { .fill = TalentUi::kBtnReset, .line_width = 0, .round_radius = 9 }),
    }, 0, 10, {
        .should_render = [](){ return !g_tal_reset_ask; },
        .no_animation = 1
    });

    Element *confirm_label = new Ui::DynamicText(13, [](){
        if (Input::is_mobile)
            return I18n::use_chinese ? std::string{"重置全部天赋并返还点数？"}
                : std::string{"Reset and refund all points?"};
        return I18n::use_chinese
            ? std::string{"重置全部天赋？将按规则返还已分配点数。"}
            : std::string{"Reset all talents? Points refunded per rules."};
    }, { .fill = TalentUi::kText });
    Element *confirm_buttons = new Ui::HContainer({
        new Ui::Button(88, 34,
            new Ui::DynamicText(13, [](){
                return I18n::use_chinese ? std::string{"取消"} : std::string{"Cancel"};
            }),
            on_talent_reset_cancel, nullptr,
            { .fill = Chrome::kBtnSlate, .line_width = 0, .round_radius = 9 }),
        new Ui::Button(108, 34,
            new Ui::DynamicText(13, [](){
                return I18n::use_chinese ? std::string{"确认重置"} : std::string{"Confirm"};
            }),
            on_talent_reset_confirm, nullptr,
            { .fill = TalentUi::kBtnReset, .line_width = 0, .round_radius = 9 }),
    }, 0, 10, { .no_animation = 1 });
    Element *confirm_actions = new Ui::VContainer({
        confirm_label, confirm_buttons
    }, 0, 4, {
        .should_render = [](){ return g_tal_reset_ask != 0; },
        .no_animation = 1
    });

    Element *panel = make_drawer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(28, [](){ return I18n::tr(I18n::kPanelTalents); }, {
                .fill = Chrome::kTitle
            }),
            new Ui::Button(36, 36,
                new Ui::DynamicText(20, [](){ return std::string{"×"}; }),
                on_close_inventory, nullptr,
                { .fill = Chrome::kBtnDanger, .round_radius = 12 }),
            0, 8, { .no_animation = 1 }),
        new Ui::HContainer({
            new TalentInfoCapsule(0),
            new TalentInfoCapsule(1),
            new TalentInfoCapsule(2),
        }, 0, 8, { .h_justify = Style::Left, .no_animation = 1 }),
        new Ui::DynamicText(12, [](){
            if (Input::is_mobile)
                return I18n::use_chinese
                    ? std::string{"点选→再点：往后升级，往前退回（点数按中间各级加减）"}
                    : std::string{"Select→tap: forward upgrades, back refunds"};
            return I18n::use_chinese
                ? std::string{"第一次点选中；再点同节点：往后升级、往前退回。点数按点亮/熄灭的节点累计加减。"}
                : std::string{"Click to select; click again: forward = upgrade, back = refund. Points = sum of ranks."};
        }, { .fill = 0xffE8D4B0, .h_justify = Style::Left }),
        tree,
        new TalentActionSummary(),
        normal_actions,
        confirm_actions,
        new Ui::DynamicText(12, [](){
            return I18n::use_chinese
                ? "局内：按住 Q/E 调转速，或拖动槽右侧圆轮"
                : "In-game: hold Q/E or drag the spin wheel";
        }, { .fill = 0xffE8D4B0, .h_justify = Style::Left }),
        new Ui::DynamicText(13, [](){ return Game::talent_status; }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        })
    }, Game::kPanelTalents);
    panel->style.round_radius = 18.f;
    if (Input::is_mobile) {
        panel->x = 76.f;
        panel->style.no_animation = 1;
    }
    return panel;
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
    }, Input::is_mobile ? 480.f : 640.f, 44, { .no_animation = 1 }) {}
    void refactor() override {
        for (Element *child : children) child->refactor();
        width = Input::is_mobile ? 480.f : 640.f;
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
                    return I18n::use_chinese
                        ? "合成为更高稀有度（失败损失1–4）"
                        : "to a higher rarity (miss loses 1–4)";
                }, { .fill = Chrome::kTitle, .h_justify = Style::Left }),
                new Ui::DynamicText(13, [](){
                    if (Game::craft_tray_id == PetalID::kNone) return std::string{};
                    uint8_t r = PETAL_DATA[Game::craft_tray_id].rarity;
                    if (r + 1 >= RarityID::kNumRarities)
                        return I18n::use_chinese ? std::string{"已达最高稀有度"} : std::string{"Max rarity"};
                    float pct = Craft::success_chance(r) * 100.f;
                    // High tiers are <1%; integer % wrongly shows "0%".
                    if (pct > 0.f && pct < 10.f)
                        return std::format("{} {:.1f}%", I18n::rarity_name((uint8_t)(r + 1)), pct);
                    return std::format("{} {:.0f}%", I18n::rarity_name((uint8_t)(r + 1)), pct);
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

class CraftFxLayer final : public Element {
public:
    CraftFxLayer() : Element(0, 0, {
        .animate = [](Element *, Renderer &) {
            CraftFx::tick((float) Ui::dt);
        },
        .should_render = [](){
            return CraftFx::fly_n > 0 || CraftFx::speck_n > 0
                || CraftFx::win_flash > 0.02f
                || (Game::game_panel == Game::kPanelCraft
                    && (Game::alive() || Game::on_game_screen));
        },
        .no_animation = 1,
        .no_polling = 1
    }) {
        style.layer = 1;
    }
    void on_render(Renderer &ctx) override {
        CraftFx::render_overlay(ctx);
    }
};

Element *Ui::make_craft_fx_layer() {
    return new CraftFxLayer();
}

void Ui::craft_play_result(uint8_t result, PetalID::T petal) {
    CraftFx::play_result(result, petal);
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
    BonusToggleButton() : ToggleButton(Input::is_mobile ? 48.f : 44.f, &Game::bonus_enabled) {}
    void on_event(uint8_t event) override {
        ToggleButton::on_event(event);
        if (event == kClick) Game::set_bonus_enabled(Game::bonus_enabled);
    }
};

Element *Ui::make_boost_panel() {
    uint8_t const mobile = Input::is_mobile;
    float const panel_w = mobile ? 360.f : 420.f;
    float const card_pad = mobile ? 16.f : 18.f;
    float const title_sz = mobile ? 26.f : 28.f;
    float const clock_sz = mobile ? 32.f : 36.f;
    float const label_sz = mobile ? 17.f : 18.f;
    float const hint_sz = mobile ? 14.f : 15.f;
    Element *elt = make_drawer({
        new Ui::Element(panel_w, 1),
        new Ui::HFlexContainer(
            new Ui::DynamicText(title_sz, [](){
                return I18n::use_chinese ? "三倍掉落加成" : "Triple drop bonus";
            }, { .fill = Chrome::kTitle }),
            new Ui::Button(mobile ? 40.f : 36.f, mobile ? 40.f : 36.f,
                new Ui::DynamicText(22, [](){ return std::string{"×"}; }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) Game::close_game_panel(); },
                nullptr, { .fill = Chrome::kBtnDanger, .round_radius = 6 }),
            0, 14),
        new Ui::HContainer({
            new BonusToggleButton(),
            new Ui::DynamicText(mobile ? 20.f : 22.f, [](){
                if (!Game::account_logged_in)
                    return I18n::use_chinese ? "登录后可开关" : "Sign in to toggle";
                return Game::bonus_enabled
                    ? (I18n::use_chinese ? "加成已开启" : "Bonus ON")
                    : (I18n::use_chinese ? "加成已关闭" : "Bonus OFF");
            }, { .fill = Chrome::kTitle })
        }, 0, 14, { .h_justify = Style::Left }),
        new Ui::VContainer({
            new Ui::DynamicText(label_sz, [](){ return I18n::use_chinese ? "每日加成" : "Daily bonus"; },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(clock_sz, [](){ return bonus_clock(visible_bonus_seconds(true)); },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(hint_sz, [](){
                return I18n::use_chinese ? "每天 1 小时 · 北京时间零点刷新" : "One hour daily · resets at Beijing midnight";
            }, { .fill = Chrome::kBody, .h_justify = Style::Left })
        }, card_pad, 10, { .fill = 0xff73c77a, .round_radius = 10, .h_justify = Style::Left }),
        new Ui::VContainer({
            new Ui::DynamicText(label_sz, [](){ return I18n::use_chinese ? "离线加成" : "Offline bonus"; },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(clock_sz, [](){ return bonus_clock(visible_bonus_seconds(false)); },
                { .fill = Chrome::kTitle, .h_justify = Style::Left }),
            new Ui::DynamicText(hint_sz, [](){
                return I18n::use_chinese ? "离线 10 分钟积 1 分钟 · 最多存 3 小时" : "10 minutes away earns 1 minute · cap 3 hours";
            }, { .fill = Chrome::kBody, .h_justify = Style::Left })
        }, card_pad, 10, { .fill = 0xff73c77a, .round_radius = 10, .h_justify = Style::Left }),
        new Ui::DynamicText(hint_sz + 1.f, [](){
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
    elt->x = mobile ? 10.f : 12.f;
    elt->y = mobile ? 78.f : 72.f;
    return elt;
}

// Assembly workshop (V): A + B = Result [hammer] — matches craft mockup.
// Tiers: 0 基础 · 1 跨图 · 2 传说 · 3 超级(coin shop)
static uint8_t recipe_filter_tier = 0;
static uint16_t recipe_selected = 0;
static constexpr uint8_t kRecipeTierCount = 4;

static char const *recipe_tier_label() {
    if (I18n::use_chinese) {
        switch (recipe_filter_tier) {
            case 0: return "基础";
            case 1: return "跨图";
            case 2: return "传说";
            case 3: return "超级";
            default: return "基础";
        }
    }
    switch (recipe_filter_tier) {
        case 0: return "Base";
        case 1: return "Cross";
        case 2: return "Legend";
        case 3: return "Super";
        default: return "Base";
    }
}

static uint8_t recipe_can_craft(uint16_t idx) {
    if (idx >= Recipe::recipe_count()) return 0;
    Recipe::Def const &rec = Recipe::recipe_at(idx);
    for (uint8_t i = 0; i < rec.ingredient_count; ++i)
        if (Game::craft_owned(rec.ingredients[i].id) < rec.ingredients[i].count)
            return 0;
    return 1;
}

// Catalog: list every recipe in the tier. Missing mats stay visible (red
// counts); hammer stays disabled until recipe_can_craft.
static uint8_t recipe_visible(uint16_t idx) {
    (void) idx;
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
        uint32_t owned = (petal_id < PetalID::kNumPetals) ? Game::craft_owned(petal_id) : 0;
        uint8_t enough = !show_need || owned >= need;
        uint32_t fill = 0xff4a4a4a; // missing mat — dark grey like mockup
        if (petal_id != PetalID::kNone && petal_id < PetalID::kNumPetals) {
            if (enough) {
                // Light green plate when mats are ready
                fill = show_need ? 0xff7dff9a : RARITY_COLORS[PETAL_DATA[petal_id].rarity];
            }
        }
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
            .fill = 0xffffffff, .size = 12, .stroke_scale = 0.1f
        });
        if (show_need) {
            ctx.translate(0, 16);
            uint32_t col = enough ? 0xff1a7a2a : 0xffff5555;
            ctx.draw_text(std::format("{} / {}", format_stack(need), format_stack(owned)).c_str(), {
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
        : Element(580, 96, { .no_animation = 1 }), recipe_index(idx) {
        style.should_render = [idx](){
            if (Game::game_panel != Game::kPanelSpecial) return false;
            if (!Game::should_render_game_ui()) return false;
            if (idx >= Recipe::recipe_count()) return false;
            if (Recipe::recipe_at(idx).tier != recipe_filter_tier)
                return false;
            return recipe_visible(idx) != 0;
        };
    }
    void refactor() override {
        uint8_t want = style.should_render ? style.should_render() : 1;
        visible = want;
        animation.set(want);
        width = Input::is_mobile ? 440.f : 580.f;
        // Slightly taller when 3+ mats so chip labels + hammer stay clear
        uint8_t n = 0;
        if (recipe_index < Recipe::recipe_count())
            n = Recipe::recipe_at(recipe_index).ingredient_count;
        height = (Input::is_mobile && n >= 3) ? 104.f : 96.f;
    }
    void on_render(Renderer &ctx) override {
        if (recipe_index >= Recipe::recipe_count()) return;
        Recipe::Def const &rec = Recipe::recipe_at(recipe_index);
        uint8_t can = recipe_can_craft(recipe_index);
        uint8_t sel = recipe_selected == recipe_index;
        // Dark row plate; selected = neon green rim (mockup)
        Chrome::draw_plate(ctx, width, height, 0xff2a6a6c, Chrome::kRadius, 0.f);
        if (sel) {
            ctx.set_stroke(0xff7CFF8A);
            ctx.set_line_width(3.5f);
            ctx.begin_path();
            ctx.round_rect(-width / 2, -height / 2, width, height, Chrome::kRadius);
            ctx.stroke();
        }
        // Fit A[+B[+C[+D]]] = Result into the row (was hard-capped at 2 chips,
        // with extras buried in a clipped "Also …" footnote).
        uint8_t n_in = rec.ingredient_count;
        if (n_in > Recipe::kMaxIngredients) n_in = Recipe::kMaxIngredients;
        uint8_t mobile = Input::is_mobile;
        // Leave ~70px on the right for the hammer button.
        float chip_w = mobile ? 52.f : 68.f;
        float chip_h = mobile ? 72.f : 80.f;
        float step = mobile ? 60.f : 78.f;
        float op_pad = mobile ? 14.f : 18.f;
        if (n_in >= 4) {
            chip_w = mobile ? 36.f : 50.f;
            chip_h = mobile ? 56.f : 68.f;
            step = mobile ? 40.f : 56.f;
            op_pad = mobile ? 9.f : 12.f;
        } else if (n_in >= 3) {
            chip_w = mobile ? 42.f : 56.f;
            chip_h = mobile ? 62.f : 72.f;
            step = mobile ? 48.f : 64.f;
            op_pad = mobile ? 11.f : 14.f;
        }
        float x = -width / 2 + chip_w * 0.55f + (mobile ? 6.f : 12.f);
        auto draw_op = [&](char const *op) {
            RenderContext c(&ctx);
            ctx.translate(x - 4.f, 0);
            ctx.draw_text(op, {
                .fill = 0xffffffff,
                .size = mobile ? 20.f : 28.f,
                .stroke_scale = 0.08f
            });
            x += op_pad;
        };
        auto draw_chip = [&](PetalID::T id, uint16_t need, uint8_t show_need) {
            RenderContext c(&ctx);
            ctx.translate(x, 0);
            RecipePetalChip chip(id, need, show_need);
            chip.width = chip_w;
            chip.height = chip_h;
            chip.on_render(ctx);
            x += step;
        };
        for (uint8_t i = 0; i < n_in; ++i) {
            if (i > 0) draw_op("+");
            draw_chip(rec.ingredients[i].id, rec.ingredients[i].count, 1);
        }
        draw_op("=");
        draw_chip(rec.result, 1, 0);
        // Combine / craft button (right) — claw-hammer glyph on a warm plate
        {
            RenderContext c(&ctx);
            float bx = width / 2 - 42;
            ctx.translate(bx, 0);
            uint32_t plate = can ? 0xff3d8f78 : 0xff3a4a4c;
            uint32_t plate_hi = can ? 0xff5bb896 : 0xff4a585a;
            Chrome::draw_plate(ctx, 56, 56, plate, Chrome::kRadius, 0.f);
            // Soft top sheen so the hit-target reads as a button
            ctx.set_fill(plate_hi);
            ctx.set_global_alpha(0.35f);
            ctx.begin_path();
            ctx.round_rect(-24, -24, 48, 18, 8);
            ctx.fill();
            ctx.set_global_alpha(1.f);

            uint32_t metal = can ? 0xfffff0c8 : 0xff9aa3a4;
            uint32_t metal_dk = can ? 0xffd4a84a : 0xff6a7274;
            uint32_t wood = can ? 0xffc9894a : 0xff6a5a4a;
            uint32_t wood_dk = can ? 0xff8a5a28 : 0xff4a3a2a;

            {
                RenderContext hammer(&ctx);
                ctx.rotate(-0.55f);
                // Handle (wood)
                ctx.set_fill(wood_dk);
                ctx.begin_path();
                ctx.round_rect(-3.5f, -2.f, 7.f, 22.f, 2.5f);
                ctx.fill();
                ctx.set_fill(wood);
                ctx.begin_path();
                ctx.round_rect(-2.6f, -1.f, 5.2f, 19.f, 2.f);
                ctx.fill();
                // Head block
                ctx.set_fill(metal_dk);
                ctx.begin_path();
                ctx.round_rect(-14.f, -16.f, 28.f, 12.f, 3.f);
                ctx.fill();
                ctx.set_fill(metal);
                ctx.begin_path();
                ctx.round_rect(-12.5f, -14.5f, 25.f, 9.f, 2.5f);
                ctx.fill();
                // Claw notch (readable “hammer”, not a random diamond)
                ctx.set_fill(plate);
                ctx.begin_path();
                ctx.move_to(-14.f, -10.f);
                ctx.line_to(-7.f, -10.f);
                ctx.line_to(-14.f, -4.f);
                ctx.close_path();
                ctx.fill();
                // Face highlight
                ctx.set_fill(can ? 0xfffff8e8 : 0xffc8d0d2);
                ctx.set_global_alpha(0.55f);
                ctx.begin_path();
                ctx.round_rect(4.f, -13.5f, 7.f, 6.f, 1.5f);
                ctx.fill();
                ctx.set_global_alpha(1.f);
            }
            if (can) {
                // Tiny spark so “ready to craft” is obvious at a glance
                ctx.set_fill(0xfffff6a0);
                ctx.begin_path();
                ctx.move_to(14.f, -16.f);
                ctx.line_to(16.5f, -11.f);
                ctx.line_to(21.f, -13.5f);
                ctx.line_to(17.f, -9.f);
                ctx.line_to(20.f, -5.f);
                ctx.line_to(14.f, -8.f);
                ctx.line_to(8.f, -5.f);
                ctx.line_to(11.f, -9.f);
                ctx.close_path();
                ctx.fill();
            }
        }
    }
    void on_event(uint8_t event) override {
        if (event != kClick) return;
        recipe_selected = recipe_index;
        // Only the hammer zone crafts (right ~70px), like the mockup.
        float local_x = (Input::mouse_x - screen_x) / (float) Ui::scale;
        if (local_x > width / 2 - 70.f && recipe_can_craft(recipe_index))
            Game::craft_recipe(recipe_index);
    }
};

static Element *make_recipe_scroll() {
    std::vector<Element *> rows;
    uint16_t n = Recipe::recipe_count();
    for (uint16_t i = 0; i < n; ++i)
        rows.push_back(new RecipeRow(i));
    Element *list = new Ui::VContainer(rows, 0, 8, { .no_animation = 1 });
    float scroll_h = Input::is_mobile ? 320.f : 460.f;
    auto *scroll = new BagScroll(list, scroll_h);
    scroll->always_capture_wheel = 1;
    return scroll;
}

static void on_recipe_tier_prev(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    recipe_filter_tier = (uint8_t) ((recipe_filter_tier + kRecipeTierCount - 1) % kRecipeTierCount);
}
static void on_recipe_tier_next(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    recipe_filter_tier = (uint8_t) ((recipe_filter_tier + 1) % kRecipeTierCount);
}

Element *Ui::make_special_craft_panel() {
    Element *elt = make_drawer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(28, [](){
                return I18n::use_chinese ? "组装" : "Assemble";
            }, { .fill = 0xffffffff }),
            new Ui::Button(36, 36,
                new Ui::DynamicText(18, [](){ return std::string{"X"}; }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::close_game_panel();
                }, nullptr,
                { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            0, 10, {}
        ),
        make_recipe_scroll(),
        // Bottom pager: ◀  超级  ▶  (matches mockup)
        new Ui::HContainer({
            new Ui::Button(48, 44,
                new Ui::DynamicText(22, [](){ return std::string{"◀"}; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_prev, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(160, 44,
                new Ui::DynamicText(18, [](){ return std::string{recipe_tier_label()}; },
                    { .fill = 0xffffffff }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) on_recipe_tier_next(nullptr, e); },
                nullptr,
                { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(48, 44,
                new Ui::DynamicText(22, [](){ return std::string{"▶"}; },
                    { .fill = 0xffffffff }),
                on_recipe_tier_next, nullptr,
                { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
        }, 0, 12, {}),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? "列出本阶全部配方；材料不足也会显示。够数点右侧锤头，原料与成品都在背包。"
                : "All recipes in this tier (missing mats shown). Hammer when ready; mats & results in bag.";
        }, { .fill = 0xffb8e0e0, .h_justify = Style::Left }),
        new Ui::DynamicText(14, [](){ return Game::craft_status; }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        })
    }, Game::kPanelSpecial);
    // Teal panel like the mockup (overrides warm Chrome drawer fill)
    elt->style.fill = 0xff1e6a6c;
    return elt;
}

// Coin shop — tabs: specials / mats / rare mats / combos / random packs
// Top-right rarity pager filters offers (bottom pager stays category tabs).
static uint8_t shop_filter_tab = Craft::kShopSpecials;
static uint8_t shop_filter_rarity = RarityID::kCommon;

static char const *shop_rarity_label() {
    if (shop_filter_rarity >= RarityID::kNumRarities) return "?";
    return I18n::rarity_name(shop_filter_rarity);
}

static uint8_t shop_offer_matches_rarity(Craft::ShopOffer const &offer) {
    if (Craft::is_shop_announce(offer)) return 1; // always listed under 奇物
    if (offer.random_rarity)
        return offer.rarity == shop_filter_rarity ? 1 : 0;
    if (offer.id == PetalID::kNone || offer.id >= PetalID::kNumPetals) return 0;
    return PETAL_DATA[offer.id].rarity == shop_filter_rarity ? 1 : 0;
}

static uint8_t shop_tab_has_rarity(uint8_t tab, uint8_t rarity) {
    uint16_t n = Craft::shop_offer_count();
    for (uint16_t i = 0; i < n; ++i) {
        Craft::ShopOffer const &o = Craft::shop_offer_at(i);
        if (o.tab != tab) continue;
        if (o.random_rarity) {
            if (o.rarity == rarity) return 1;
            continue;
        }
        if (o.id == PetalID::kNone || o.id >= PetalID::kNumPetals) continue;
        if (PETAL_DATA[o.id].rarity == rarity) return 1;
    }
    return 0;
}

static void shop_snap_rarity_to_tab() {
    if (shop_tab_has_rarity(shop_filter_tab, shop_filter_rarity)) return;
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        if (shop_tab_has_rarity(shop_filter_tab, r)) {
            shop_filter_rarity = r;
            return;
        }
    }
}

static void on_shop_rarity_prev(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    for (uint8_t step = 0; step < RarityID::kNumRarities; ++step) {
        shop_filter_rarity = (uint8_t) ((shop_filter_rarity + RarityID::kNumRarities - 1) % RarityID::kNumRarities);
        if (shop_tab_has_rarity(shop_filter_tab, shop_filter_rarity)) return;
    }
}
static void on_shop_rarity_next(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    for (uint8_t step = 0; step < RarityID::kNumRarities; ++step) {
        shop_filter_rarity = (uint8_t) ((shop_filter_rarity + 1) % RarityID::kNumRarities);
        if (shop_tab_has_rarity(shop_filter_tab, shop_filter_rarity)) return;
    }
}

static char const *shop_tab_label() {
    if (I18n::use_chinese) {
        switch (shop_filter_tab) {
            case Craft::kShopSpecials: return "奇物";
            case Craft::kShopMaterials: return "素材";
            case Craft::kShopRareMats: return "稀有素材";
            case Craft::kShopCombos: return "组合件";
            case Craft::kShopRandom: return "随机包";
            default: return "奇物";
        }
    }
    switch (shop_filter_tab) {
        case Craft::kShopSpecials: return "Specials";
        case Craft::kShopMaterials: return "Mats";
        case Craft::kShopRareMats: return "Rare Mats";
        case Craft::kShopCombos: return "Combos";
        case Craft::kShopRandom: return "Random";
        default: return "Specials";
    }
}

static char const *shop_tab_hint() {
    if (I18n::use_chinese) {
        switch (shop_filter_tab) {
            case Craft::kShopSpecials:
                return "机制花瓣与全服广播；广播点购买后输入内容，空则取消。";
            case Craft::kShopMaterials:
                return "常见群系素材；图内刷更划算，商店补缺口。";
            case Craft::kShopRareMats:
                return "稀有/Boss 素材，对应组装高阶配方。";
            case Craft::kShopCombos:
                return "基础组合成品；贵于自刷素材+组装。";
            case Craft::kShopRandom:
                return "按稀有度开随机花瓣（期望低于专注刷图）。";
            default: return "";
        }
    }
    switch (shop_filter_tab) {
        case Craft::kShopSpecials:
            return "Utility petals + server announce; buy then type (empty cancels).";
        case Craft::kShopMaterials:
            return "Common biome mats; farming is better EV, shop fills gaps.";
        case Craft::kShopRareMats:
            return "Rare/boss mats for high assemble recipes.";
        case Craft::kShopCombos:
            return "Base craft results — priced above farm+assemble.";
        case Craft::kShopRandom:
            return "Random petal by rarity (worse EV than focused farming).";
        default: return "";
    }
}

class ShopItemRow final : public Element {
    uint16_t offer_index;
public:
    explicit ShopItemRow(uint16_t idx)
        : Element(Input::is_mobile ? 420.f : 520.f, 72.f, { .no_animation = 1 }),
          offer_index(idx) {
        style.should_render = [idx]() -> int {
            if (idx >= Craft::shop_offer_count()) return 0;
            Craft::ShopOffer const &o = Craft::shop_offer_at(idx);
            if (o.tab != shop_filter_tab) return 0;
            return shop_offer_matches_rarity(o) ? 1 : 0;
        };
    }
    void refactor() override {
        uint8_t want = style.should_render ? style.should_render() : 1;
        visible = want;
        animation.set(want);
        width = Input::is_mobile ? 420.f : 520.f;
        height = want ? 72.f : 0.f;
    }
    void on_render(Renderer &ctx) override {
        if (offer_index >= Craft::shop_offer_count()) return;
        Craft::ShopOffer const &offer = Craft::shop_offer_at(offer_index);
        if (offer.tab != shop_filter_tab) return;
        if (!shop_offer_matches_rarity(offer)) return;
        uint32_t cost = Craft::shop_offer_cost(offer);
        uint32_t have = Game::craft_owned(PetalID::kCoin);
        uint8_t can = have >= cost;
        Chrome::draw_plate(ctx, width, height, 0xff3a4a52, Chrome::kRadius, 0.f);
        if (Craft::is_shop_announce(offer)) {
            ctx.translate(-width * 0.5f + 36.f, 0);
            Chrome::draw_plate(ctx, 56.f, 56.f, 0xffb8860b, Chrome::kRadius, 0.f);
            ctx.draw_text("!", { .fill = 0xffffffff, .size = 28.f, .stroke_scale = 0.1f });
            ctx.translate(width * 0.5f - 36.f, 0);
            ctx.translate(-width * 0.5f + 110.f, -10.f);
            ctx.draw_text(I18n::use_chinese ? "全服广播" : "Server Announce", {
                .fill = 0xffffffff, .size = 18.f, .stroke_scale = 0.1f
            });
        } else if (offer.random_rarity) {
            ctx.translate(-width * 0.5f + 36.f, 0);
            Chrome::draw_plate(ctx, 56.f, 56.f, RARITY_COLORS[offer.rarity], Chrome::kRadius, 0.f);
            ctx.draw_text("?", { .fill = 0xffffffff, .size = 28.f, .stroke_scale = 0.1f });
            ctx.translate(width * 0.5f - 36.f, 0);
            ctx.translate(-width * 0.5f + 110.f, -10.f);
            char const *rname = offer.rarity < RarityID::kNumRarities
                ? RARITY_NAMES[offer.rarity] : "?";
            ctx.draw_text(I18n::use_chinese
                ? std::format("随机·{}", rname).c_str()
                : std::format("Random {}", rname).c_str(), {
                .fill = 0xffffffff, .size = 18.f, .stroke_scale = 0.1f
            });
        } else {
            PetalID::T petal_id = offer.id;
            if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
            {
                RenderContext c(&ctx);
                ctx.translate(-width * 0.5f + 36.f, 0);
                ctx.scale(0.95f);
                draw_loadout_background(ctx, petal_id, 1.f, 0);
            }
            ctx.translate(-width * 0.5f + 110.f, -10.f);
            ctx.draw_text(I18n::petal_name(petal_id), {
                .fill = 0xffffffff, .size = 18.f, .stroke_scale = 0.1f
            });
        }
        ctx.translate(0, 22.f);
        ctx.draw_text(std::format("{} {}", format_score((float) cost),
                                  I18n::use_chinese ? "币" : "coins").c_str(), {
            .fill = can ? Chrome::kGold : 0xffff8888, .size = 14.f, .stroke_scale = 0.1f
        });
        ctx.translate(width * 0.5f - 110.f, -12.f);
        {
            RenderContext c(&ctx);
            float bx = width * 0.5f - 52.f;
            ctx.translate(bx, 0);
            Chrome::draw_plate(ctx, 88.f, 44.f,
                               can ? 0xff3d8f78 : 0xff4a585a, Chrome::kRadius, 0.f);
            ctx.draw_text(I18n::use_chinese ? "购买" : "Buy", {
                .fill = 0xffffffff, .size = 16.f, .stroke_scale = 0.1f
            });
        }
    }
    void on_event(uint8_t event) override {
        if (event != kClick) return;
        if (offer_index >= Craft::shop_offer_count()) return;
        Craft::ShopOffer const &offer = Craft::shop_offer_at(offer_index);
        if (offer.tab != shop_filter_tab) return;
        if (!shop_offer_matches_rarity(offer)) return;
        float local_x = (Input::mouse_x - screen_x) / (float) Ui::scale;
        if (local_x < width * 0.5f - 100.f) return;
        uint32_t cost = Craft::shop_offer_cost(offer);
        if (Game::craft_owned(PetalID::kCoin) < cost) {
            Game::craft_status = I18n::use_chinese ? "钱币不足" : "Not enough coins";
            return;
        }
        if (Craft::is_shop_announce(offer)) {
            Game::open_shop_announce_prompt();
            return;
        }
        if (offer.random_rarity) {
            Game::craft_buy_rarity(offer.rarity);
            return;
        }
        if (!Craft::is_shop_purchasable(offer.id)) return;
        Game::craft_buy_special(offer.id);
    }
};

static Element *make_shop_scroll() {
    std::vector<Element *> rows;
    uint16_t n = Craft::shop_offer_count();
    for (uint16_t i = 0; i < n; ++i)
        rows.push_back(new ShopItemRow(i));
    Element *list = new Ui::VContainer(rows, 0, 8, { .no_animation = 1 });
    float scroll_h = Input::is_mobile ? 280.f : 400.f;
    auto *scroll = new BagScroll(list, scroll_h);
    scroll->always_capture_wheel = 1;
    return scroll;
}

static void on_shop_tab_prev(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    shop_filter_tab = (uint8_t) ((shop_filter_tab + Craft::kShopTabCount - 1) % Craft::kShopTabCount);
    shop_snap_rarity_to_tab();
}
static void on_shop_tab_next(Element *, uint8_t e) {
    if (e != Ui::kClick) return;
    shop_filter_tab = (uint8_t) ((shop_filter_tab + 1) % Craft::kShopTabCount);
    shop_snap_rarity_to_tab();
}

Element *Ui::make_shop_panel() {
    Element *elt = make_drawer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(28, [](){
                return I18n::use_chinese ? "商店" : "Shop";
            }, { .fill = 0xffffffff }),
            new Ui::HContainer({
                new Ui::Button(32, 32,
                    new Ui::DynamicText(16, [](){ return std::string{"◀"}; },
                        { .fill = 0xffffffff }),
                    on_shop_rarity_prev, nullptr,
                    { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
                new Ui::Button(88, 32,
                    new Ui::DynamicText(14, [](){ return std::string{shop_rarity_label()}; },
                        { .fill = 0xffffffff }),
                    [](Element *, uint8_t e){ if (e == Ui::kClick) on_shop_rarity_next(nullptr, e); },
                    nullptr,
                    { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
                new Ui::Button(32, 32,
                    new Ui::DynamicText(16, [](){ return std::string{"▶"}; },
                        { .fill = 0xffffffff }),
                    on_shop_rarity_next, nullptr,
                    { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
                new Ui::Button(36, 36,
                    new Ui::DynamicText(18, [](){ return std::string{"X"}; }),
                    [](Element *, uint8_t e){
                        if (e == Ui::kClick) Game::close_game_panel();
                    }, nullptr,
                    { .fill = Chrome::kBtnDanger, .line_width = 0, .round_radius = Chrome::kRadius }),
            }, 0, 6, {}),
            0, 10, {}
        ),
        new Ui::DynamicText(15, [](){
            return I18n::use_chinese
                ? std::format("持有币：{}", format_score((float) Game::craft_owned(PetalID::kCoin)))
                : std::format("Coins: {}", format_score((float) Game::craft_owned(PetalID::kCoin)));
        }, { .fill = Chrome::kGold, .h_justify = Style::Left }),
        make_shop_scroll(),
        new Ui::HContainer({
            new Ui::Button(48, 44,
                new Ui::DynamicText(22, [](){ return std::string{"◀"}; },
                    { .fill = 0xffffffff }),
                on_shop_tab_prev, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(180, 44,
                new Ui::DynamicText(18, [](){ return std::string{shop_tab_label()}; },
                    { .fill = 0xffffffff }),
                [](Element *, uint8_t e){ if (e == Ui::kClick) on_shop_tab_next(nullptr, e); },
                nullptr,
                { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(48, 44,
                new Ui::DynamicText(22, [](){ return std::string{"▶"}; },
                    { .fill = 0xffffffff }),
                on_shop_tab_next, nullptr,
                { .fill = 0xff3a3a3a, .line_width = 0, .round_radius = Chrome::kRadius }),
        }, 0, 12, {}),
        new Ui::DynamicText(13, [](){ return shop_tab_hint(); }, {
            .fill = 0xffd0d8e0, .h_justify = Style::Left
        }),
        new Ui::DynamicText(14, [](){ return Game::craft_status; }, {
            .fill = Chrome::kGold, .h_justify = Style::Left
        })
    }, Game::kPanelShop);
    elt->style.fill = 0xff2c3e50;
    return elt;
}

static TextInput *g_shop_announce_input = nullptr;

TextInput *Ui::shop_announce_text_input() {
    return g_shop_announce_input;
}

Element *Ui::make_shop_announce_prompt() {
    g_shop_announce_input = new TextInput(
        Game::shop_announce_draft,
        Input::is_mobile ? 300.f : 420.f,
        42, MAX_CHAT_LENGTH, {
            .fill = 0xfff5f5f5, .line_width = 2, .round_radius = 6,
            .h_justify = Style::Left, .no_animation = 1
        });
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(22, [](){
            return I18n::use_chinese ? "全服广播" : "Server Announce";
        }, { .fill = 0xffffffff }),
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? std::format("输入内容后回车发送（{} 币）；空内容 / Esc 取消",
                              Craft::kShopAnnounceCost)
                : std::format("Enter to send ({} coins); empty / Esc cancels",
                              Craft::kShopAnnounceCost);
        }, { .fill = 0xffd0d8e0 }),
        g_shop_announce_input,
        new Ui::HContainer({
            new Ui::Button(100, 40,
                new Ui::DynamicText(15, [](){
                    return I18n::use_chinese ? std::string{"取消"} : std::string{"Cancel"};
                }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::close_shop_announce_prompt();
                }, nullptr,
                { .fill = Chrome::kBtnIron, .line_width = 0, .round_radius = Chrome::kRadius }),
            new Ui::Button(100, 40,
                new Ui::DynamicText(15, [](){
                    return I18n::use_chinese ? std::string{"发送"} : std::string{"Send"};
                }),
                [](Element *, uint8_t e){
                    if (e == Ui::kClick) Game::submit_shop_announce();
                }, nullptr,
                { .fill = Chrome::kBtnMoss, .line_width = 0, .round_radius = Chrome::kRadius }),
        }, 0, 12)
    }, 18, 12, {
        .fill = 0xee2c3e50,
        .line_width = Chrome::kRim,
        .round_radius = Chrome::kRadius,
        .should_render = [](){
            return Game::shop_announce_prompt_open
                && (Game::alive() || Game::on_game_screen)
                && Game::should_render_game_ui();
        },
        .h_justify = Style::Center,
        .v_justify = Style::Middle
    });
    return elt;
}

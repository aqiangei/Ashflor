#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>
#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Extern.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <format>
#include <string>

using namespace Ui;

// First primary slot holding this Unique receives the account level bonus.
static uint8_t loadout_slot_is_leveled(uint8_t static_pos, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (static_pos >= Game::loadout_count) return 0;
    if (!Craft::can_level_petal(id) || Craft::petal_level_get(Game::petal_levels, id) == 0) return 0;
    for (uint8_t i = 0; i < static_pos; ++i)
        if (Game::cached_loadout[i] == id) return 0;
    return 1;
}

static Element *loadout_petal_tooltip(uint8_t static_pos, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return nullptr;
    if (loadout_slot_is_leveled(static_pos, id)
        && Ui::UiLoadout::petal_tooltips_leveled[id] != nullptr)
        return Ui::UiLoadout::petal_tooltips_leveled[id];
    return Ui::UiLoadout::petal_tooltips[id];
}

UiLoadoutPetal *Ui::UiLoadout::petal_slots[2 * MAX_SLOT_COUNT] = {nullptr};
uint8_t Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
double Ui::UiLoadout::last_key_select = 0;
uint32_t Ui::UiLoadout::num_petals_selected = 0;

void Ui::forward_secondary_select() {
    Ui::UiLoadout::last_key_select = Game::timestamp;
    uint8_t n = Game::loadout_count;
    if (n == 0) { Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT; return; }
    if (Ui::UiLoadout::selected_with_keys == MAX_SLOT_COUNT)
        Ui::UiLoadout::selected_with_keys = 0;
    else
        ++Ui::UiLoadout::selected_with_keys;
    for (uint8_t i = 0; i < n; ++i) {
        Ui::UiLoadout::selected_with_keys = Ui::UiLoadout::selected_with_keys % n;
        if (Game::cached_loadout[Game::loadout_count + Ui::UiLoadout::selected_with_keys] != PetalID::kNone) return;
        ++Ui::UiLoadout::selected_with_keys;
    }
    Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
}

void Ui::backward_secondary_select() {
    Ui::UiLoadout::last_key_select = Game::timestamp;
    uint8_t n = Game::loadout_count;
    if (n == 0) { Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT; return; }
    if (Ui::UiLoadout::selected_with_keys == MAX_SLOT_COUNT)
        return Ui::forward_secondary_select();
    for (uint8_t i = 0; i < n; ++i) {
        Ui::UiLoadout::selected_with_keys = (Ui::UiLoadout::selected_with_keys + n - 1) % n;
        if (Game::cached_loadout[Game::loadout_count + Ui::UiLoadout::selected_with_keys] != PetalID::kNone) return;
    }
    Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
}

static uint8_t static_to_dynamic(uint8_t static_pos) {
    if (static_pos >= Game::loadout_count) 
        return std::min(MAX_SLOT_COUNT + static_pos - Game::loadout_count, 2 * MAX_SLOT_COUNT);
    else
        return static_pos;
}

void Ui::ui_delete_petal(uint8_t static_pos) {
    if (static_pos < 2 * MAX_SLOT_COUNT)
        Game::cached_loadout[static_pos] = PetalID::kNone;
    Game::delete_petal(static_pos);
    UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[static_pos];
    if (p != nullptr) {
        p->curr_pos = 2 * MAX_SLOT_COUNT;
        p->petal_id = PetalID::kNone;
    }
}

void Ui::ui_swap_petals(uint8_t static_pos1, uint8_t static_pos2) {
    if (static_pos1 >= 2 * MAX_SLOT_COUNT || static_pos2 >= 2 * MAX_SLOT_COUNT) return;
    if (static_pos1 == static_pos2) return;
    PetalID::T id1 = Game::cached_loadout[static_pos1];
    PetalID::T id2 = Game::cached_loadout[static_pos2];
    if (id1 == id2) return;
    uint8_t primary = Game::loadout_count;
    // Coins (and invalid ids) cannot land on primary. kNone must be allowed so
    // primary ↔ empty secondary (and the reverse) can clear / fill a main slot.
    if ((static_pos1 < primary && Recipe::is_bag_only(id2))
        || (static_pos2 < primary && Recipe::is_bag_only(id1)))
        return;
    // Match server: Corruption cannot leave the primary loadout while alive.
    // Optimistic R-swap used to move it into secondary, then snap back and
    // leave the paired secondary gear looking lost/displaced.
    if (Game::alive()) {
        auto is_corr = [](PetalID::T id) -> uint8_t {
            return id != PetalID::kNone && id < PetalID::kNumPetals
                && PETAL_DATA[id].attributes.corruption;
        };
        if ((static_pos1 < primary && static_pos2 >= primary && is_corr(id1))
            || (static_pos2 < primary && static_pos1 >= primary && is_corr(id2)))
            return;
    }
    // Widgets stay glued to their slot index. Remapping static_pos made the
    // next drag operate on a bag index and silently no-op.
    Game::cached_loadout[static_pos1] = id2;
    Game::cached_loadout[static_pos2] = id1;
    auto paint = [](UiLoadoutPetal *w, PetalID::T id) {
        if (w == nullptr) return;
        w->petal_id = id;
        w->last_id = id;
        w->no_change_ticks = 100;
        // Empty primaries never rendered (animation stuck near 0). Snap visible
        // so a petal arriving from secondary shows on the main slot immediately.
        if (id != PetalID::kNone) {
            w->showed = 1;
            w->animation = 1.f;
        }
    };
    paint(Ui::UiLoadout::petal_slots[static_pos1], id2);
    paint(Ui::UiLoadout::petal_slots[static_pos2], id1);
    Game::swap_petals(static_pos1, static_pos2);
}

// Empty / Basic only — never displace a real equipped petal.
static uint8_t find_auto_equip_target(PetalID::T incoming) {
    (void) incoming;
    uint8_t n = Game::loadout_count;
    for (uint8_t i = 0; i < n; ++i) {
        if (Game::cached_loadout[i] == PetalID::kNone)
            return i;
    }
    for (uint8_t i = 0; i < n; ++i) {
        if (Game::cached_loadout[i] == PetalID::kBasic)
            return i;
    }
    for (uint8_t i = 0; i < n; ++i) {
        uint8_t s = (uint8_t) (n + i);
        if (Game::cached_loadout[s] == PetalID::kNone)
            return s;
    }
    return (uint8_t) -1;
}

// Bounce ghost: fly toward loadout tip then return to origin.
static PetalID::T bag_bounce_id = PetalID::kNone;
static float bag_bounce_ox = 0, bag_bounce_oy = 0;
static float bag_bounce_tx = 0, bag_bounce_ty = 0;
static double bag_bounce_start = 0;
static constexpr float BAG_BOUNCE_MS = 320.f;

void Ui::play_bag_equip_bounce(PetalID::T id, float from_sx, float from_sy,
                               float tip_sx, float tip_sy) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    bag_bounce_id = id;
    bag_bounce_ox = from_sx;
    bag_bounce_oy = from_sy;
    if (tip_sx < 0.f || tip_sy < 0.f) {
        // Aim at the middle of the primary bar (slot 0 or first visible bg).
        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[0];
        if (slot != nullptr && slot->showed) {
            tip_sx = slot->screen_x;
            tip_sy = slot->screen_y;
        } else {
            tip_sx = Ui::window_width * 0.5f;
            tip_sy = Ui::window_height - 80.f * Ui::scale;
        }
    }
    bag_bounce_tx = tip_sx;
    bag_bounce_ty = tip_sy;
    bag_bounce_start = Game::timestamp;
}

static PetalID::T bag_bounce_ghost_petal() {
    if (bag_bounce_id == PetalID::kNone) return PetalID::kNone;
    float elapsed = (float) (Game::timestamp - bag_bounce_start);
    if (elapsed >= BAG_BOUNCE_MS) {
        bag_bounce_id = PetalID::kNone;
        return PetalID::kNone;
    }
    return bag_bounce_id;
}

static uint8_t loadout_editable() {
    return Game::alive()
        || (Game::on_game_screen && Game::simulation.ent_exists(Game::camera_id));
}

uint8_t Ui::ui_auto_equip_from_bag(uint8_t bag_pos, float screen_x, float screen_y) {
    if (!loadout_editable()) return 0;
    uint8_t n = Game::loadout_count;
    if (bag_pos < n || bag_pos >= n + MAX_SLOT_COUNT) return 0;
    PetalID::T id = Game::cached_loadout[bag_pos];
    if (id == PetalID::kNone || id >= PetalID::kNumPetals || Recipe::is_bag_only(id)) return 0;
    uint8_t target = find_auto_equip_target(id);
    if (target == (uint8_t) -1 || target == bag_pos) {
        play_bag_equip_bounce(id, screen_x, screen_y);
        return 0;
    }

    ui_swap_petals(target, bag_pos);
    if (Game::alive()) {
        // Widget now living on the primary slot starts at the bag click
        // and lerps onto that slot (swap remaps which object sits at target).
        UiLoadoutPetal *at = Ui::UiLoadout::petal_slots[target];
        if (at != nullptr) {
            at->x = (screen_x - Ui::window_width / 2) / Ui::scale;
            at->y = (screen_y - Ui::window_height / 2) / Ui::scale;
            at->width = 48;
            at->height = 48;
            at->petal_id = id;
            at->last_id = id;
            at->selected = 0;
            at->drag_from_bag = 0;
            at->fly_hint = 1;
            at->showed = 1;
            at->style.layer = 1;
        }
    }
    return 1;
}

static void apply_unequip_fly(UiLoadoutPetal *at_dest, float from_sx, float from_sy,
                             float from_w = 60.f, float from_h = 60.f) {
    if (at_dest == nullptr) return;
    PetalID::T id = Game::cached_loadout[at_dest->static_pos];
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    at_dest->petal_id = id;
    at_dest->last_id = id;
    at_dest->x = (from_sx - Ui::window_width / 2) / Ui::scale;
    at_dest->y = (from_sy - Ui::window_height / 2) / Ui::scale;
    at_dest->width = from_w;
    at_dest->height = from_h;
    at_dest->fly_hint = 1;
    at_dest->style.layer = 1;
    at_dest->showed = 1;
}

static void slot_screen_rect(uint8_t static_pos, float &sx, float &sy, float &sw, float &sh) {
    uint8_t dyn = static_to_dynamic(static_pos);
    UiLoadoutPetal *w = Ui::UiLoadout::petal_slots[static_pos];
    UiLoadoutSlot *bg = dyn < 2 * MAX_SLOT_COUNT + 1
        ? Ui::UiLoadout::petal_backgrounds[dyn] : nullptr;
    if (w != nullptr && w->showed && w->last_id != PetalID::kNone) {
        sx = w->screen_x;
        sy = w->screen_y;
        sw = w->width > 1.f ? w->width : 60.f;
        sh = w->height > 1.f ? w->height : 60.f;
        return;
    }
    if (bg != nullptr) {
        sx = bg->screen_x;
        sy = bg->screen_y;
        sw = bg->width;
        sh = bg->height;
        return;
    }
    sx = Ui::window_width / 2;
    sy = Ui::window_height;
    sw = sh = 60.f;
}

uint8_t Ui::ui_unequip_to_backpack(uint8_t static_pos, float x, float y) {
    if (!loadout_editable()) return 0;
    uint8_t n = Game::loadout_count;
    // Primary or secondary → backpack stacks. Never park in secondary.
    if (static_pos >= 2 * n) return 0;
    if (Game::cached_loadout[static_pos] == PetalID::kNone) return 0;
    (void) x; (void) y;
    // Always stack into the backpack — one collection, no separate warehouse.
    Game::deposit_bank_petal(static_pos);
    return 1;
}

void Ui::ui_compact_backpack() {
    if (!loadout_editable()) return;
    uint8_t n = Game::loadout_count;
    if (n == 0) return;
    // Pure bag only [2n, n+MAX). Starting at n used to slide secondary gear
    // left into holes and look like "副槽位移".
    uint8_t write = (uint8_t) (2 * n);
    uint8_t end = (uint8_t) (n + MAX_SLOT_COUNT);
    if (end > 2 * MAX_SLOT_COUNT) end = 2 * MAX_SLOT_COUNT;
    if (write >= end) return;
    for (uint8_t read = write; read < end; ++read) {
        if (Game::cached_loadout[read] == PetalID::kNone) continue;
        if (write != read)
            ui_swap_petals(write, read);
        ++write;
    }
}

static uint8_t bag_gesture_active = 0;
static uint8_t bag_gesture_dragging = 0;
static uint8_t bag_gesture_pos = 0;
static PetalID::T bag_gesture_bank_id = PetalID::kNone;
static uint8_t bag_gesture_selected_bump = 0;
static float bag_gesture_x = 0;
static float bag_gesture_y = 0;
static uint32_t bag_gesture_touch = (uint32_t) -1;
static double bag_gesture_started = 0;

static void bag_gesture_clear_selection_bump() {
    if (!bag_gesture_selected_bump) return;
    if (Ui::UiLoadout::num_petals_selected > 0)
        --Ui::UiLoadout::num_petals_selected;
    bag_gesture_selected_bump = 0;
}

void Ui::cancel_petal_drags() {
    bag_gesture_active = 0;
    bag_gesture_dragging = 0;
    bag_gesture_bank_id = PetalID::kNone;
    bag_gesture_clear_selection_bump();
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[i];
        if (p == nullptr || !p->selected) continue;
        p->selected = 0;
        p->drag_from_bag = 0;
        p->persistent_touch_id = (uint32_t)-1;
        p->style.layer = 0;
        p->fly_hint = 0;
    }
    Ui::UiLoadout::num_petals_selected = 0;
}

uint8_t Ui::loadout_drag_over_bag(float x, float y) {
    if (Game::game_panel != Game::kPanelInventory) return 0;
    Element *panel = Ui::game_panel_roots[Game::kPanelInventory];
    if (panel == nullptr || !panel->visible) return 0;
    float hw = panel->width * Ui::scale / 2;
    float hh = panel->height * Ui::scale / 2;
    return fabsf(x - panel->screen_x) <= hw && fabsf(y - panel->screen_y) <= hh;
}

void Ui::ui_swap_paired_slots(uint8_t from_pos, uint8_t to_pos) {
    if (from_pos >= 2 * MAX_SLOT_COUNT || to_pos >= 2 * MAX_SLOT_COUNT) return;
    if (from_pos == to_pos) return;
    PetalID::T id_a = Game::cached_loadout[from_pos];
    PetalID::T id_b = Game::cached_loadout[to_pos];
    if (id_a == id_b) return;
    float ax, ay, aw, ah, bx, by, bw, bh;
    slot_screen_rect(from_pos, ax, ay, aw, ah);
    slot_screen_rect(to_pos, bx, by, bw, bh);
    ui_swap_petals(from_pos, to_pos);
    // Widgets stay on their slot index; content crossed — each flies from the
    // other's old screen rect into its new home (primary ↔ secondary).
    if (id_b != PetalID::kNone)
        apply_unequip_fly(Ui::UiLoadout::petal_slots[from_pos], bx, by, bw, bh);
    if (id_a != PetalID::kNone)
        apply_unequip_fly(Ui::UiLoadout::petal_slots[to_pos], ax, ay, aw, ah);
}

void Ui::ui_toggle_petal_slot(uint8_t static_pos) {
    if (!loadout_editable()) return;
    if (static_pos >= Game::loadout_count + MAX_SLOT_COUNT) return;
    if (Game::cached_loadout[static_pos] == PetalID::kNone) return;
    uint8_t n = Game::loadout_count;
    if (static_pos < n) {
        // Click: primary i ↔ secondary i only (never first-empty scan).
        // Same-id is a content no-op — stash into pure bag instead.
        uint8_t sec = (uint8_t) (n + static_pos);
        if (Game::cached_loadout[sec] == Game::cached_loadout[static_pos]) {
            ui_unequip_to_backpack(static_pos);
            return;
        }
        ui_swap_paired_slots(static_pos, sec);
        return;
    }
    if (static_pos < 2 * n) {
        uint8_t pri = (uint8_t) (static_pos - n);
        if (Game::cached_loadout[pri] == Game::cached_loadout[static_pos]) {
            ui_unequip_to_backpack(static_pos);
            return;
        }
        ui_swap_paired_slots(static_pos, pri);
        return;
    }
}

static uint8_t dynamic_to_static(uint8_t dynamic_pos) {
    if (dynamic_pos >= MAX_SLOT_COUNT)
        dynamic_pos -= (MAX_SLOT_COUNT - Game::loadout_count);
    return dynamic_pos; 
}


uint8_t Ui::find_loadout_drop_target(float x, float y) {
    // Drawer sits over the left of the loadout bar — don't drop onto slots under it.
    if (Ui::game_panel_blocks_loadout(x, y)) return (uint8_t)-1;
    float min_dist = 10000;
    uint8_t min_index = (uint8_t)-1;
    float const pad = Input::is_mobile ? 36.f : 20.f;
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT + 1; ++i) {
        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[i];
        // Skip unused secondary shells — stale screen coords steal bag drops.
        if (slot == nullptr || !slot->visible) continue;
        float dist = std::max(fabsf(x - slot->screen_x), fabsf(y - slot->screen_y));
        if (dist >= (slot->width + pad) * Ui::scale / 2) continue;
        if (dist >= min_dist) continue;
        min_dist = dist;
        min_index = i;
        if (dist <= slot->width * Ui::scale / 2) break;
    }
    return min_index;
}

uint8_t Ui::loadout_dynamic_to_static(uint8_t dynamic_pos) {
    return dynamic_to_static(dynamic_pos);
}

uint8_t Ui::bag_gesture_busy() {
    return bag_gesture_active;
}

// Soft bag press still inside the open inventory (and not on a loadout slot)
// must stay scrollable — fast vertical swipes jump past drag_slop otherwise.
static uint8_t bag_press_scrollable(float mx, float my) {
    if (!Input::is_mobile) return 0;
    if (Game::game_panel != Game::kPanelInventory) return 0;
    if (Ui::bag_gesture_wants_equip(mx, my)) return 0;
    if (Ui::find_loadout_drop_target(mx, my) != (uint8_t) -1) return 0;
    return Ui::loadout_drag_over_bag(mx, my);
}

uint8_t Ui::bag_gesture_wants_equip(float x, float y) {
    if (!bag_gesture_active) return 0;
    if (Ui::find_loadout_drop_target(x, y) != (uint8_t) -1) return 1;
    if (Ui::loadout_bar_hit(x, y)) return 1;
    // Left the inventory drawer — equip drag, not list scroll.
    if (!Ui::loadout_drag_over_bag(x, y)) return 1;
    // Still inside the bag: any clear downward swipe toward the loadout bar
    // (drawer sits above it) is an equip drag — do not let ScrollContainer cancel.
    float dy = y - bag_gesture_y;
    float dx = x - bag_gesture_x;
    float slop = Input::pointer_slop() * 0.45f;
    if (dy >= slop && dy >= std::fabs(dx) * 0.65f)
        return 1;
    Element *panel = Ui::game_panel_roots[Game::kPanelInventory];
    if (panel == nullptr || !panel->visible) return 0;
    float bag_bottom = panel->screen_y + panel->height * Ui::scale * 0.5f;
    float edge = 140.f * Ui::scale;
    if (dy >= slop && y >= bag_bottom - edge)
        return 1;
    return 0;
}

uint8_t Ui::petal_drag_committed() {
    float const slop = Input::pointer_slop();
    if (bag_gesture_dragging) return 1;
    if (bag_gesture_active) {
        float mx = bag_gesture_x;
        float my = bag_gesture_y;
        auto it = Input::touches.find(bag_gesture_touch);
        if (it != Input::touches.end()) {
            mx = it->second.x;
            my = it->second.y;
        } else {
            auto ended = Input::ended_touches.find(bag_gesture_touch);
            if (ended != Input::ended_touches.end()) {
                mx = ended->second.x;
                my = ended->second.y;
            }
        }
        if (Ui::bag_gesture_wants_equip(mx, my)
            && std::hypot(mx - bag_gesture_x, my - bag_gesture_y) >= slop * 0.75f)
            return 1;
        // Match finish_bag_gesture drag threshold so scroll can win soft presses.
        if (std::hypot(mx - bag_gesture_x, my - bag_gesture_y) >= slop * 1.6f) {
            if (bag_press_scrollable(mx, my)) return 0;
            return 1;
        }
    }
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[i];
        if (p == nullptr || !p->selected) continue;
        float mx = Input::mouse_x;
        float my = Input::mouse_y;
        auto it = Input::touches.find(p->persistent_touch_id);
        if (it != Input::touches.end()) {
            mx = it->second.x;
            my = it->second.y;
        }
        if (std::hypot(mx - p->press_x, my - p->press_y) >= slop)
            return 1;
    }
    return 0;
}

uint8_t Ui::loadout_bar_hit(float x, float y) {
    float const pad = Input::is_mobile ? 48.f : 20.f;
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT + 1; ++i) {
        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[i];
        if (slot == nullptr || !slot->visible) continue;
        float dist = std::max(fabsf(x - slot->screen_x), fabsf(y - slot->screen_y));
        if (dist < (slot->width + pad) * Ui::scale / 2)
            return 1;
    }
    if (Ui::rotation_wheel_hit(x, y)) return 1;
    return 0;
}

void Ui::begin_bag_gesture(uint8_t static_pos, float x, float y, uint32_t touch_id,
                           PetalID::T bank_id) {
    uint8_t from_bank = static_pos == (uint8_t) -1;
    if (from_bank) {
        if (bank_id == PetalID::kNone || bank_id >= PetalID::kNumPetals) return;
        if (Game::craft_bank[bank_id] == 0 || Recipe::is_bag_only(bank_id)) return;
    } else {
        if (static_pos >= 2 * MAX_SLOT_COUNT) return;
        if (Game::cached_loadout[static_pos] == PetalID::kNone) return;
        bank_id = PetalID::kNone;
    }
    // Mobile delivers kMouseDown every held frame — do not reset mid-gesture.
    if (bag_gesture_active
        && bag_gesture_pos == static_pos
        && bag_gesture_bank_id == bank_id
        && bag_gesture_touch == touch_id)
        return;
    bag_gesture_clear_selection_bump();
    bag_gesture_active = 1;
    bag_gesture_dragging = 0;
    bag_gesture_pos = static_pos;
    bag_gesture_bank_id = bank_id;
    bag_gesture_x = x;
    bag_gesture_y = y;
    bag_gesture_touch = touch_id;
    bag_gesture_started = Game::timestamp;
}

PetalID::T Ui::bag_gesture_ghost_petal() {
    // Slot-copy drags already have a UiLoadoutPetal ghost — only bank stacks
    // need this floating icon.
    if (!bag_gesture_active || !bag_gesture_dragging) return PetalID::kNone;
    return bag_gesture_bank_id;
}

void Ui::finish_bag_gesture() {
    if (!bag_gesture_active) return;
    float mx = Input::mouse_x;
    float my = Input::mouse_y;
    uint8_t released = 0;
    if (Input::is_mobile) {
        auto iter = Input::touches.find(bag_gesture_touch);
        if (iter == Input::touches.end()) {
            released = 1;
            auto ended = Input::ended_touches.find(bag_gesture_touch);
            if (ended != Input::ended_touches.end()) {
                mx = ended->second.x;
                my = ended->second.y;
            }
        } else {
            mx = iter->second.x;
            my = iter->second.y;
        }
    } else {
        released = BitMath::at(Input::mouse_buttons_released, Input::LeftMouse);
    }
    float dist = std::hypot(mx - bag_gesture_x, my - bag_gesture_y);
    uint8_t over_slot = find_loadout_drop_target(mx, my) != (uint8_t) -1;
    // Mobile: promote sooner when aiming at loadout so scroll does not win first.
    float drag_slop = Input::is_mobile ? Input::pointer_slop() * 1.15f : Input::pointer_slop();
    uint8_t from_bank = bag_gesture_pos == (uint8_t) -1;
    // Inside open bag: vertical swipe scrolls the list. Promote to a loadout
    // drag once the finger leaves the panel, aims at the bar, or lands on a slot.
    uint8_t keep_soft = bag_press_scrollable(mx, my);
    uint8_t aim_equip = Ui::bag_gesture_wants_equip(mx, my);
    if (!bag_gesture_dragging
        && (!keep_soft || aim_equip)
        && dist >= Input::pointer_slop() * (aim_equip && Input::is_mobile ? 0.55f : 1.f)
        && (over_slot || aim_equip || dist >= drag_slop)) {
        bag_gesture_dragging = 1;
        // Collapse backpack while aiming at loadout slots so the bar is clear.
        if (Game::game_panel == Game::kPanelInventory)
            Game::close_game_panel();
        if (from_bank) {
            if (!bag_gesture_selected_bump) {
                ++Ui::UiLoadout::num_petals_selected;
                bag_gesture_selected_bump = 1;
            }
        } else {
            begin_petal_drag(bag_gesture_pos, bag_gesture_touch);
        }
    }
    if (!released) return;
    uint8_t pos = bag_gesture_pos;
    PetalID::T bank_id = bag_gesture_bank_id;
    uint8_t was_drag = bag_gesture_dragging;
    bag_gesture_active = 0;
    bag_gesture_dragging = 0;
    bag_gesture_bank_id = PetalID::kNone;
    bag_gesture_clear_selection_bump();
    if (!from_bank) {
        UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[pos];
        if (p != nullptr && p->selected) {
            --Ui::UiLoadout::num_petals_selected;
            p->selected = 0;
            p->drag_from_bag = 0;
            p->style.layer = 0;
            p->persistent_touch_id = (uint32_t) -1;
        }
    }
    // A deliberate long hold only inspects the petal. A short tap equips.
    if (!was_drag) {
        if (Input::is_mobile && Game::timestamp - bag_gesture_started >= 450)
            return;
        // Moved past slop inside the bag = scroll/swipe, not tap-to-equip.
        if (Input::is_mobile && dist >= Input::pointer_slop())
            return;
        if (from_bank) {
            if (!Game::withdraw_bank_petal(bank_id))
                play_bag_equip_bounce(bank_id, bag_gesture_x, bag_gesture_y);
        } else if (Game::cached_loadout[pos] != PetalID::kNone)
            ui_auto_equip_from_bag(pos, bag_gesture_x, bag_gesture_y);
        return;
    }
    // Releasing a drag away from a slot returns it to the bag.
    if (!over_slot) return;
    uint8_t dyn = find_loadout_drop_target(mx, my);
    if (dyn == 2 * MAX_SLOT_COUNT) {
        if (!from_bank) ui_delete_petal(pos);
        return;
    }
    if (dyn == (uint8_t) -1) return;
    uint8_t dest = loadout_dynamic_to_static(dyn);
    if (from_bank) {
        float tip_sx = mx, tip_sy = my;
        UiLoadoutSlot *bg = dyn < 2 * MAX_SLOT_COUNT + 1
            ? Ui::UiLoadout::petal_backgrounds[dyn] : nullptr;
        if (bg != nullptr) { tip_sx = bg->screen_x; tip_sy = bg->screen_y; }
        if (!Game::withdraw_bank_petal(bank_id, dest))
            play_bag_equip_bounce(bank_id, bag_gesture_x, bag_gesture_y, tip_sx, tip_sy);
    } else if (Game::cached_loadout[pos] != PetalID::kNone && dest != pos)
        ui_swap_petals(dest, pos);
}

void Ui::begin_petal_drag(uint8_t static_pos, uint32_t touch_id) {
    if (static_pos >= 2 * MAX_SLOT_COUNT) return;
    UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[static_pos];
    if (p == nullptr) return;
    if (Game::cached_loadout[static_pos] == PetalID::kNone) return;
    if (!p->selected) {
        ++Ui::UiLoadout::num_petals_selected;
        p->selected = 1;
    }
    p->drag_from_bag = 1;
    if (Game::game_panel == Game::kPanelInventory)
        Game::close_game_panel();
    Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
    p->style.layer = 1;
    // Snap ghost to cursor so bag-origin drags don't jump from the bottom bar
    float mx = Input::mouse_x;
    float my = Input::mouse_y;
    auto touch_iter = Input::touches.find(touch_id);
    if (touch_iter != Input::touches.end()) {
        mx = touch_iter->second.x;
        my = touch_iter->second.y;
        p->persistent_touch_id = touch_id;
    }
    p->press_x = mx;
    p->press_y = my;
    p->x = (mx - Ui::window_width / 2) / Ui::scale;
    p->y = (my - Ui::window_height / 2) / Ui::scale;
}

UiLoadoutPetal::UiLoadoutPetal(uint8_t pos) : Element(60, 60), 
    persistent_touch_id((uint32_t)-1),
    press_x(0), press_y(0),
    static_pos(pos), curr_pos(pos), no_change_ticks(0), selected(0), drag_from_bag(0), fly_hint(0),
    petal_id(PetalID::kNone), last_id(PetalID::kNone) 
{
    reload = 1;
    health = 1;
    style.should_render = [&](){
        if (!loadout_editable()) return false;
        //coincidentally also works for trashing
        if (static_pos >= MAX_SLOT_COUNT + Game::loadout_count) return false;
        if (curr_pos == 2 * MAX_SLOT_COUNT) return false;
        // Pure bag petals: only while dragged or flying back into the bag
        if (!selected && !fly_hint
            && static_pos >= Game::loadout_count + Game::loadout_count
            && static_pos < Game::loadout_count + MAX_SLOT_COUNT)
            return false;
        // Game::tick owns server acknowledgement / timeout for every slot,
        // including offscreen backpack widgets which never render here.
        petal_id = Game::cached_loadout[static_pos];
        if (petal_id != PetalID::kNone && petal_id < PetalID::kNumPetals)
            last_id = petal_id;
        if (static_pos < Game::loadout_count && Game::alive()) {
            Entity const &player = Game::simulation.get_ent(Game::player_id);
            if (player.get_state_loadout_reloads(static_pos)) {
                float old = player.get_loadout_reloads(static_pos) / 255.0f;
                if (old > reload) reload.set(old);
                else reload = old;
            }
            // Poll every frame (not only on dirty-state). Hits on dummies /
            // mobs can land between should_render frames and drop the flag.
            {
                float hp = player.get_loadout_healths(static_pos) / 255.0f;
                if (hp < (float) health) health.set(hp);
                else health = hp;
            }
        }
        else {
            reload = 0;
            health = 1;
        }
        if ((no_change_ticks == 0 && petal_id == PetalID::kNone) || last_id == PetalID::kNone) return false;
        return true;
    };
    style.animate = [&](Element *elt, Renderer &ctx) {
        float lerp_amt = Ui::lerp_amount * 0.75;
        reload.step(lerp_amt);
        health.step(lerp_amt);
        if (curr_pos != 2 * MAX_SLOT_COUNT) 
            curr_pos = static_to_dynamic(static_pos);
        
        float mouse_x, mouse_y;
        uint8_t released;
        if (Input::is_mobile) {
            released = 0;
            auto iter = Input::touches.find(persistent_touch_id);
            if (iter == Input::touches.end()) {
                if (persistent_touch_id != (uint32_t)-1)
                    released = 1;
                auto ended = Input::ended_touches.find(persistent_touch_id);
                persistent_touch_id = (uint32_t)-1;
                mouse_x = ended != Input::ended_touches.end() ? ended->second.x : elt->screen_x;
                mouse_y = ended != Input::ended_touches.end() ? ended->second.y : elt->screen_y;
            } else {
                mouse_x = iter->second.x;
                mouse_y = iter->second.y;
            }
        }
        else {
            mouse_x = Input::mouse_x;
            mouse_y = Input::mouse_y;
            released = BitMath::at(Input::mouse_buttons_released, Input::LeftMouse);
        }
        UiLoadoutSlot *parent_slot = curr_pos < 2 * MAX_SLOT_COUNT + 1
            ? Ui::UiLoadout::petal_backgrounds[curr_pos] : nullptr;
        if (parent_slot == nullptr) return;
        if (selected && loadout_editable()) {
            style.layer = 1;
            float drag_dist = std::hypot(mouse_x - press_x, mouse_y - press_y);
            float const kClickSlop = Input::pointer_slop();
            uint8_t is_click = drag_dist < kClickSlop;
            // Past slop = dragging; hide inspect tip so it doesn't stick to the finger.
            if (!is_click || released)
                rendering_tooltip = 0;
            else if (last_id != PetalID::kNone) {
                rendering_tooltip = 1;
                tooltip = loadout_petal_tooltip(static_pos, last_id);
            }
            // Bag-origin drags: InventoryBagSlot owns click + drop (avoids
            // double-swap when this widget runs before the bag cell).
            if (drag_from_bag) {
                if (!released) {
                    uint8_t potential_swap = find_loadout_drop_target(mouse_x, mouse_y);
                    if (potential_swap != ((uint8_t)-1)
                        && potential_swap != static_to_dynamic(static_pos)
                        && potential_swap != 2 * MAX_SLOT_COUNT) {
                        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[potential_swap];
                        x = lerp(x, (slot->screen_x - Ui::window_width / 2) / Ui::scale, lerp_amt);
                        y = lerp(y, (slot->screen_y - Ui::window_height / 2) / Ui::scale, lerp_amt);
                        width = lerp(width, slot->width, lerp_amt);
                        height = lerp(height, slot->height, lerp_amt);
                    } else {
                        x = lerp(x, (mouse_x - Ui::window_width / 2) / Ui::scale, lerp_amt);
                        y = lerp(y, (mouse_y - Ui::window_height / 2) / Ui::scale, lerp_amt);
                        width = lerp(width, parent_slot->width + 10, lerp_amt);
                        height = lerp(height, parent_slot->height + 10, lerp_amt);
                        ctx.rotate(sin(Game::timestamp / 150) * 0.1);
                    }
                }
            } else if (released && is_click) {
                ui_toggle_petal_slot(static_pos);
            } else if (released && !is_click
                && static_pos < 2 * Game::loadout_count
                && loadout_drag_over_bag(mouse_x, mouse_y)) {
                // Open bag panel wins over nearby secondary slot hits.
                ui_unequip_to_backpack(static_pos, mouse_x, mouse_y);
            } else {
                uint8_t potential_swap = find_loadout_drop_target(mouse_x, mouse_y);
                if (potential_swap != ((uint8_t)-1) && potential_swap != static_to_dynamic(static_pos)) {
                    if (released) {
                        if (potential_swap == 2 * MAX_SLOT_COUNT)
                            ui_delete_petal(static_pos);
                        else {
                            potential_swap = loadout_dynamic_to_static(potential_swap);
                            ui_swap_petals(potential_swap, static_pos);
                        }
                    } else {
                        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[potential_swap];
                        x = lerp(x, (slot->screen_x - Ui::window_width / 2) / Ui::scale, lerp_amt);
                        y = lerp(y, (slot->screen_y - Ui::window_height / 2) / Ui::scale, lerp_amt);
                        width = lerp(width, slot->width, lerp_amt);
                        height = lerp(height, slot->height, lerp_amt);
                    }
                } else if (released && !is_click
                    && static_pos < 2 * Game::loadout_count) {
                    // Drag off equipment → backpack stacks.
                    ui_unequip_to_backpack(static_pos);
                } else {
                    x = lerp(x, (mouse_x - Ui::window_width / 2) / Ui::scale, lerp_amt);
                    y = lerp(y, (mouse_y - Ui::window_height / 2) / Ui::scale, lerp_amt);
                    width = lerp(width, parent_slot->width + 10, lerp_amt);
                    height = lerp(height, parent_slot->height + 10, lerp_amt);
                    ctx.rotate(sin(Game::timestamp / 150) * 0.1);
                }
            }
        } else if (Ui::UiLoadout::selected_with_keys + Game::loadout_count == static_pos && loadout_editable()) {
            if (!showed) lerp_amt = 1;
            x = lerp(x, (parent_slot->screen_x - Ui::window_width / 2) / Ui::scale, lerp_amt);
            y = lerp(y, (parent_slot->screen_y - Ui::window_height / 2) / Ui::scale, lerp_amt);
            width = lerp(width, parent_slot->width + 20, lerp_amt);
            height = lerp(height, parent_slot->height + 20, lerp_amt);
            ctx.rotate(sin(Game::timestamp / 150) * 0.1);
        } else {
            float tx = (parent_slot->screen_x - Ui::window_width / 2) / Ui::scale;
            float ty = (parent_slot->screen_y - Ui::window_height / 2) / Ui::scale;
            float tw = parent_slot->width;
            float th = parent_slot->height;
            // Unequip fly: pure-bag petals have no bottom-bar home — aim at inventory chrome
            uint8_t n = Game::loadout_count;
            uint8_t in_pure_bag = static_pos >= 2 * n
                && static_pos < n + MAX_SLOT_COUNT;
            if (fly_hint && in_pure_bag) {
                Element *aim = nullptr;
                if (Game::game_panel == Game::kPanelInventory
                    && Ui::game_panel_roots[Game::kPanelInventory] != nullptr)
                    aim = Ui::game_panel_roots[Game::kPanelInventory];
                else if (Ui::game_side_buttons_root != nullptr)
                    aim = Ui::game_side_buttons_root;
                if (aim != nullptr) {
                    tx = (aim->screen_x - Ui::window_width / 2) / Ui::scale;
                    ty = (aim->screen_y - Ui::window_height / 2) / Ui::scale;
                    tw = 40;
                    th = 40;
                }
            }
            if (fly_hint) {
                style.layer = 1;
                if (!showed) showed = 1;
                // Primary↔secondary crosses a short gap — keep it readable.
                lerp_amt = Ui::lerp_amount * 2.1f;
            } else {
                style.layer = 0;
                if (!showed) lerp_amt = 1;
            }
            x = lerp(x, tx, lerp_amt);
            y = lerp(y, ty, lerp_amt);
            width = lerp(width, tw, lerp_amt);
            height = lerp(height, th, lerp_amt);
            if (fly_hint && fabsf(x - tx) + fabsf(y - ty) < 4.f) {
                fly_hint = 0;
                style.layer = 0;
            }
        }
        //if (!Game::alive())
            //Ui::UiLoadout::petal_selected = nullptr;
        ctx.scale((float) animation);
        if (released && selected) {
            --Ui::UiLoadout::num_petals_selected;
            selected = 0;
            drag_from_bag = 0;
        }
    };
    Ui::UiLoadout::petal_slots[static_pos] = this;
}

void UiLoadoutPetal::on_render(Renderer &ctx) {
    if (Game::loadout_preview_bank() != 0 && static_pos < Game::loadout_count && !selected) {
        rendering_tooltip = 0;
        return;
    }
    if (last_id == PetalID::kNone) return;
    ctx.scale(width / 60);
    if (static_pos < Game::loadout_count && PETAL_DATA[last_id].count != 0)
        draw_loadout_background(ctx, last_id, (float) reload, 1, (float) health);
    else
        draw_loadout_background(ctx, last_id);
    // Unique level badge — top-right of the plate (name sits at the bottom).
    if (Craft::can_level_petal(last_id) && Craft::petal_level_get(Game::petal_levels, last_id) > 0
        && loadout_slot_is_leveled(static_pos, last_id)) {
        RenderContext c(&ctx);
        std::string lv = std::format("Lv.{}",
            Craft::petal_level_get(Game::petal_levels, last_id));
        ctx.set_fill(Chrome::kGold);
        ctx.set_stroke(0xff111111);
        ctx.set_text_size(11);
        ctx.set_line_width(1.25f);
        float tw = (float) ctx.get_text_size(lv.c_str());
        ctx.translate(28.f - tw, -26.f);
        ctx.stroke_text(lv.c_str());
        ctx.fill_text(lv.c_str());
    }
    // Sponge: banked damage pill in the top rim. Bound to the live slot id so
    // swap / unequip clears it immediately (do not follow last_id ghosts).
    if (!selected && static_pos < Game::loadout_count && Game::alive()) {
        PetalID::T const slot_id = Game::cached_loadout[static_pos];
        if (slot_id != PetalID::kNone && slot_id < PetalID::kNumPetals
            && slot_id == last_id
            && PETAL_DATA[slot_id].attributes.damage_delay_seconds > 0.f) {
            Entity const &player = Game::simulation.get_ent(Game::player_id);
            float pending = (float) player.get_sponge_pending();
            if (pending >= 1.f) {
                RenderContext c(&ctx);
                std::string txt = format_score(pending);
                ctx.set_text_size(9.5f);
                float tw = (float) ctx.get_text_size(txt.c_str());
                float pw = tw + 10.f;
                if (pw < 22.f) pw = 22.f;
                if (pw > 48.f) pw = 48.f;
                float const px = -pw * 0.5f;
                float const py = -29.5f;
                float const ph = 11.f;
                // Soft dark pill nested in the plate's top edge.
                ctx.set_fill(0xd9181014);
                ctx.begin_path();
                ctx.round_rect(px, py, pw, ph, 4.f);
                ctx.fill();
                ctx.set_stroke(0x88ff9a72);
                ctx.set_line_width(1.f);
                ctx.begin_path();
                ctx.round_rect(px + 0.5f, py + 0.5f, pw - 1.f, ph - 1.f, 3.5f);
                ctx.stroke();
                ctx.set_fill(0xffffe2c4);
                ctx.set_stroke(0xff2a1410);
                ctx.set_line_width(1.05f);
                ctx.translate(0.f, py + ph * 0.5f + 0.5f);
                ctx.stroke_text(txt.c_str());
                ctx.fill_text(txt.c_str());
            }
        }
    }
}

void UiLoadoutPetal::on_render_skip(Renderer &ctx) {
    if (fly_hint || selected) return;
    x = y = 0;
    if (Game::alive()) showed = 1;
    else showed = 0;
    curr_pos = 0;
    // Keep last_id when the cache still holds a petal (hidden bag widgets).
    petal_id = Game::cached_loadout[static_pos];
    if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals)
        last_id = PetalID::kNone;
    else
        last_id = petal_id;
}

void UiLoadoutPetal::poll_events(ScreenEvent const &event) {
    // Pure-bag widgets are ghosts. If they take the pointer, bag cells never
    // see the click and a second throw operates on a bag index (no-op).
    if (drag_from_bag) return;
    uint8_t n = Game::loadout_count;
    if (n > 0 && static_pos >= 2 * n && static_pos < n + MAX_SLOT_COUNT)
        return;
    // Open X/Z/C/V drawer covers part of the bar — clicks belong to the panel.
    float const px = Input::is_mobile ? event.x : Input::mouse_x;
    float const py = Input::is_mobile ? event.y : Input::mouse_y;
    if (!selected && Ui::game_panel_blocks_loadout(px, py)) return;
    if (Input::is_mobile) {
        // Capture the starting finger against the fixed slot, not the petal's
        // animated position. Keep ownership while it moves across the bag.
        if (selected || static_pos >= 2 * n
            || Game::cached_loadout[static_pos] == PetalID::kNone)
            return;
        auto touch = Input::touches.find(event.id);
        if (touch == Input::touches.end() || touch->second.saturated) return;
        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[static_to_dynamic(static_pos)];
        if (slot == nullptr || !slot->visible) return;
        float const hw = slot->width * Ui::scale * 0.55f;
        float const hh = slot->height * Ui::scale * 0.55f;
        if (fabsf(event.x - slot->screen_x) > hw
            || fabsf(event.y - slot->screen_y) > hh) return;
        touch_id = event.id;
        on_event(kMouseDown);
        touch->second.saturated = 1;
        return;
    }
    Element::poll_events(event);
}

void UiLoadoutPetal::on_event(uint8_t event) {
    if (Game::loadout_preview_bank() != 0 && static_pos < Game::loadout_count
        && (!Game::mobile_l_preset_pick || event != kMouseDown)) {
        rendering_tooltip = 0;
        return;
    }
    if (loadout_editable() && event == kMouseDown) {
        // Sticky L-pick (mobile long-press R): slot index → L bank, no drag.
        if (Game::mobile_l_preset_pick && static_pos < Game::loadout_count) {
            Game::try_mobile_l_preset_slot(static_pos);
            rendering_tooltip = 0;
            return;
        }
        if (!selected) {
            ++Ui::UiLoadout::num_petals_selected;
            press_x = Input::mouse_x;
            press_y = Input::mouse_y;
            auto touch_iter = Input::touches.find(touch_id);
            if (touch_iter != Input::touches.end()) {
                persistent_touch_id = touch_id;
                press_x = touch_iter->second.x;
                press_y = touch_iter->second.y;
            }
        }
        selected = 1;
        drag_from_bag = 0;
        Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
        // Hold-to-inspect on mobile (and press ahead of drag on desktop): tip
        // stays until the pointer moves past slop (see animate).
        if (last_id != PetalID::kNone) {
            rendering_tooltip = 1;
            tooltip = loadout_petal_tooltip(static_pos, last_id);
        } else {
            rendering_tooltip = 0;
        }
        return;
    }
    if (event == kFocusLost) {
        rendering_tooltip = 0;
        return;
    }
    // Desktop hover / mobile hold without drag
    if ((!selected || Input::is_mobile) && last_id != PetalID::kNone) {
        rendering_tooltip = 1;
        tooltip = loadout_petal_tooltip(static_pos, last_id);
    } else {
        rendering_tooltip = 0;
    }
}

class BagDragGhost final : public Element {
public:
    BagDragGhost() : Element(60, 60, { .no_animation = 1, .no_polling = 1 }) {
        style.layer = 1;
        style.should_render = [](){
            if (!Game::alive() && !Game::on_game_screen) return false;
            return Ui::bag_gesture_ghost_petal() != PetalID::kNone
                || bag_bounce_ghost_petal() != PetalID::kNone;
        };
        style.animate = [](Element *elt, Renderer &){
            PetalID::T bounce = bag_bounce_ghost_petal();
            if (bounce != PetalID::kNone) {
                float elapsed = (float) (Game::timestamp - bag_bounce_start);
                float u = elapsed / BAG_BOUNCE_MS;
                if (u < 0.f) u = 0.f;
                if (u > 1.f) u = 1.f;
                // Out to tip by ~45%, then ease back to origin.
                float tip = 0.45f;
                float a = u < tip ? (u / tip) : (1.f - (u - tip) / (1.f - tip));
                a = a * a * (3.f - 2.f * a); // smoothstep
                float sx = bag_bounce_ox + (bag_bounce_tx - bag_bounce_ox) * a;
                float sy = bag_bounce_oy + (bag_bounce_ty - bag_bounce_oy) * a;
                elt->x = (sx - Ui::window_width / 2) / Ui::scale;
                elt->y = (sy - Ui::window_height / 2) / Ui::scale;
                elt->width = lerp(elt->width, 56.f, Ui::lerp_amount);
                elt->height = lerp(elt->height, 56.f, Ui::lerp_amount);
                return;
            }
            float mx = Input::mouse_x;
            float my = Input::mouse_y;
            if (Input::is_mobile) {
                for (auto const &[id, touch] : Input::touches) {
                    (void) id;
                    mx = touch.x;
                    my = touch.y;
                    break;
                }
            }
            elt->x = (mx - Ui::window_width / 2) / Ui::scale;
            elt->y = (my - Ui::window_height / 2) / Ui::scale;
            uint8_t dyn = find_loadout_drop_target(mx, my);
            if (dyn != (uint8_t) -1 && dyn != 2 * MAX_SLOT_COUNT) {
                UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[dyn];
                if (slot != nullptr) {
                    elt->width = lerp(elt->width, slot->width, Ui::lerp_amount);
                    elt->height = lerp(elt->height, slot->height, Ui::lerp_amount);
                    return;
                }
            }
            elt->width = lerp(elt->width, 64.f, Ui::lerp_amount);
            elt->height = lerp(elt->height, 64.f, Ui::lerp_amount);
        };
    }
    void on_render(Renderer &ctx) override {
        PetalID::T id = Ui::bag_gesture_ghost_petal();
        if (id == PetalID::kNone) id = bag_bounce_ghost_petal();
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
        ctx.scale(width / 60.f);
        draw_loadout_background(ctx, id);
    }
};

Element *Ui::make_bag_drag_ghost() {
    return new BagDragGhost();
}

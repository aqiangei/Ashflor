#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>
#include <Client/Ui/Extern.hh>

#include <Helpers/Bits.hh>
#include <Shared/Recipe.hh>

#include <cmath>

using namespace Ui;

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
    if ((static_pos1 < primary && Recipe::is_bag_only(id2))
        || (static_pos2 < primary && Recipe::is_bag_only(id1)))
        return;
    // Widgets stay glued to their slot index. Remapping static_pos made the
    // next drag operate on a bag index and silently no-op.
    Game::cached_loadout[static_pos1] = id2;
    Game::cached_loadout[static_pos2] = id1;
    auto paint = [](UiLoadoutPetal *w, PetalID::T id) {
        if (w == nullptr) return;
        w->petal_id = id;
        w->last_id = id;
        w->no_change_ticks = 100;
    };
    paint(Ui::UiLoadout::petal_slots[static_pos1], id2);
    paint(Ui::UiLoadout::petal_slots[static_pos2], id1);
    Game::swap_petals(static_pos1, static_pos2);
}

// Gardn swap is a no-op when both sides are the same petal. Skip those slots
// or a click on Basic looks like it did nothing.
static uint8_t find_auto_equip_target(PetalID::T incoming) {
    uint8_t n = Game::loadout_count;
    auto open = [&](uint8_t i) {
        PetalID::T id = Game::cached_loadout[i];
        return id != incoming;
    };
    for (uint8_t i = 0; i < n; ++i) {
        if (Game::cached_loadout[i] == PetalID::kNone && open(i))
            return i;
    }
    for (uint8_t i = 0; i < n; ++i) {
        if (Game::cached_loadout[i] == PetalID::kBasic && open(i))
            return i;
    }
    for (uint8_t i = 0; i < n; ++i) {
        uint8_t s = (uint8_t) (n + i);
        if (Game::cached_loadout[s] == PetalID::kNone && open(s))
            return s;
    }
    // Full bar: still equip by swapping with the first different primary.
    for (uint8_t i = 0; i < n; ++i) {
        if (open(i)) return i;
    }
    return (uint8_t) -1;
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
    if (target == (uint8_t) -1 || target == bag_pos) return 0;

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

static void apply_unequip_fly(UiLoadoutPetal *at_dest, float from_sx, float from_sy) {
    if (at_dest == nullptr) return;
    PetalID::T id = Game::cached_loadout[at_dest->static_pos];
    if (id != PetalID::kNone) {
        at_dest->petal_id = id;
        at_dest->last_id = id;
    }
    at_dest->x = (from_sx - Ui::window_width / 2) / Ui::scale;
    at_dest->y = (from_sy - Ui::window_height / 2) / Ui::scale;
    at_dest->fly_hint = 1;
    at_dest->style.layer = 1;
    at_dest->showed = 1;
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
    uint8_t write = n;
    uint8_t end = (uint8_t) (n + MAX_SLOT_COUNT);
    if (end > 2 * MAX_SLOT_COUNT) end = 2 * MAX_SLOT_COUNT;
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
static float bag_gesture_x = 0;
static float bag_gesture_y = 0;
static uint32_t bag_gesture_touch = (uint32_t) -1;
static double bag_gesture_started = 0;

void Ui::cancel_petal_drags() {
    bag_gesture_active = 0;
    bag_gesture_dragging = 0;
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

static void swap_paired_with_fly(uint8_t from_pos, uint8_t to_pos) {
    UiLoadoutPetal *from = Ui::UiLoadout::petal_slots[from_pos];
    float sx = from != nullptr ? from->screen_x : Ui::window_width / 2;
    float sy = from != nullptr ? from->screen_y : Ui::window_height;
    ui_swap_petals(from_pos, to_pos);
    apply_unequip_fly(Ui::UiLoadout::petal_slots[to_pos], sx, sy);
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
        swap_paired_with_fly(static_pos, sec);
        return;
    }
    if (static_pos < 2 * n) {
        uint8_t pri = (uint8_t) (static_pos - n);
        if (Game::cached_loadout[pri] == Game::cached_loadout[static_pos]) {
            ui_unequip_to_backpack(static_pos);
            return;
        }
        swap_paired_with_fly(static_pos, pri);
        return;
    }
}

static uint8_t dynamic_to_static(uint8_t dynamic_pos) {
    if (dynamic_pos >= MAX_SLOT_COUNT)
        dynamic_pos -= (MAX_SLOT_COUNT - Game::loadout_count);
    return dynamic_pos; 
}


uint8_t Ui::find_loadout_drop_target(float x, float y) {
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

uint8_t Ui::loadout_bar_hit(float x, float y) {
    float const pad = Input::is_mobile ? 48.f : 20.f;
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT + 1; ++i) {
        UiLoadoutSlot *slot = Ui::UiLoadout::petal_backgrounds[i];
        if (slot == nullptr || !slot->visible) continue;
        float dist = std::max(fabsf(x - slot->screen_x), fabsf(y - slot->screen_y));
        if (dist < (slot->width + pad) * Ui::scale / 2)
            return 1;
    }
    return 0;
}

void Ui::begin_bag_gesture(uint8_t static_pos, float x, float y, uint32_t touch_id) {
    if (static_pos >= 2 * MAX_SLOT_COUNT) return;
    if (Game::cached_loadout[static_pos] == PetalID::kNone) return;
    // Mobile delivers kMouseDown every held frame — do not reset mid-gesture.
    if (bag_gesture_active
        && bag_gesture_pos == static_pos
        && bag_gesture_touch == touch_id)
        return;
    bag_gesture_active = 1;
    bag_gesture_dragging = 0;
    bag_gesture_pos = static_pos;
    bag_gesture_x = x;
    bag_gesture_y = y;
    bag_gesture_touch = touch_id;
    bag_gesture_started = Game::timestamp;
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
    float drag_slop = Input::is_mobile ? Input::pointer_slop() * 1.6f : Input::pointer_slop();
    if (!bag_gesture_dragging
        && dist >= Input::pointer_slop()
        && (over_slot || dist >= drag_slop)) {
        bag_gesture_dragging = 1;
        begin_petal_drag(bag_gesture_pos, bag_gesture_touch);
    }
    if (!released) return;
    uint8_t pos = bag_gesture_pos;
    uint8_t was_drag = bag_gesture_dragging;
    bag_gesture_active = 0;
    bag_gesture_dragging = 0;
    UiLoadoutPetal *p = Ui::UiLoadout::petal_slots[pos];
    if (p != nullptr && p->selected) {
        --Ui::UiLoadout::num_petals_selected;
        p->selected = 0;
        p->drag_from_bag = 0;
        p->style.layer = 0;
        p->persistent_touch_id = (uint32_t) -1;
    }
    if (Game::cached_loadout[pos] == PetalID::kNone) return;
    // A deliberate long hold only inspects the petal. A short tap equips.
    if (!was_drag) {
        if (Input::is_mobile && Game::timestamp - bag_gesture_started >= 450)
            return;
        ui_auto_equip_from_bag(pos, bag_gesture_x, bag_gesture_y);
        return;
    }
    // Releasing a drag away from a slot returns it to the bag.
    if (!over_slot) return;
    uint8_t dyn = find_loadout_drop_target(mx, my);
    if (dyn == 2 * MAX_SLOT_COUNT) {
        ui_delete_petal(pos);
        return;
    }
    if (dyn != (uint8_t) -1) {
        uint8_t dest = loadout_dynamic_to_static(dyn);
        if (dest != pos)
            ui_swap_petals(dest, pos);
    }
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
    style.should_render = [&](){
        if (!Game::alive()) return false;
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
        }
        else
            reload = 0;
        if ((no_change_ticks == 0 && petal_id == PetalID::kNone) || last_id == PetalID::kNone) return false;
        return true;
    };
    style.animate = [&](Element *elt, Renderer &ctx) {
        float lerp_amt = Ui::lerp_amount * 0.75;
        reload.step(lerp_amt);
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
        if (selected && Game::alive()) {
            style.layer = 1;
            float drag_dist = std::hypot(mouse_x - press_x, mouse_y - press_y);
            float const kClickSlop = Input::pointer_slop();
            uint8_t is_click = drag_dist < kClickSlop;
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
        } else if (Ui::UiLoadout::selected_with_keys + Game::loadout_count == static_pos && Game::alive()) {
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
                lerp_amt = Ui::lerp_amount * 1.6f; // snappier bag↔slot path
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
    if (last_id == PetalID::kNone) return;
    ctx.scale(width / 60);
    if (static_pos < Game::loadout_count && PETAL_DATA[last_id].count != 0)
        draw_loadout_background(ctx, last_id, (float) reload);
    else
        draw_loadout_background(ctx, last_id);
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
    if (Game::alive() && event == kMouseDown) {
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
        rendering_tooltip = 0;
        return;
    }
    if (event != kFocusLost && (!selected || Input::is_mobile) && last_id != PetalID::kNone) {
        rendering_tooltip = 1;
        tooltip = Ui::UiLoadout::petal_tooltips[last_id];
    } else 
        rendering_tooltip = 0;
}

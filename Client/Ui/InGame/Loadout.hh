#pragma once

#include <Client/Ui/Choose.hh>
#include <Client/Ui/Element.hh>

#include <Client/StaticData.hh>

namespace Ui {
    class UiLoadoutSlot : public Element {
    public:
        uint8_t position;
        UiLoadoutSlot(uint8_t);

        virtual void on_render(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };

    class UiDeleteSlot final : public UiLoadoutSlot {
    public:
        UiDeleteSlot();
        LerpFloat delete_text_opacity;
        virtual void on_render(Renderer &) override;
    };

    class InputFreeze final : public Container {
    public:
        float render_width;
        float render_height;
        InputFreeze();
        virtual void on_render(Renderer &) override;
        virtual void on_render_skip(Renderer &) override;
        virtual void on_event(uint8_t) override;
        virtual void poll_events(ScreenEvent const &) override;
    };

    class UiLoadoutPetal final : public Element {
    public:
        LerpFloat reload;
        uint32_t persistent_touch_id;
        float press_x;
        float press_y;
        uint8_t static_pos;
        uint8_t curr_pos;
        uint8_t no_change_ticks;
        uint8_t selected;
        uint8_t drag_from_bag;
        uint8_t fly_hint; // bag click → slot fly animation
        PetalID::T petal_id;
        PetalID::T last_id;
        UiLoadoutPetal(uint8_t);

        virtual void on_render(Renderer &) override;
        virtual void on_render_skip(Renderer &) override;
        virtual void on_event(uint8_t) override;
        virtual void poll_events(ScreenEvent const &) override;
    };

    namespace UiLoadout {
        extern UiLoadoutPetal *petal_slots[2 * MAX_SLOT_COUNT];
        extern UiLoadoutSlot *petal_backgrounds[2 * MAX_SLOT_COUNT + 1];
        extern Element *petal_tooltips[PetalID::kNumPetals];
        extern uint8_t selected_with_keys;
        extern double last_key_select;
        extern uint32_t num_petals_selected;
    };

    Element *make_loadout_backgrounds();
    Element *make_rotation_wheel();
    void make_petal_tooltips();

    void forward_secondary_select();
    void backward_secondary_select();
    void ui_delete_petal(uint8_t); // drop on ground (wire: kPetalDelete)
    void ui_swap_petals(uint8_t, uint8_t);
    // Primary ↔ paired secondary with cross-fly animation
    void ui_swap_paired_slots(uint8_t from_pos, uint8_t to_pos);
    // Bag click: empty/Basic primary L→R, then empty secondary; 0 if full (bounce)
    uint8_t ui_auto_equip_from_bag(uint8_t bag_pos, float screen_x, float screen_y);
    // Brief fly toward the loadout bar then snap back (no empty slot / refuse drop).
    void play_bag_equip_bounce(PetalID::T id, float from_sx, float from_sy,
                               float tip_sx = -1.f, float tip_sy = -1.f);
    // Click primary↔paired secondary (equip out / retract)
    void ui_toggle_petal_slot(uint8_t static_pos);
    // Unequip → backpack stacks (never secondary)
    uint8_t ui_unequip_to_backpack(uint8_t static_pos, float x = -1, float y = -1);
    // Visible inventory cell under a screen-space drop, or 0xFF.
    uint8_t bag_drop_target(float x, float y);
    // Pack inventory cells left (no gaps)
    void ui_compact_backpack();
    // Abort in-flight petal drag without swapping (blur / visibility)
    void cancel_petal_drags();
    // Returns dynamic loadout slot index under (x,y), or 0xFF if none
    uint8_t find_loadout_drop_target(float x, float y);
    uint8_t loadout_dynamic_to_static(uint8_t dynamic_pos);
    void begin_petal_drag(uint8_t static_pos, uint32_t touch_id);
    // Inventory cell press. finish_bag_gesture resolves tap, hold, and drop.
    // static_pos 0xFF + bank_id → stack-only (no slot copy) drag/tap.
    void begin_bag_gesture(uint8_t static_pos, float x, float y, uint32_t touch_id,
                           PetalID::T bank_id = PetalID::kNone);
    void finish_bag_gesture();
    // Petal drawn under the cursor while dragging a bank stack, else kNone.
    PetalID::T bag_gesture_ghost_petal();
    // Floating cursor ghost for bank-stack bag drags.
    Element *make_bag_drag_ghost();
    // True while a bag tap/drag is in progress (blocks scroll + outside-close)
    uint8_t bag_gesture_busy();
    // True only after a petal/bag press has moved past slop (real drag).
    // Soft presses still allow ScrollContainer to take over for scrolling.
    uint8_t petal_drag_committed();
    // True if (x,y) hits the bottom loadout bar (primary/secondary/trash)
    uint8_t loadout_bar_hit(float x, float y);
    // True while a loadout petal is held over the open inventory panel
    uint8_t loadout_drag_over_bag(float x, float y);
}

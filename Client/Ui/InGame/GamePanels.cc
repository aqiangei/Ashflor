#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Button.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>
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
#include <Shared/StaticDefinitions.hh>

#include <cmath>
#include <format>

using namespace Ui;

static bool petal_equipped(PetalID::T id) {
    if (id == PetalID::kNone) return false;
    for (uint32_t i = 0; i < Game::loadout_count; ++i)
        if (Game::cached_loadout[i] == id) return true;
    return false;
}

// Auto-equip bag petal into primary (paired secondary, else first Basic/empty)
static void auto_equip_bag_petal(uint8_t bag_pos) {
    if (!Game::alive()) return;
    if (bag_pos < Game::loadout_count || bag_pos >= Game::loadout_count + MAX_SLOT_COUNT) return;
    if (Game::cached_loadout[bag_pos] == PetalID::kNone) return;
    uint8_t sec_i = (uint8_t) (bag_pos - Game::loadout_count);
    if (sec_i < Game::loadout_count) {
        ui_swap_petals(sec_i, bag_pos);
        return;
    }
    for (uint8_t i = 0; i < Game::loadout_count; ++i) {
        PetalID::T id = Game::cached_loadout[i];
        if (id == PetalID::kNone || id == PetalID::kBasic) {
            ui_swap_petals(i, bag_pos);
            return;
        }
    }
    ui_swap_petals(0, bag_pos);
}

// Bag grid cell — click = equip to bottom bar; drag = drop onto bottom equip slots
class InventoryBagSlot final : public Element {
    uint8_t bag_index; // 0 .. MAX_SLOT_COUNT-1 → static loadout_count + bag_index
    uint8_t dragging;
    uint32_t drag_touch;
    float press_x, press_y;
public:
    InventoryBagSlot(uint8_t idx)
        : Element(64, 72, { .no_animation = 1 }),
          bag_index(idx), dragging(0), drag_touch((uint32_t)-1),
          press_x(0), press_y(0) {
        style.should_render = [idx](){
            return Game::alive() && idx < MAX_SLOT_COUNT;
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
            uint8_t static_pos = (uint8_t) (Game::loadout_count + bag_index);
            if (static_pos >= 2 * MAX_SLOT_COUNT) { dragging = 0; return; }
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
                    auto_equip_bag_petal(static_pos);
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
        return (uint8_t) (Game::loadout_count + bag_index);
    }
    PetalID::T id() const {
        uint8_t sp = slot_static();
        if (sp >= 2 * MAX_SLOT_COUNT) return PetalID::kNone;
        return Game::cached_loadout[sp];
    }
    void on_render(Renderer &ctx) override {
        PetalID::T pid = id();
        ctx.set_fill(0xff3ecf6e);
        ctx.set_stroke(0xff2a9a4a);
        ctx.set_line_width(2);
        ctx.begin_path();
        ctx.round_rect(-width / 2, -height / 2, width, height, 6);
        ctx.fill();
        ctx.stroke();
        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) return;
        if (dragging) ctx.set_global_alpha(0.4f);
        {
            RenderContext c(&ctx);
            ctx.translate(0, -8);
            ctx.scale(48.f / 60.f);
            draw_loadout_background(ctx, pid);
        }
        ctx.translate(0, height / 2 - 11);
        ctx.set_fill(0xffffffff);
        ctx.draw_text(PETAL_DATA[pid].name, { .size = 10 });
        ctx.translate(0, -(height / 2 - 11));
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

static Element *make_bag_grid() {
    // 4-column grid of bag slots only (no equipment rows inside panel)
    Element *grid = new Ui::VContainer({}, 0, 6, {
        .h_justify = Style::Left,
        .v_justify = Style::Top,
        .no_animation = 1
    });
    for (uint8_t row = 0; row < 2; ++row) {
        Element *h = new Ui::HContainer({}, 0, 6, {
            .h_justify = Style::Left,
            .v_justify = Style::Top,
            .no_animation = 1
        });
        for (uint8_t col = 0; col < 4; ++col) {
            uint8_t idx = (uint8_t) (row * 4 + col);
            if (idx < MAX_SLOT_COUNT)
                h->add_child(new InventoryBagSlot(idx));
        }
        grid->add_child(h);
    }
    return grid;
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

#define MAKE_RARITY_SELECT(N) \
static void select_rarity_##N(Element *, uint8_t e) { \
    if (e == Ui::kClick) Game::craft_selected_rarity = N; \
} \
static bool rarity_##N##_active() { return Game::craft_selected_rarity == N; }

MAKE_RARITY_SELECT(0)
MAKE_RARITY_SELECT(1)
MAKE_RARITY_SELECT(2)
MAKE_RARITY_SELECT(3)
MAKE_RARITY_SELECT(4)
MAKE_RARITY_SELECT(5)

static void on_gamble(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::craft_petals(CraftMode::kCraftGamble);
}
static void on_exchange(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::craft_petals(CraftMode::kCraftExchange);
}
static void on_buy_rarity(Element *, uint8_t e) {
    if (e == Ui::kClick) Game::craft_buy_rarity(Game::craft_selected_rarity);
}
#define MAKE_SPECIAL_BUY(NAME, ID) \
static void on_buy_##NAME(Element *, uint8_t e) { \
    if (e == Ui::kClick) Game::craft_buy_special(PetalID::ID); \
}
MAKE_SPECIAL_BUY(antennae, kAntennae)
MAKE_SPECIAL_BUY(accelerator, kAccelerator)
MAKE_SPECIAL_BUY(magnet, kMagnet)

static Element *make_rarity_row(uint8_t rarity, void (*on_click)(Element *, uint8_t), bool (*active)()) {
    return new Ui::HContainer({
        new Ui::Button(110, 28,
            new Ui::DynamicText(12, [rarity](){
                return std::format("{} ×{}", I18n::rarity_name(rarity), Game::craft_bank[rarity]);
            }),
            on_click, active,
            { .fill = RARITY_COLORS[rarity], .line_width = 3, .round_radius = 4 }
        ),
        new Ui::DynamicText(11, [rarity](){
            if (!Craft::can_upgrade(rarity)) return std::string{};
            float p = Craft::success_chance(rarity) * 100.f;
            return std::format("{:.0f}% →{}", p, I18n::rarity_name(rarity + 1));
        }, { .fill = 0xffdddddd })
    }, 0, 6, { .h_justify = Style::Left });
}

Element *Ui::make_game_side_buttons() {
    // Vertical column against left wall (bottom)
    Element *elt = new Ui::VContainer({
        new Ui::Button(44, 44,
            new Ui::DynamicText(14, [](){ return I18n::use_chinese ? "包" : "B"; }),
            on_inventory_btn, inventory_active,
            { .fill = 0xff5a9fdb, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            new Ui::DynamicText(14, [](){ return I18n::use_chinese ? "天" : "Z"; }),
            on_talents_btn, talents_active,
            { .fill = 0xff9b6bdb, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            new Ui::DynamicText(14, [](){ return I18n::use_chinese ? "合" : "C"; }),
            on_craft_btn, craft_active,
            { .fill = 0xff1dd129, .line_width = 3, .round_radius = 8 }),
        new Ui::Button(44, 44,
            new Ui::DynamicText(14, [](){ return I18n::use_chinese ? "特" : "V"; }),
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
    // Large backpack panel (no equipment slots inside) — drag/click to bottom bar
    Element *header = new Ui::HFlexContainer(
        new Ui::DynamicText(22, [](){ return I18n::tr(I18n::kPanelInventory); }, {
            .fill = 0xffffffff, .h_justify = Style::Left
        }),
        new Ui::Button(32, 32,
            new Ui::DynamicText(16, [](){ return std::string{"X"}; }),
            on_close_inventory, nullptr,
            { .fill = 0xffe03a3a, .line_width = 3, .round_radius = 6 }),
        0, 8, { .h_justify = Style::Left }
    );
    // Give header a stable width so title left / X right
    header->width = 300;

    Element *body = new Ui::VContainer({
        header,
        new Ui::DynamicText(13, [](){
            return I18n::use_chinese
                ? std::format("钱币 {}　点击装备 · 拖到下方装备栏", Game::coins)
                : std::format("Coins {}  click=equip · drag→bottom bar", Game::coins);
        }, { .fill = 0xffffd24a, .h_justify = Style::Left }),
        make_bag_grid(),
        new Ui::DynamicText(11, [](){
            std::string s = I18n::use_chinese ? "材料 " : "Mats ";
            for (uint8_t r = 0; r < RarityID::kNumRarities - 1; ++r) {
                if (r) s += " ";
                s += std::format("{}×{}", I18n::rarity_name(r), Game::craft_bank[r]);
            }
            return s;
        }, { .fill = 0xffdddddd, .h_justify = Style::Left })
    }, 14, 8, {
        .fill = 0xe0286a9e,
        .line_width = 5,
        .round_radius = 10,
        .animate = [](Element *elt, Renderer &ctx){
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

Element *Ui::make_talents_panel() {
    return make_drawer({
        new Ui::DynamicText(20, [](){ return I18n::tr(I18n::kPanelTalents); }, {
            .fill = 0xffffffff, .h_justify = Style::Left
        }),
        new Ui::DynamicText(13, [](){ return I18n::tr(I18n::kPanelTalentsHint); }, {
            .fill = 0xffcccccc, .h_justify = Style::Left
        }),
        new Ui::DynamicText(12, [](){
            return I18n::use_chinese
                ? "天赋系统即将开放。先用 V 面板的触角 / 加速器 / 磁铁。"
                : "Talents coming soon. Use Antennae / Accelerator / Magnet (V).";
        }, { .fill = 0xffdddddd, .h_justify = Style::Left })
    }, Game::kPanelTalents);
}

Element *Ui::make_craft_panel() {
    return make_drawer({
        new Ui::DynamicText(20, [](){ return I18n::tr(I18n::kPanelCraft); }, {
            .fill = 0xffffffff, .h_justify = Style::Left
        }),
        new Ui::DynamicText(14, [](){
            return I18n::use_chinese
                ? std::format("钱币：{}", Game::coins)
                : std::format("Coins: {}", Game::coins);
        }, { .fill = 0xffffd24a, .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "击杀怪物掉钱（稀有度越高越多）。材料合成 / 钱币购买 → 进背包"
                : "Kill mobs for coins (higher rarity = more). Craft / buy → backpack";
        }, { .fill = 0xffbbbbbb, .h_justify = Style::Left }),
        make_rarity_row(0, select_rarity_0, rarity_0_active),
        make_rarity_row(1, select_rarity_1, rarity_1_active),
        make_rarity_row(2, select_rarity_2, rarity_2_active),
        make_rarity_row(3, select_rarity_3, rarity_3_active),
        make_rarity_row(4, select_rarity_4, rarity_4_active),
        make_rarity_row(5, select_rarity_5, rarity_5_active),
        new Ui::DynamicText(12, [](){
            uint8_t r = Game::craft_selected_rarity;
            if (!Craft::can_upgrade(r)) return std::string{};
            float p = Craft::success_chance(r) * 100.f;
            uint32_t ev = Craft::exchange_cost(r);
            return I18n::use_chinese
                ? std::format("材料：{} → {}　成功率 {:.0f}% · 直兑≈{}",
                    I18n::rarity_name(r), I18n::rarity_name(r + 1), p, ev)
                : std::format("Mats {} → {}  {:.0f}%  EV≈{}",
                    I18n::rarity_name(r), I18n::rarity_name(r + 1), p, ev);
        }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::HContainer({
            new Ui::Button(100, 30,
                new Ui::DynamicText(12, [](){ return I18n::use_chinese ? "五合赌" : "Gamble"; }),
                on_gamble, nullptr,
                { .fill = 0xff1dd129, .line_width = 3, .round_radius = 4 }),
            new Ui::Button(100, 30,
                new Ui::DynamicText(12, [](){ return I18n::use_chinese ? "材料直兑" : "Exchange"; }),
                on_exchange, nullptr,
                { .fill = 0xff5a9fdb, .line_width = 3, .round_radius = 4 }),
            new Ui::Button(130, 30,
                new Ui::DynamicText(12, [](){
                    uint8_t r = Game::craft_selected_rarity;
                    uint32_t c = Craft::buy_rarity_cost(r);
                    return I18n::use_chinese
                        ? std::format("买{}·{}币", I18n::rarity_name(r), c)
                        : std::format("Buy {}·{}", I18n::rarity_name(r), c);
                }),
                on_buy_rarity, nullptr,
                { .fill = 0xffe0a030, .line_width = 3, .round_radius = 4 })
        }, 0, 6, { .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            return I18n::use_chinese
                ? "钱币购买：随机该稀有度花瓣，直接进背包"
                : "Coin buy: random petal of that rarity → backpack";
        }, { .fill = 0xffaaddff, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [](){ return Game::craft_status; }, {
            .fill = 0xffffe0a0, .h_justify = Style::Left
        })
    }, Game::kPanelCraft);
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

#include <Client/Ui/Extern.hh>

#include <Client/Ui/Element.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>

#include <Helpers/Bits.hh>

#include <cmath>

namespace Ui {
    namespace Panel {
        Element *settings = nullptr;
        Element *petal_gallery = nullptr;
        Element *petal_gallery_alt = nullptr;
        Element *mob_gallery = nullptr;
        Element *mob_gallery_alt = nullptr;
        Element *leaderboard = nullptr;
        Element *leaderboard_alt = nullptr;
        Element *changelog = nullptr;
    }
    double dt = 0;
    double window_width = 1920;
    double window_height = 1080;
    double scale = 1;
    double lerp_amount = 0.05;
    Element *focused = nullptr;
    Element *pressed = nullptr;
    uint8_t panel_open = Panel::kNone;

    Element *pause_menu_root = nullptr;
    Element *leave_confirm_root = nullptr;
    Element *game_topbar_root = nullptr;
    Element *panel_buttons_root = nullptr;
    Element *game_side_buttons_root = nullptr;
    Element *chat_panel_root = nullptr;
    Element *game_panel_roots[8] = {};

    void toggle_panel(uint8_t panel) {
        if (panel_open == panel) {
            panel_open = Panel::kNone;
            return;
        }
        panel_open = panel;
        Element *pg = nullptr;
        switch (panel) {
            case Panel::kSettings: pg = Panel::settings; break;
            case Panel::kPetals:
                pg = (Game::alive() && Panel::petal_gallery_alt)
                    ? Panel::petal_gallery_alt : Panel::petal_gallery;
                break;
            case Panel::kMobs:
                pg = (Game::alive() && Panel::mob_gallery_alt)
                    ? Panel::mob_gallery_alt : Panel::mob_gallery;
                break;
            case Panel::kLeaderboard:
                pg = (Game::alive() && Panel::leaderboard_alt)
                    ? Panel::leaderboard_alt : Panel::leaderboard;
                break;
            case Panel::kChangelog: pg = Panel::changelog; break;
            default: break;
        }
        if (pg == nullptr) return;
        pg->x = 12;
        // Below status bar in-game; under title icons on home
        pg->y = Game::alive() ? 170.f : 84.f;
    }

    static bool hit_element(Element *elt, float x, float y) {
        if (elt == nullptr || !elt->visible) return false;
        return std::abs(x - elt->screen_x) <= elt->width * Ui::scale / 2
            && std::abs(y - elt->screen_y) <= elt->height * Ui::scale / 2;
    }

    static uint8_t read_outside_press(float &x, float &y) {
        x = Input::mouse_x;
        y = Input::mouse_y;
        uint8_t pressed = BitMath::at(Input::mouse_buttons_pressed, Input::LeftMouse);
        if (Input::is_mobile) {
            pressed = 0;
            for (auto const &[id, touch] : Input::touches) {
                if (!touch.saturated) {
                    pressed = 1;
                    x = touch.x;
                    y = touch.y;
                    break;
                }
            }
        }
        return pressed;
    }

    static Element *open_info_panel_primary() {
        switch (panel_open) {
            case Panel::kSettings: return Panel::settings;
            case Panel::kPetals: return Panel::petal_gallery;
            case Panel::kMobs: return Panel::mob_gallery;
            case Panel::kLeaderboard: return Panel::leaderboard;
            case Panel::kChangelog: return Panel::changelog;
            default: return nullptr;
        }
    }

    static Element *open_info_panel_alt() {
        switch (panel_open) {
            case Panel::kPetals: return Panel::petal_gallery_alt;
            case Panel::kMobs: return Panel::mob_gallery_alt;
            case Panel::kLeaderboard: return Panel::leaderboard_alt;
            default: return nullptr;
        }
    }

    void try_close_popups_outside_click() {
        float x = 0, y = 0;
        if (!read_outside_press(x, y)) return;

        // Leave confirm (modal): click outside dialog dismisses
        if (Game::leave_confirm_open) {
            if (!hit_element(leave_confirm_root, x, y)
                && !hit_element(game_topbar_root, x, y))
                Game::leave_confirm_open = 0;
            return;
        }

        // Pause menu
        if (Game::pause_menu_open) {
            if (!hit_element(pause_menu_root, x, y)
                && !hit_element(game_topbar_root, x, y))
                Game::pause_menu_open = 0;
        }

        // Chat panel: tap/click outside collapses (same as Esc). Keep open when
        // pressing send / history / channel label inside the panel.
        if (Game::chat_open) {
            if (!hit_element(chat_panel_root, x, y))
                Game::close_chat();
        }

        // Settings / galleries / leaderboard / changelog
        if (panel_open != Panel::kNone) {
            if (!hit_element(open_info_panel_primary(), x, y)
                && !hit_element(open_info_panel_alt(), x, y)
                && !hit_element(panel_buttons_root, x, y)
                && !hit_element(game_topbar_root, x, y))
                panel_open = Panel::kNone;
        }

        // X / Z / C / V drawers. Mobile: touching the loadout bar to throw a
        // petal into the open bag must NOT close the panel mid-gesture.
        if (Game::game_panel != Game::kPanelNone) {
            if (Ui::bag_gesture_busy() || Ui::UiLoadout::num_petals_selected != 0)
                return;
            Element *panel = nullptr;
            if (Game::game_panel < Game::kPanelCount)
                panel = game_panel_roots[Game::game_panel];
            if (!hit_element(panel, x, y)
                && !hit_element(game_side_buttons_root, x, y)
                && !hit_element(game_topbar_root, x, y)
                && !Ui::loadout_bar_hit(x, y))
                Game::close_game_panel();
        }
    }
}

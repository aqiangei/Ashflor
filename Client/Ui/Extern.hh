#pragma once

#include <cstdint>

namespace Ui {
    class Element;
    namespace Panel {
        enum {
            kNone,
            kSettings,
            kPetals,
            kMobs,
            kLeaderboard,
            kChangelog
        };
        extern Element *settings;
        extern Element *petal_gallery;
        extern Element *mob_gallery;
        extern Element *mob_gallery_alt; // in-game copy
        extern Element *leaderboard;
        extern Element *leaderboard_alt; // in-game copy
        extern Element *changelog;
    }
    extern double dt;
    extern double window_width;
    extern double window_height;
    extern double scale;
    extern double lerp_amount;
    extern Element *focused;
    extern Element *pressed;
    extern uint8_t panel_open;

    // Roots used for click-outside dismiss
    extern Element *pause_menu_root;
    extern Element *leave_confirm_root;
    extern Element *game_topbar_root;
    extern Element *panel_buttons_root;
    extern Element *game_side_buttons_root;
    extern Element *game_panel_roots[7];

    // Toggle title/in-game info panels (leaderboard, galleries, …)
    void toggle_panel(uint8_t panel);
    // Close any open popup/drawer when pressing outside it
    void try_close_popups_outside_click();
};

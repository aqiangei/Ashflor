#pragma once

#include <Client/Render/Renderer.hh>

namespace Ui {
    class Element;

    // Shared 24×24 white HUD glyphs (top-left + bottom-left buttons).
    // Drawing origin is top-left of the 24×24 box; call via make_hud_icon.
    void hud_icon_settings(Renderer &);
    void hud_icon_boost(Renderer &);
    void hud_icon_guild(Renderer &);
    void hud_icon_mob_gallery(Renderer &);
    void hud_icon_petal_gallery(Renderer &);
    void hud_icon_leaderboard(Renderer &);
    void hud_icon_exit(Renderer &);
    void hud_icon_inventory(Renderer &);
    void hud_icon_talents(Renderer &);
    void hud_icon_craft(Renderer &);
    void hud_icon_special_craft(Renderer &);
    void hud_icon_shop(Renderer &);

    Element *make_hud_icon(void (*draw)(Renderer &), float size = 26.f);
}

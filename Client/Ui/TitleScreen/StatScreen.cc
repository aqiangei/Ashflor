#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Game.hh>
#include <Client/Ui/Element.hh>

using namespace Ui;

// Death score/level moved under the top-right life summary (DeathSummary.cc).
// Loadout petal row removed — it overlapped bottom-left game controls.
Element *Ui::make_stat_screen() {
    Element *elt = new Element(0, 0, {
        .should_render = [](){ return 0; },
        .no_animation = 1
    });
    return elt;
}

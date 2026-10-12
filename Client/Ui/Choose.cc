#include <Client/Ui/Choose.hh>

#include <Client/Ui/Extern.hh>

#include <Helpers/Macros.hh>

using namespace Ui;

uint8_t no_show() { return 0; }
uint8_t do_show() { return 1; }

Choose::Choose(Element *l, Element *r, std::function<uint8_t(void)> const &c, Style s) : 
    Container({l, r},0,0,s), chooser(std::move(c)), choose_showing(c())
{
    l->showed = 1;
    r->showed = 1;
    l->refactor();
    r->refactor();
    // Hide the inactive branch immediately (avoids layout/DOM overlap)
    children[1 - choose_showing]->animation.set(0);
    children[1 - choose_showing]->style.should_render = no_show;
    children[choose_showing]->style.should_render = do_show;
    children[choose_showing]->animation.set(1);
    refactor();
}

void Choose::on_render(Renderer &ctx) {
    DEBUG_ONLY(assert(choose_showing < children.size());)
    // Only one branch — never draw both (was causing login/register overlap)
    children[choose_showing]->render(ctx);
    // Still skip the inactive branch so TextInput DOM / children hide
    children[1 - choose_showing]->on_render_skip(ctx);
}

void Choose::refactor() {
    uint8_t to_show = chooser();
    if (to_show != choose_showing) {
        // Snap old branch out so TextInput DOM / layout cannot linger
        children[choose_showing]->style.should_render = no_show;
        children[choose_showing]->animation.set(0);
        children[choose_showing]->visible = 0;
        choose_showing = to_show;
        children[choose_showing]->style.should_render = do_show;
        if (children[choose_showing]->style.no_animation)
            children[choose_showing]->animation.set(1);
    } else {
        children[to_show]->style.should_render = do_show;
        children[1 - to_show]->style.should_render = no_show;
    }
    Element *rendering = children[choose_showing];
    rendering->refactor();
    width = rendering->width;
    height = rendering->height;
}

void Choose::poll_events(ScreenEvent const &event) {
    if (style.no_polling) return;
    Element::poll_events(event);
    if (Ui::focused != this)
        return;
    if (children[choose_showing]->visible)
        children[choose_showing]->poll_events(event);
}

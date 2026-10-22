#include <Client/Ui/Container.hh>

#include <Client/Ui/Extern.hh>

#include <cmath>
#include <iostream>

using namespace Ui;

Container::Container(std::vector<Element *> const &elts, float w, float h, Style s) :
    Element(w, h, s)
{
    for (Element *elt : elts) 
        if (elt != nullptr) children.push_back(elt); 
    for (Element *elt : children)
        elt->parent = this;
}

void Container::on_render(Renderer &ctx) {
    Element::on_render(ctx);
    for (uint32_t layer = 0; layer < 2; ++layer) {
        for (Element *elt : children) {
            if (elt->style.layer != layer) continue;
            RenderContext context(&ctx);
            ctx.translate(elt->x, elt->y);
            ctx.translate(elt->style.h_justify * (width - elt->width) / 2, elt->style.v_justify * (height - elt->height) / 2);
            elt->render(ctx);
        }
    }
}

void Container::poll_events(ScreenEvent const &event) {
    if (style.no_polling) return;
    Element::poll_events(event);
    if (Ui::focused != this)
        return;
    for (Element *elt : children)
        if (elt->visible) elt->poll_events(event);
}

HContainer::HContainer(std::vector<Element *> const &elts, float opad, float ipad, Style s) :
    Container(elts, 0, 0, s), outer_pad(opad), inner_pad(ipad)
{
    for (Element *elt : children)
        elt->style.h_justify = Style::Left;
    refactor();
}

void HContainer::refactor() {
    float x = outer_pad;
    float y = 0;
    for (Element *elt : children) {
        elt->refactor();
        if (!elt->visible) continue;
        //if (!elt->rendering || elt->detached) continue;
        //Layout l = elt->get_layout();
        elt->x = x;
        elt->y = -elt->style.v_justify * outer_pad;
        x += elt->width + inner_pad;
        y = fmax(y, elt->height);
    }
    x += outer_pad - inner_pad;
    y += 2 * outer_pad;
    width = x;
    height = y;
}

VContainer::VContainer(std::vector<Element *> const &elts, float opad, float ipad, Style s) :
    Container(elts, 0, 0, s), outer_pad(opad), inner_pad(ipad)
{
    for (Element *elt : children)
        elt->style.v_justify = Style::Top;
    refactor();
}

void VContainer::refactor() {
    float x = 0;
    float y = outer_pad;
    for (Element *elt : children) {
        //elt->style.v_justify = -1;
        elt->refactor();
        if (!elt->visible) continue;
        //if (!elt->rendering || elt->detached) continue;
        //Layout l = elt->get_layout();
        elt->x = -elt->style.h_justify * outer_pad;
        elt->y = y;
        x = fmax(x, elt->width);
        y += elt->height + inner_pad;
    }
    x += 2 * outer_pad;
    y += outer_pad - inner_pad;
    width = x;
    height = y;
}

HFlexContainer::HFlexContainer(Element *l, Element *r, float opad, float ipad, Style s) : 
    Container({l,r},0,0,s), outer_pad(opad), inner_pad(ipad) 
{
    //style.h_flex = 1;
    l->style.h_justify = Style::Left;
    r->style.h_justify = Style::Right;
    refactor();
}

void HFlexContainer::refactor() {
    Container::refactor();
    width = children[0]->width + inner_pad + children[1]->width;
    height = fmax(children[0]->height, children[1]->height);
}

float HFlexContainer::stretched_width() const {
    // Keep title / close inside the plate face (rim ~7px + text stroke).
    float const inset = fmax(outer_pad, 20.f) + 8.f;
    float w = width;
    if (parent != nullptr)
        w = fmax(w, parent->width - 2.f * inset);
    return w;
}

void HFlexContainer::on_render(Renderer &ctx) {
    // Stretch to parent for this frame's title/close layout only.
    // Do NOT keep the expanded width: VContainer parents set width =
    // max(children) + 2*pad, so persisting parent->width here grows the
    // panel forever (talents / other make_drawer panels).
    float saved = width;
    width = stretched_width();
    Container::on_render(ctx);
    width = saved;
}

void HFlexContainer::poll_events(ScreenEvent const &event) {
    // Must match on_render stretch: otherwise the Right-justified close (×)
    // sits outside the narrow intrinsic hit box and never receives clicks.
    float saved = width;
    width = stretched_width();
    Container::poll_events(event);
    width = saved;
}
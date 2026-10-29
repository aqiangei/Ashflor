#pragma once
#include <Client/Ui/Element.hh>

namespace Ui {
// Keep tooltip pointers available immediately, but build stats/drop trees only
// when inspected. Building every mob rarity and petal at boot blocks the RAF.
class LazyTooltip final : public Element {
    std::function<Element *()> factory;
    Element *body = nullptr;
public:
    explicit LazyTooltip(std::function<Element *()> f) : factory(std::move(f)) {}
    void refactor() override {
        if (!body) { body = factory(); factory = nullptr; }
        body->refactor();
        width = body->width;
        height = body->height;
    }
    void on_render(Renderer &ctx) override { body->on_render(ctx); }
};
}

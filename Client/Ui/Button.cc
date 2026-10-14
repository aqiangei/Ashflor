#include <Client/Ui/Button.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Input.hh>

using namespace Ui;

// Enlarge compact controls on touch devices so they meet ~48–64pt targets
// (title icons, side bar, topbar, dialog X, etc.). Wide action buttons only
// get a taller hit/visual height.
static void apply_mobile_touch_size(float &w, float &h) {
    if (!Input::mobile_controls()) return;
    if (w <= 56.f && h <= 56.f) {
        w = h = 64.f;
        return;
    }
    if (h < 44.f) h = 44.f;
    if (w < 88.f && w <= h * 2.5f) w = 88.f;
}

static void poll_with_mobile_pad(Element *self, ScreenEvent const &event) {
    if (!Input::mobile_controls()) {
        self->Element::poll_events(event);
        return;
    }
    float const ow = self->width;
    float const oh = self->height;
    // Extra finger slop beyond the already-enlarged visual size
    self->width = ow + 16.f;
    self->height = oh + 16.f;
    self->Element::poll_events(event);
    self->width = ow;
    self->height = oh;
}

Button::Button(float w, float h, Element *l, void x(Element *, uint8_t), bool y (void), Style s) :
    Element(w, h, s), on_click(x), should_darken(y)
{
    apply_mobile_touch_size(width, height);
    if (l != nullptr) children.push_back(l);
}

void Button::on_render(Renderer &ctx) {
    if (style.fill != 0x00000000) {
        uint32_t face = style.fill;
        if (should_darken != nullptr && should_darken())
            face = Renderer::HSV(face, 0.72f);
        else if (focus_state == kMouseDown)
            face = Renderer::HSV(face, 0.85f);
        else if (focus_state == kMouseHover)
            face = Renderer::HSV(face, 1.12f);
        float radius = style.round_radius > 0 ? style.round_radius : Chrome::kRadius;
        // Cap bubble radius — prefer carved slab over candy pill
        if (radius > 6.f) radius = 6.f;
        float rim = style.line_width > 0 ? style.line_width : Chrome::kRim;
        if (rim < 4.f) rim = 4.f;
        Chrome::draw_plate(ctx, width, height, face, radius, rim);
        if (focus_state == kMouseDown) {
            ctx.set_fill(0x30000000);
            ctx.begin_path();
            ctx.rect(-width / 2 + rim * 0.7f, -height / 2 + rim * 0.7f,
                     width - rim * 1.4f, height - rim * 1.4f);
            ctx.fill();
        }
    }
    for (Element *child : children) child->render(ctx);
}

void Button::refactor() {
    for (Element *child : children) child->refactor();
}

void Button::on_event(uint8_t event) {
    on_click(this, event);
}

void Button::poll_events(ScreenEvent const &event) {
    poll_with_mobile_pad(this, event);
}

ToggleButton::ToggleButton(float w, uint8_t *t) : 
    Element(w,w,{ .fill = Chrome::kBtnIron, .stroke_hsv = 0.4, .line_width = 5, .round_radius = 3 }), toggler(t) {
    apply_mobile_touch_size(width, height);
    lerp_toggle = 0;
}

void ToggleButton::on_render(Renderer &ctx) {
    lerp_toggle = lerp(lerp_toggle, *toggler, Ui::lerp_amount * 2);
    uint32_t face = Renderer::MIX(Chrome::kBtnIron, Chrome::kBtnMoss, lerp_toggle);
    Chrome::draw_plate(ctx, width, height, face, 3.f, 5.f);
    // Inner marker
    float m = width * 0.28f;
    ctx.set_fill(*toggler ? Chrome::kGold : 0x66ffffff);
    ctx.begin_path();
    ctx.rect(-m / 2, -m / 2, m, m);
    ctx.fill();
}


void ToggleButton::on_event(uint8_t event) {
    if (event == kClick) *toggler ^= 1;
}

void ToggleButton::poll_events(ScreenEvent const &event) {
    poll_with_mobile_pad(this, event);
}

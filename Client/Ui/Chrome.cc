#include <Client/Ui/Chrome.hh>

#include <Client/Render/Renderer.hh>

#include <cmath>

void Ui::Chrome::draw_plate(Renderer &ctx, float w, float h, uint32_t face,
                            float radius, float rim) {
    float hw = w / 2.f;
    float hh = h / 2.f;
    radius = fmaxf(5.f, radius);
    bool panel = face == kPanelDeep || face == kPanelFace;
    float edge = rim > 0.f ? rim : (panel ? 7.f : 3.f);
    edge = fminf(edge, fminf(w, h) * 0.16f);

    ctx.set_fill(0x38000000);
    ctx.begin_path();
    ctx.round_rect(-hw + 2.f, -hh + 4.f, w, h, radius);
    ctx.fill();
    ctx.set_fill(panel ? kRimBrass : Renderer::HSV(face, 0.65f));
    ctx.begin_path();
    ctx.round_rect(-hw, -hh, w, h, radius);
    ctx.fill();
    ctx.set_fill(face);
    ctx.begin_path();
    ctx.round_rect(-hw + edge, -hh + edge, w - 2.f * edge,
                   h - 2.f * edge, fmaxf(3.f, radius - edge * 0.5f));
    ctx.fill();
}

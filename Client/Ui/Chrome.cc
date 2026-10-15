#include <Client/Ui/Chrome.hh>

#include <Client/Render/Renderer.hh>

#include <cmath>

void Ui::Chrome::draw_plate(Renderer &ctx, float w, float h, uint32_t face,
                            float radius, float rim) {
    float hw = w / 2.f;
    float hh = h / 2.f;
    if (radius < 8.f) radius = 8.f;

    // Soft drop shadow — depth without a flashy outline
    ctx.set_fill(0x28000000);
    ctx.begin_path();
    ctx.round_rect(-hw + 1.f, -hh + 3.f, w, h, radius);
    ctx.fill();

    // Single rounded body
    ctx.set_fill(face);
    ctx.begin_path();
    ctx.round_rect(-hw, -hh, w, h, radius);
    ctx.fill();

    // Subtle top light (not a rim)
    float sheen_h = fmax(5.f, (h - 4.f) * 0.32f);
    float inner_r = fmax(4.f, radius - 2.f);
    ctx.set_fill(0x16ffffff);
    ctx.begin_path();
    ctx.round_rect(-hw + 2.f, -hh + 2.f, w - 4.f, sheen_h, inner_r);
    ctx.fill();

    (void)rim;
}

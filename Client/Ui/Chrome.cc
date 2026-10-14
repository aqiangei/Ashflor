#include <Client/Ui/Chrome.hh>

#include <Client/Render/Renderer.hh>

void Ui::Chrome::draw_plate(Renderer &ctx, float w, float h, uint32_t face,
                            float radius, float rim) {
    float hw = w / 2.f;
    float hh = h / 2.f;
    // Deep outer slab
    ctx.set_fill(kRimDark);
    ctx.begin_path();
    ctx.round_rect(-hw - 2.f, -hh - 2.f, w + 4.f, h + 4.f, radius + 1.f);
    ctx.fill();
    // Body
    ctx.set_fill(Renderer::HSV(face, 0.82f));
    ctx.begin_path();
    ctx.round_rect(-hw, -hh, w, h, radius);
    ctx.fill();
    // Brass rim
    ctx.set_stroke(kRimBrass);
    ctx.set_line_width(rim * 0.55f);
    ctx.begin_path();
    ctx.round_rect(-hw + 1.f, -hh + 1.f, w - 2.f, h - 2.f, radius);
    ctx.stroke();
    // Inner face (sharper, less "bubble")
    float inset = rim * 0.7f;
    ctx.set_fill(face);
    ctx.begin_path();
    ctx.rect(-hw + inset, -hh + inset, w - 2.f * inset, h - 2.f * inset);
    ctx.fill();
    // Top sheen — weight without glow spam
    ctx.set_fill(0x22ffffff);
    ctx.begin_path();
    ctx.rect(-hw + inset, -hh + inset, w - 2.f * inset, (h - 2.f * inset) * 0.18f);
    ctx.fill();
    // Bottom shade
    ctx.set_fill(0x28000000);
    ctx.begin_path();
    ctx.rect(-hw + inset, hh - inset - (h - 2.f * inset) * 0.14f,
             w - 2.f * inset, (h - 2.f * inset) * 0.14f);
    ctx.fill();
}

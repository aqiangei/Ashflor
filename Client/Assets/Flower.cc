#include <Client/Assets/Assets.hh>

#include <Client/StaticData.hh>

#include <Helpers/Bits.hh>

void draw_static_flower(Renderer &ctx, FlowerRenderAttributes attributes) {
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kCutter)) {
        RenderContext context(&ctx);
        ctx.scale(attributes.radius / 25);
        ctx.rotate(attributes.cutter_angle);
        draw_static_petal_single(PetalID::kCutter, ctx);
    }
    uint32_t base_color = FLOWER_COLORS[attributes.color];
    if (BitMath::at(attributes.face_flags, FaceFlags::kPoisoned)) 
        base_color = 0xffce76db;
    else if (BitMath::at(attributes.face_flags, FaceFlags::kDandelioned)) 
        base_color = Renderer::MIX(base_color, 0xffffffff, 0.4);
    // DeadEyes: keep normal petal color — only the X eyes mark death (no grey wash).
    ctx.set_stroke(Renderer::HSV(base_color, 0.8));
    ctx.set_fill(base_color);
    ctx.set_line_width(3);
    ctx.begin_path();
    ctx.arc(0, 0, attributes.radius);
    ctx.fill();
    ctx.stroke();
    ctx.scale(attributes.radius / 25);
    {
        RenderContext context(&ctx);
        ctx.set_fill(0xff222222);
        ctx.set_stroke(0xff222222);
        ctx.set_line_width(1);
        ctx.begin_path();
        if (BitMath::at(attributes.face_flags, FaceFlags::kDeadEyes)) {
            float const len = 4;
            ctx.set_stroke(0xff222222);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(-7-len, -5-len);
            ctx.line_to(-7+len,-5+len);
            ctx.move_to(-7+len,-5-len);
            ctx.line_to(-7-len,-5+len);
            ctx.move_to(7-len, -5-len);
            ctx.line_to(7+len,-5+len);
            ctx.move_to(7+len,-5-len);
            ctx.line_to(7-len,-5+len);
            ctx.stroke();
        } else if (BitMath::at(attributes.face_flags, FaceFlags::kSquareEyes)) {
            ctx.rect(-7-3, -5-6, 6, 12);
            ctx.move_to(7, -5);
            ctx.rect(7-3, -5-6, 6, 12);
            ctx.fill();
            ctx.stroke();
            ctx.clip();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.rect(-7 + attributes.eye_x-3, -5 + attributes.eye_y-3, 6, 6);
            ctx.rect(7 + attributes.eye_x-3, -5 + attributes.eye_y-3, 6, 6);
            ctx.fill();
        } else {
            ctx.ellipse(-7, -5, 3, 6);
            ctx.move_to(7, -5);
            ctx.ellipse(7, -5, 3, 6);
            ctx.fill();
            ctx.stroke();
            ctx.clip();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(-7 + attributes.eye_x, -5 + attributes.eye_y, 3);
            ctx.arc(7 + attributes.eye_x, -5 + attributes.eye_y, 3);
            ctx.fill();
        }
    }
    ctx.set_stroke(0xff222222);
    ctx.set_line_width(1.5);
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.move_to(-6, 10);
    ctx.qcurve_to(0, attributes.mouth, 6, 10);
    ctx.stroke();
    if (!BitMath::at(attributes.face_flags, FaceFlags::kDeadEyes) && attributes.mouth <= 8 && BitMath::at(attributes.face_flags, FaceFlags::kAttacking))
    {
        RenderContext context(&ctx);
        ctx.translate(0, -attributes.mouth - 8);
        ctx.set_fill(base_color);
        ctx.begin_path();
        ctx.move_to(-12, 0);
        ctx.line_to(12, 0);
        ctx.line_to(0, 6);
        ctx.line_to(-12, 0);
        ctx.fill();
    }
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kThirdEye))
    {
        RenderContext g(&ctx);
        ctx.translate(0, -14);
        ctx.scale(0.5);
        if (BitMath::at(attributes.face_flags, FaceFlags::kDeadEyes)) {
            float const len = 4;
            ctx.set_stroke(0xff222222);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(-len,-len);
            ctx.line_to(len,len);
            ctx.move_to(len,-len);
            ctx.line_to(-len,len);
            ctx.stroke();
        }
        else
            draw_static_petal(PetalID::kThirdEye, ctx);
    }
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kObserver))
    {
        RenderContext g(&ctx);
        ctx.translate(0, -35);
        draw_static_petal(PetalID::kObserver, ctx);
    } else if (BitMath::at(attributes.equip_flags, EquipmentFlags::kAntennae))
    {
        RenderContext g(&ctx);
        ctx.translate(0, -35);
        draw_static_petal(PetalID::kAntennae, ctx);
    }
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kAccelerator))
    {
        // Rear thruster under the flower — readable silhouette, not a clutter blot.
        RenderContext g(&ctx);
        ctx.translate(0, 30);
        ctx.scale(0.72f);
        draw_static_petal(PetalID::kAccelerator, ctx);
    }
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kMagnet))
    {
        // Hip accessory — horseshoe reads clearly at small scale.
        RenderContext g(&ctx);
        ctx.translate(30, 10);
        ctx.scale(0.62f);
        draw_static_petal(PetalID::kMagnet, ctx);
    }
    // Bandage: layered gauze wraps + fray tips (equipped look).
    if (BitMath::at(attributes.equip_flags, EquipmentFlags::kBandage))
    {
        RenderContext g(&ctx);
        ctx.round_line_cap();
        ctx.round_line_join();
        // Soft under-shadow so wraps sit on the petal body
        ctx.set_stroke(0x33000000);
        ctx.set_line_width(7.2f);
        ctx.begin_path();
        ctx.move_to(-19, -5);
        ctx.qcurve_to(-2, -23, 19, -3);
        ctx.move_to(-18, 5);
        ctx.qcurve_to(2, 20, 18, 7);
        ctx.move_to(-12, -14);
        ctx.qcurve_to(8, -2, 14, 14);
        ctx.stroke();
        // Main cream strips
        ctx.set_stroke(0xfff4ebe0);
        ctx.set_line_width(6.2f);
        ctx.begin_path();
        ctx.move_to(-19, -5);
        ctx.qcurve_to(-2, -23, 19, -3);
        ctx.move_to(-18, 5);
        ctx.qcurve_to(2, 20, 18, 7);
        ctx.move_to(-12, -14);
        ctx.qcurve_to(8, -2, 14, 14);
        ctx.stroke();
        // Weave seam lines
        ctx.set_stroke(0xffc8b090);
        ctx.set_line_width(1.6f);
        ctx.begin_path();
        ctx.move_to(-19, -5);
        ctx.qcurve_to(-2, -23, 19, -3);
        ctx.move_to(-18, 5);
        ctx.qcurve_to(2, 20, 18, 7);
        ctx.move_to(-12, -14);
        ctx.qcurve_to(8, -2, 14, 14);
        ctx.stroke();
        // Soft highlight edge on top strip
        ctx.set_stroke(0x99ffffff);
        ctx.set_line_width(1.8f);
        ctx.begin_path();
        ctx.move_to(-17, -7);
        ctx.qcurve_to(-2, -24, 17, -5);
        ctx.stroke();
        // Frayed ends
        ctx.set_stroke(0xffd4c4a8);
        ctx.set_line_width(1.3f);
        ctx.begin_path();
        ctx.move_to(19, -3); ctx.line_to(23, -6);
        ctx.move_to(19.5f, -1); ctx.line_to(24, -1);
        ctx.move_to(18, 7); ctx.line_to(22, 10);
        ctx.move_to(18.5f, 5); ctx.line_to(23, 6);
        ctx.stroke();
        // Tiny red seal over the cross-wrap
        ctx.set_fill(0xffe04848);
        ctx.set_stroke(0xffa02828);
        ctx.set_line_width(1.1f);
        ctx.begin_path();
        ctx.arc(4, 0, 2.6f);
        ctx.fill();
        ctx.stroke();
        ctx.set_stroke(0xfffff0f0);
        ctx.set_line_width(1.4f);
        ctx.begin_path();
        ctx.move_to(2.4f, 0); ctx.line_to(5.6f, 0);
        ctx.move_to(4, -1.6f); ctx.line_to(4, 1.6f);
        ctx.stroke();
    }
}

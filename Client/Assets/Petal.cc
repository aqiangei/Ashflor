#include <Client/Assets/Assets.hh>
#include <Client/Assets/FlorrArt.hh>
#ifndef ART_STANDALONE
#include <Client/Assets/PetalRasterCache.hh>
#endif

#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <cassert>
#include <cmath>
#include <cstring>

// Ladder variants share art with the first petal of the same English name.
static PetalID::T petal_art_base(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return id;
    char const *name = PETAL_DATA[id].name;
    for (PetalID::T i = 1; i < id; ++i) {
        if (std::strcmp(PETAL_DATA[i].name, name) == 0)
            return i;
    }
    return id;
}

void draw_static_petal_single(PetalID::T id, Renderer &ctx) {
    float r = PETAL_DATA[id].radius;
    PetalID::T art = petal_art_base(id);
    // Florr sprites first. Never bake them into PetalRasterCache — if the PNG is
    // still loading, a procedural bake would lock out florrclone art forever.
    if (FlorrArt::try_draw_petal(art, ctx, r)) return;
#ifndef ART_STANDALONE
    if (!FlorrArt::has_petal_sprite(art) && PetalRasterCache::try_draw(art, ctx))
        return;
#endif
    switch(art) {
        case PetalID::kNone:
            break;
        case PetalID::kDandelion:
            ctx.set_stroke(0xff222222);
            ctx.round_line_cap();
            ctx.set_line_width(7);
            ctx.begin_path();
            ctx.move_to(0,0);
            ctx.line_to(-1.6 * r, 0);
            ctx.stroke();
        case PetalID::kUniqueBasic:
        case PetalID::kBasic:
        case PetalID::kLight:
        case PetalID::kTwin:
        case PetalID::kTriplet:
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xffcfcfcf);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kHeavy:
            ctx.set_fill(0xffbbbbbb);
            ctx.set_stroke(0xff8d8d8d);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kStinger: {
        case PetalID::kTringer:
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(r,0);
            ctx.line_to(-r*0.5,r*0.866);
            ctx.line_to(-r*0.5,-r*0.866);
            ctx.line_to(r,0);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kGoldenLeaf:
        case PetalID::kGoldenLeafEpic:
        case PetalID::kGoldenLeafLegendary:
        case PetalID::kGoldenLeafMythic:
        case PetalID::kGoldenLeafUltra:
        case PetalID::kGoldenLeafSuper:
        case PetalID::kGoldenLeafUnique:
        case PetalID::kLeaf: 
            // green Leaf ladder vs gold Golden Leaf ladder (art_base usually collapses first)
            if (art == PetalID::kLeaf) {
                ctx.set_fill(0xff39b54a);
                ctx.set_stroke(0xff2e933c);
            } else {
                ctx.set_fill(0xffebeb34);
                ctx.set_stroke(0xffbebe2a);
            }
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-20, 0);
            ctx.line_to(-15, 0);
            ctx.bcurve_to(-10,-12,5,-12,15,0);
            ctx.bcurve_to(5,12,-10,12,-15,0);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-9,0);
            ctx.qcurve_to(0,-1.5,7.5,0);
            ctx.stroke();
            break;
        case PetalID::kRose:
        case PetalID::kDahlia:
            ctx.set_fill(0xffff94c9);
            ctx.set_stroke(0xffcf78a3);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kAntEgg:
            ctx.set_stroke(0xffcfc295);
            ctx.set_fill(0xfffff0b8);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kBeetleEgg:
            ctx.begin_path();
            ctx.ellipse(0,0,r * 0.85, r * 1.15);
            ctx.set_fill(0xfffff0b8);
            ctx.fill();
            ctx.set_stroke(0xffcfc295);
            ctx.set_line_width(3);
            ctx.stroke();
            break;
        case PetalID::kMissile:
            ctx.scale(r / 10);
            ctx.set_fill(0xff222222);
            ctx.set_stroke(0xff222222);
            ctx.set_line_width(5.0);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(11.0, 0.0);
            ctx.line_to(-11.0, -6.0);
            ctx.line_to(-11.0, 6.0);
            ctx.line_to(11.0, 0.0);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kIris:
            ctx.set_fill(0xffce76db);
            ctx.set_stroke(0xffa760b1);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kPollen:
            ctx.set_fill(0xffffe763);
            ctx.set_stroke(0xffcfbb50);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kBubble:
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.set_stroke(0xb2ffffff);
            ctx.set_line_width(3);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0,0,r-1.5);
            ctx.set_fill(0x59ffffff);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(r/3,-r/3,r/4);
            ctx.set_fill(0x59ffffff);
            ctx.fill();
            break;
        case PetalID::kFaster:
            ctx.set_fill(0xfffeffc9);
            ctx.set_stroke(0xffcecfa3);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kThirdEye:
            ctx.scale(r / 10);
            ctx.set_fill(0xff111111);
            ctx.begin_path();
            ctx.move_to(0,-10);
            ctx.qcurve_to(8,0,0,10);
            ctx.qcurve_to(-8,0,0,-10);
            ctx.fill();
            ctx.set_fill(0xffeeeeee);
            ctx.begin_path();
            ctx.arc(0, 0, 5);
            ctx.fill();
            ctx.set_stroke(0xff111111);
            ctx.set_line_width(1.5);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0,-10);
            ctx.qcurve_to(8,0,0,10);
            ctx.qcurve_to(-8,0,0,-10);
            ctx.stroke();
            break;
        case PetalID::kWeb:
        case PetalID::kTriweb:
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xffcfcfcf);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(11.00, 0.00);
            ctx.qcurve_to(4.32, 3.14, 3.40, 10.46);
            ctx.qcurve_to(-1.65, 5.08, -8.90, 6.47);
            ctx.qcurve_to(-5.34, -0.00, -8.90, -6.47);
            ctx.qcurve_to(-1.65, -5.08, 3.40, -10.46);
            ctx.qcurve_to(4.32, -3.14, 11.00, 0.00);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kWing:
            ctx.begin_path();
            ctx.partial_arc(0,0,15,-1.5707963267948966,1.5707963267948966,0);
            ctx.qcurve_to(10,0,0,-15);
            ctx.set_fill(0xffffffff);
            ctx.fill();
            ctx.set_stroke(0xffcfcfcf);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.stroke();
            break;
        case PetalID::kRock: {
            ctx.set_fill(0xff777777);
            ctx.set_stroke(Renderer::HSV(0xff777777, 0.8));
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(12.138091087341309,0);
            ctx.line_to(3.8414306640625,12.377452850341797);
            ctx.line_to(-11.311542510986328,7.916932582855225);
            ctx.line_to(-11.461170196533203,-7.836822032928467);
            ctx.line_to(4.538298606872559,-13.891617774963379);
            ctx.line_to(12.138091087341309,0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kAntennae: {
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(5, 12.5);
            ctx.qcurve_to(10, -2.5, 15, -12.5);
            ctx.qcurve_to(5, -2.5, 5, 12.5);
            ctx.move_to(-5, 12.5);
            ctx.qcurve_to(-10, -2.5, -15, -12.5);
            ctx.qcurve_to(-5, -2.5, -5, 12.5);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kAccelerator: {
            // Teal thruster pod + amber chevron exhaust (scales with radius).
            float const s = fmaxf(8.f, r);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_fill(0xff2eb88a);
            ctx.set_stroke(0xff176848);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s * 0.52f, -s * 0.78f, s * 1.04f, s * 1.15f, s * 0.32f);
            ctx.fill();
            ctx.stroke();
            // Side fins
            ctx.set_fill(0xff48d4a4);
            ctx.begin_path();
            ctx.move_to(-s * 0.52f, -s * 0.15f);
            ctx.line_to(-s * 0.95f, s * 0.1f);
            ctx.line_to(-s * 0.52f, s * 0.28f);
            ctx.close_path();
            ctx.fill();
            ctx.begin_path();
            ctx.move_to(s * 0.52f, -s * 0.15f);
            ctx.line_to(s * 0.95f, s * 0.1f);
            ctx.line_to(s * 0.52f, s * 0.28f);
            ctx.close_path();
            ctx.fill();
            // Intake highlight
            ctx.set_fill(0xffb8ffe0);
            ctx.begin_path();
            ctx.ellipse(0, -s * 0.55f, s * 0.3f, s * 0.16f);
            ctx.fill();
            // Exhaust chevrons
            ctx.set_fill(0xffffc14a);
            ctx.set_stroke(0xffd48820);
            ctx.set_line_width(1.5f);
            for (int i = 0; i < 3; ++i) {
                float y0 = s * (0.22f + i * 0.26f);
                float w = s * (0.48f - i * 0.09f);
                ctx.begin_path();
                ctx.move_to(0, y0 + s * 0.24f);
                ctx.line_to(-w, y0);
                ctx.line_to(w, y0);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            }
            break;
        }
        case PetalID::kMagnet: {
            // Classic horseshoe: red / blue arms + silver pole tips.
            float const s = fmaxf(8.f, r);
            float const thick = s * 0.4f;
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_line_width(thick);
            ctx.set_stroke(0xffc62828);
            ctx.begin_path();
            ctx.move_to(-s * 0.58f, s * 0.78f);
            ctx.line_to(-s * 0.58f, s * 0.02f);
            ctx.qcurve_to(-s * 0.58f, -s * 0.72f, 0, -s * 0.72f);
            ctx.stroke();
            ctx.set_stroke(0xff1565c0);
            ctx.begin_path();
            ctx.move_to(s * 0.58f, s * 0.78f);
            ctx.line_to(s * 0.58f, s * 0.02f);
            ctx.qcurve_to(s * 0.58f, -s * 0.72f, 0, -s * 0.72f);
            ctx.stroke();
            // Soft inner highlight on the arch
            ctx.set_line_width(thick * 0.35f);
            ctx.set_stroke(0x66ffffff);
            ctx.begin_path();
            ctx.move_to(-s * 0.35f, -s * 0.15f);
            ctx.qcurve_to(0, -s * 0.48f, s * 0.35f, -s * 0.15f);
            ctx.stroke();
            // Pole tips
            ctx.set_fill(0xfff2f2f2);
            ctx.set_stroke(0xff9a9a9a);
            ctx.set_line_width(1.6f);
            float tip_w = thick * 1.15f;
            float tip_h = s * 0.38f;
            ctx.begin_path();
            ctx.round_rect(-s * 0.58f - tip_w * 0.5f, s * 0.52f, tip_w, tip_h, 2.f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.round_rect(s * 0.58f - tip_w * 0.5f, s * 0.52f, tip_w, tip_h, 2.f);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kObserver: {
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(5, 12.5);
            ctx.qcurve_to(10, -2.5, 15, -12.5);
            ctx.qcurve_to(5, -2.5, 5, 12.5);
            ctx.move_to(-5, 12.5);
            ctx.qcurve_to(-10, -2.5, -15, -12.5);
            ctx.qcurve_to(-5, -2.5, -5, 12.5);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffd01c1d);
            ctx.begin_path();
            ctx.arc(15, -12.5, 2.5);
            ctx.close_path();
            ctx.arc(-15, -12.5, 2.5);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kBlueIris:
            ctx.set_fill(0xff39e9f1);
            ctx.set_stroke(0xff2dbac0);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kCactus:
        case PetalID::kTricac:
            ctx.set_fill(0xff38c75f);
            ctx.set_stroke(Renderer::HSV(0xff38c75f, 0.8));
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(15,0);
            for (uint32_t i = 0; i < 8; ++i) {
                float base_angle = M_PI * 2 * i / 8;
                ctx.qcurve_to(15*0.8*cosf(base_angle+M_PI/8),15*0.8*sinf(base_angle+M_PI/8),15*cosf(base_angle+2*M_PI/8),15*sinf(base_angle+2*M_PI/8));
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff74d68f);
            ctx.begin_path();
            ctx.arc(0,0,8);
            ctx.fill();
            break;
        case PetalID::kPoisonPeas:
            ctx.set_fill(0xffce76db);
            ctx.set_stroke(0xffa760b1);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,PETAL_DATA[id].radius);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kPeas:
            ctx.set_fill(0xff8ac255);
            ctx.set_stroke(0xff709d45);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0,0,PETAL_DATA[id].radius);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kSand:
            ctx.set_fill(0xffe0c85c);
            ctx.set_stroke(0xffb5a24b);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(7,0);
            ctx.line_to(3.499999761581421,6.062178134918213);
            ctx.line_to(-3.500000476837158,6.062177658081055);
            ctx.line_to(-7,-6.119594218034763e-7);
            ctx.line_to(-3.4999992847442627,-6.062178134918213);
            ctx.line_to(3.4999992847442627,-6.062178134918213);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kStick:
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_line_width(7);
            ctx.set_stroke(0xff654a19);
            ctx.begin_path();
            ctx.move_to(0,10);
            ctx.line_to(0,0);
            ctx.line_to(4,-7);
            ctx.move_to(0,0);
            ctx.line_to(-6,-10);
            ctx.stroke();
            ctx.set_line_width(3);
            ctx.set_stroke(0xff7d5b1f);
            ctx.stroke();
            break;
        case PetalID::kPincer:
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(10,5);
            ctx.qcurve_to(4,-14,-10,5);
            ctx.qcurve_to(4,0,10,5);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kAzalea: {
            ctx.set_fill(0xffff94c9);
            ctx.set_stroke(0xffcf78a3);
            ctx.set_line_width(3);
            ctx.begin_path();
            uint32_t s = 3;
            ctx.move_to(r, 0);
            for (uint32_t i = 1; i <= s; ++i) {
                float angle = i * 2 * M_PI / s;
                float angle2 = angle - M_PI / s;
                ctx.qcurve_to(2 * r * cosf(angle2), 2 * r * sinf(angle2), r * cosf(angle), r * sinf(angle));
            }
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kPoisonCactus:
            ctx.set_fill(0xffce76db);
            ctx.set_stroke(0xffa760b1);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(15,0);
            for (uint32_t i = 0; i < 8; ++i) {
                float base_angle = M_PI * 2 * i / 8;
                ctx.qcurve_to(15*0.8*cosf(base_angle+M_PI/8),15*0.8*sinf(base_angle+M_PI/8),15*cosf(base_angle+2*M_PI/8),15*sinf(base_angle+2*M_PI/8));
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffcea0db);
            ctx.begin_path();
            ctx.arc(0,0,8);
            ctx.fill();
            break;
        case PetalID::kSalt:
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xffcfcfcf);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(10.404077529907227,0);
            ctx.line_to(6.643442630767822,8.721502304077148);
            ctx.line_to(-2.6667866706848145,11.25547981262207);
            ctx.line_to(-10.940428733825684,4.95847225189209);
            ctx.line_to(-11.341578483581543,-5.432167053222656);
            ctx.line_to(-2.4972469806671143,-11.472168922424316);
            ctx.line_to(7.798409461975098,-9.584606170654297);
            ctx.line_to(10.404077529907227,0);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kSquare:
            ctx.set_fill(0xffffe869);
            ctx.set_stroke(0xffcfbc55);
            ctx.set_line_width(0.15*r);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.rect(-r * 0.707, -r * 0.707, r * 1.414, r * 1.414);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kTriangle: {
            // White spike — same silhouette as Stinger, single tip (florr Triangle).
            ctx.set_fill(0xfff4f4f4);
            ctx.set_stroke(0xffc8c8c8);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(r, 0);
            ctx.line_to(-r * 0.5f, r * 0.866f);
            ctx.line_to(-r * 0.5f, -r * 0.866f);
            ctx.line_to(r, 0);
            ctx.fill();
            ctx.stroke();
            break;
        }

        case PetalID::kLentil: {
            RenderContext save(&ctx);
            ctx.rotate(0.785398f);
            ctx.set_fill(0xffebb134);
            ctx.set_stroke(0xffc4942d);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, r * 0.85f, r * 1.05f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffd4a83a);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(0, -r * 0.7f);
            ctx.qcurve_to(r * 0.15f, 0, 0, r * 0.7f);
            ctx.stroke();
            ctx.set_fill(0x66ffe9a0);
            ctx.begin_path();
            ctx.ellipse(-r * 0.2f, -r * 0.25f, r * 0.28f, r * 0.4f);
            ctx.fill();
            break;
        }
        case PetalID::kSawblade: {
            ctx.set_fill(0xff9a9a9a);
            ctx.set_stroke(0xff6e6e6e);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.55f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffb8b8b8);
            ctx.set_stroke(0xff5a5a5a);
            ctx.set_line_width(2);
            ctx.begin_path();
            uint32_t const teeth = 12;
            for (uint32_t i = 0; i < teeth; ++i) {
                float a0 = (float) i * (float) (2 * M_PI) / (float) teeth;
                float a1 = a0 + (float) (2 * M_PI) / (float) teeth;
                float amid = (a0 + a1) * 0.5f;
                float tip = amid - 0.08f;
                if (i == 0) ctx.move_to(cosf(a0) * r * 0.62f, sinf(a0) * r * 0.62f);
                ctx.line_to(cosf(tip) * r, sinf(tip) * r);
                ctx.line_to(cosf(a1) * r * 0.62f, sinf(a1) * r * 0.62f);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff5a5a5a);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.22f);
            ctx.fill();
            ctx.set_fill(0xff3a3a3a);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.12f);
            ctx.fill();
            break;
        }
        case PetalID::kDice: {
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.round_rect(-r, -r, 2 * r, 2 * r, r * 0.25f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff222222);
            for (float px : {-0.4f, 0.4f})
                for (float py : {-0.4f, 0.4f}) {
                    ctx.begin_path();
                    ctx.arc(px * r, py * r, r * 0.14f);
                    ctx.fill();
                }
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.14f);
            ctx.fill();
            break;
        }
        case PetalID::kLightning: {
            ctx.set_fill(0xffffe066);
            ctx.set_stroke(0xffc9a227);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-0.2f * r, -r);
            ctx.line_to(0.35f * r, -0.1f * r);
            ctx.line_to(0.05f * r, -0.1f * r);
            ctx.line_to(0.4f * r, r);
            ctx.line_to(-0.35f * r, 0.15f * r);
            ctx.line_to(-0.05f * r, 0.15f * r);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }

        case PetalID::kMoon: {
            ctx.set_fill(0xff878787);
            ctx.set_stroke(0xff6d6d6d);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.arc(0,0,r);
            ctx.stroke();
            ctx.fill();
            ctx.clip();
            SeedGenerator gen(id * 274633 + 284562);
            uint32_t ct = 10;
            ctx.set_fill(0xff999999);
            ctx.set_stroke(0xff7c7c7c);
            ctx.set_line_width(3);
            ctx.begin_path();
            for (uint32_t i = 0; i < ct; ++i) {
                float _x = gen.binext() * (r + 10);
                float _y = gen.binext() * (r + 10);
                float _r = gen.binext() * 10 + 10;
                ctx.move_to(_x,_y);
                ctx.arc(_x,_y,_r);
            }
            ctx.stroke();
            ctx.fill(1);
            break;
        }
        case PetalID::kLotus:
            ctx.scale(r / 10);
            ctx.set_fill(0xffce76db);
            ctx.set_stroke(0xffa760b1);
            ctx.set_line_width(2);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0.00, -10.00);
            ctx.bcurve_to(1.44, -7.11, 2.05, -5.89, 1.83, -4.41);
            ctx.bcurve_to(2.72, -5.62, 4.01, -6.05, 7.07, -7.07);
            ctx.bcurve_to(6.05, -4.01, 5.62, -2.72, 4.41, -1.83);
            ctx.bcurve_to(5.89, -2.05, 7.11, -1.44, 10.00, 0.00);
            ctx.bcurve_to(7.11, 1.44, 5.89, 2.05, 4.41, 1.83);
            ctx.bcurve_to(5.62, 2.72, 6.05, 4.01, 7.07, 7.07);
            ctx.bcurve_to(4.01, 6.05, 2.72, 5.62, 1.83, 4.41);
            ctx.bcurve_to(2.05, 5.89, 1.44, 7.11, 0.00, 10.00);
            ctx.bcurve_to(-1.44, 7.11, -2.05, 5.89, -1.83, 4.41);
            ctx.bcurve_to(-2.72, 5.62, -4.01, 6.05, -7.07, 7.07);
            ctx.bcurve_to(-6.05, 4.01, -5.62, 2.72, -4.41, 1.83);
            ctx.bcurve_to(-5.89, 2.05, -7.11, 1.44, -10.00, 0.00);
            ctx.bcurve_to(-7.11, -1.44, -5.89, -2.05, -4.41, -1.83);
            ctx.bcurve_to(-5.62, -2.72, -6.05, -4.01, -7.07, -7.07);
            ctx.bcurve_to(-4.01, -6.05, -2.72, -5.62, -1.83, -4.41);
            ctx.bcurve_to(-2.05, -5.89, -1.44, -7.11, 0.00, -10.00);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffa760b1);
            ctx.begin_path();
            ctx.arc(0,0,1.74);
            ctx.fill();
            break;
        case PetalID::kHeaviest:
            ctx.begin_path();
            ctx.arc(0,0,16);
            ctx.set_fill(0xff333333);
            ctx.fill();
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(3);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(6,-6,4.6);
            ctx.set_fill(0xffcccccc);
            ctx.fill();
            break;
        case PetalID::kCutter:
            ctx.set_fill(0xff111111);
            ctx.begin_path();
            ctx.arc(0,0,25);
            ctx.move_to(24.748737335205078,24.748737335205078);
            ctx.qcurve_to(9.899494171142578,23.899494171142578,-0.0000015298985545086907,35);
            ctx.qcurve_to(-9.899496078491211,23.899494171142578,-24.748737335205078,24.748737335205078);
            ctx.qcurve_to(-23.899494171142578,9.899493217468262,-35,-0.0000030597971090173814);
            ctx.qcurve_to(-23.89949607849121,-9.899496078491211,-24.74873924255371,-24.748735427856445);
            ctx.qcurve_to(-9.899496078491211,-23.899494171142578,4.173708134658227e-7,-35);
            ctx.qcurve_to(9.899493217468262,-23.89949607849121,24.748733520507812,-24.748741149902344);
            ctx.qcurve_to(23.899494171142578,-9.899494171142578,35,0.000006119594218034763);
            ctx.qcurve_to(23.899494171142578,9.899497032165527,24.748737335205078,24.748737335205078);
            ctx.fill();
            break;
        case PetalID::kYinYang:
            ctx.set_line_width(3);
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xffcfcfcf);
            ctx.begin_path();
            ctx.partial_arc(0,0,r,M_PI/2,3*M_PI/2,0);
            ctx.partial_arc(0,-r/2,r/2,-M_PI/2,M_PI/2,0);
            ctx.partial_arc(0,r/2,r/2,-M_PI/2,M_PI/2,1);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff292929);
            ctx.begin_path();
            ctx.partial_arc(0,0,r,-M_PI/2,M_PI/2,0);
            ctx.partial_arc(0,r/2,r/2,M_PI/2,3*M_PI/2,0);
            ctx.partial_arc(0,-r/2,r/2,M_PI/2,3*M_PI/2,1);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffcfcfcf);
            ctx.begin_path();
            ctx.partial_arc(0,0,r,M_PI,3*M_PI/2,0);
            ctx.partial_arc(0,-r/2,r/2,-M_PI/2,M_PI/2,0);
            ctx.stroke();
            break;
        case PetalID::kYggdrasil:
            ctx.scale(r / 255);
            ctx.set_fill(0xff886d35);
            ctx.begin_path();
            ctx.move_to(-273.54, -218.49);
            ctx.qcurve_to(-284.88, -187.49, -267.08, -151.41);
            ctx.qcurve_to(-262.72, -142.57, -254.82, -136.69);
            ctx.qcurve_to(-246.93, -130.80, -237.22, -129.15);
            ctx.qcurve_to(-222.87, -126.71, -208.62, -122.97);
            ctx.qcurve_to(-212.49, -112.19, -216.13, -100.75);
            ctx.qcurve_to(-218.33, -96.85, -219.63, -92.56);
            ctx.qcurve_to(-220.93, -88.27, -221.28, -83.80);
            ctx.qcurve_to(-229.91, -54.12, -240.77, -7.55);
            ctx.qcurve_to(-244.62, 8.97, -235.66, 23.37);
            ctx.qcurve_to(-226.71, 37.78, -210.19, 41.64);
            ctx.line_to(-199.07, 44.23);
            ctx.line_to(-203.90, 73.81);
            ctx.qcurve_to(-203.91, 73.87, -203.91, 73.93);
            ctx.qcurve_to(-203.92, 73.98, -203.93, 74.04);
            ctx.qcurve_to(-205.20, 82.09, -203.29, 90.01);
            ctx.qcurve_to(-201.38, 97.93, -196.58, 104.52);
            ctx.qcurve_to(-191.79, 111.10, -184.84, 115.35);
            ctx.qcurve_to(-177.88, 119.60, -169.84, 120.87);
            ctx.line_to(-168.30, 121.11);
            ctx.line_to(-169.08, 125.06);
            ctx.qcurve_to(-169.09, 125.10, -169.10, 125.14);
            ctx.qcurve_to(-169.11, 125.18, -169.11, 125.22);
            ctx.qcurve_to(-170.67, 133.22, -169.04, 141.20);
            ctx.qcurve_to(-167.42, 149.18, -162.86, 155.94);
            ctx.qcurve_to(-158.30, 162.69, -151.51, 167.19);
            ctx.qcurve_to(-144.72, 171.69, -136.72, 173.24);
            ctx.line_to(-118.37, 176.80);
            ctx.qcurve_to(-118.31, 176.82, -118.25, 176.83);
            ctx.qcurve_to(-118.18, 176.84, -118.12, 176.85);
            ctx.qcurve_to(-117.31, 178.64, -116.33, 180.35);
            ctx.qcurve_to(-115.35, 182.05, -114.21, 183.65);
            ctx.qcurve_to(-109.49, 190.30, -102.59, 194.63);
            ctx.qcurve_to(-95.69, 198.96, -87.66, 200.32);
            ctx.line_to(-69.22, 203.45);
            ctx.qcurve_to(-68.74, 203.53, -68.27, 203.60);
            ctx.qcurve_to(-67.79, 203.67, -67.31, 203.73);
            ctx.qcurve_to(-63.19, 213.00, -55.22, 219.27);
            ctx.qcurve_to(-47.25, 225.55, -37.27, 227.38);
            ctx.line_to(-18.90, 230.76);
            ctx.qcurve_to(-14.56, 231.55, -10.15, 231.41);
            ctx.qcurve_to(-5.73, 231.26, -1.46, 230.18);
            ctx.qcurve_to(2.41, 233.19, 6.88, 235.20);
            ctx.qcurve_to(11.34, 237.21, 16.16, 238.11);
            ctx.line_to(34.53, 241.54);
            ctx.qcurve_to(34.57, 241.55, 34.60, 241.56);
            ctx.qcurve_to(34.64, 241.56, 34.68, 241.57);
            ctx.qcurve_to(37.13, 242.02, 39.62, 242.17);
            ctx.qcurve_to(42.11, 242.32, 44.60, 242.16);
            ctx.qcurve_to(50.12, 249.01, 57.96, 252.99);
            ctx.qcurve_to(65.80, 256.97, 74.58, 257.39);
            ctx.line_to(93.24, 258.26);
            ctx.qcurve_to(94.95, 258.34, 96.66, 258.28);
            ctx.qcurve_to(98.37, 258.21, 100.07, 258.01);
            ctx.qcurve_to(105.67, 262.39, 112.42, 264.63);
            ctx.qcurve_to(119.17, 266.87, 126.27, 266.70);
            ctx.line_to(144.96, 266.26);
            ctx.line_to(144.99, 266.26);
            ctx.qcurve_to(146.79, 266.22, 148.59, 266.01);
            ctx.qcurve_to(150.38, 265.81, 152.15, 265.45);
            ctx.qcurve_to(158.42, 269.45, 165.70, 270.98);
            ctx.qcurve_to(172.97, 272.52, 180.32, 271.40);
            ctx.line_to(198.79, 268.59);
            ctx.line_to(198.83, 268.58);
            ctx.qcurve_to(215.60, 266.01, 225.64, 252.34);
            ctx.qcurve_to(235.68, 238.66, 233.11, 221.89);
            ctx.qcurve_to(232.88, 220.35, 232.63, 218.81);
            ctx.line_to(248.18, 211.12);
            ctx.line_to(248.21, 211.11);
            ctx.qcurve_to(263.41, 203.59, 268.84, 187.51);
            ctx.qcurve_to(274.27, 171.44, 266.74, 156.23);
            ctx.qcurve_to(262.68, 148.04, 257.67, 140.39);
            ctx.qcurve_to(258.08, 139.88, 258.48, 139.35);
            ctx.qcurve_to(258.87, 138.83, 259.25, 138.29);
            ctx.line_to(269.93, 122.97);
            ctx.qcurve_to(277.99, 111.43, 277.25, 97.36);
            ctx.qcurve_to(276.50, 83.30, 267.27, 72.67);
            ctx.qcurve_to(267.91, 71.51, 268.47, 70.30);
            ctx.qcurve_to(269.04, 69.10, 269.52, 67.86);
            ctx.line_to(276.33, 50.46);
            ctx.line_to(276.35, 50.40);
            ctx.qcurve_to(279.31, 42.81, 279.14, 34.67);
            ctx.qcurve_to(278.97, 26.52, 275.69, 19.06);
            ctx.qcurve_to(272.55, 11.90, 266.98, 6.40);
            ctx.qcurve_to(261.41, 0.89, 254.21, -2.17);
            ctx.qcurve_to(254.51, -3.41, 254.73, -4.66);
            ctx.qcurve_to(254.96, -5.92, 255.10, -7.18);
            ctx.line_to(257.23, -25.75);
            ctx.qcurve_to(258.22, -34.46, 255.57, -42.82);
            ctx.qcurve_to(252.91, -51.18, 247.06, -57.71);
            ctx.qcurve_to(247.37, -59.47, 247.53, -61.25);
            ctx.qcurve_to(247.68, -63.03, 247.68, -64.81);
            ctx.line_to(247.68, -83.54);
            ctx.qcurve_to(247.67, -96.77, 239.92, -107.49);
            ctx.qcurve_to(232.17, -118.21, 219.62, -122.38);
            ctx.line_to(219.52, -129.09);
            ctx.qcurve_to(219.26, -146.05, 207.09, -157.86);
            ctx.qcurve_to(194.92, -169.68, 177.96, -169.43);
            ctx.qcurve_to(172.94, -169.35, 167.85, -169.16);
            ctx.line_to(167.84, -169.31);
            ctx.line_to(167.84, -169.34);
            ctx.qcurve_to(167.26, -186.29, 154.87, -197.87);
            ctx.qcurve_to(142.47, -209.45, 125.51, -208.88);
            ctx.qcurve_to(117.74, -208.61, 109.85, -208.00);
            ctx.qcurve_to(106.05, -222.95, 93.34, -231.68);
            ctx.qcurve_to(80.63, -240.40, 65.31, -238.58);
            ctx.qcurve_to(45.33, -236.20, 25.95, -232.73);
            ctx.qcurve_to(22.07, -247.33, 9.63, -255.91);
            ctx.qcurve_to(-2.81, -264.48, -17.83, -262.93);
            ctx.qcurve_to(-44.96, -260.13, -75.59, -250.78);
            ctx.qcurve_to(-79.88, -250.32, -83.97, -248.97);
            ctx.qcurve_to(-88.07, -247.62, -91.80, -245.44);
            ctx.qcurve_to(-103.36, -241.40, -116.91, -235.99);
            ctx.qcurve_to(-150.38, -254.09, -185.96, -269.59);
            ctx.qcurve_to(-192.91, -272.62, -200.49, -272.95);
            ctx.qcurve_to(-208.07, -273.29, -215.27, -270.89);
            ctx.qcurve_to(-216.07, -270.62, -216.85, -270.33);
            ctx.qcurve_to(-217.64, -270.03, -218.41, -269.70);
            ctx.qcurve_to(-261.32, -251.90, -273.54, -218.49);
            ctx.fill();
            ctx.set_fill(0xffa88642);
            ctx.begin_path();
            ctx.move_to(-230.34, -169.53);
            ctx.bcurve_to(-242.75, -194.66, -239.90, -216.60, -202.31, -232.03);
            ctx.line_to(-202.31, -232.03);
            ctx.bcurve_to(-175.35, -220.29, -147.49, -206.55, -119.56, -190.60);
            ctx.bcurve_to(-103.63, -197.35, -87.58, -203.79, -71.38, -209.13);
            ctx.line_to(-71.16, -210.06);
            ctx.bcurve_to(-70.64, -209.95, -70.14, -209.84, -69.63, -209.72);
            ctx.bcurve_to(-51.16, -215.71, -32.51, -220.24, -13.63, -222.19);
            ctx.line_to(-11.69, -203.60);
            ctx.bcurve_to(-18.65, -202.88, -25.65, -201.75, -32.69, -200.28);
            ctx.bcurve_to(-18.86, -196.28, -5.65, -191.88, 6.97, -187.13);
            ctx.bcurve_to(28.05, -191.87, 49.12, -195.40, 70.16, -197.91);
            ctx.line_to(72.38, -179.35);
            ctx.bcurve_to(60.85, -177.97, 49.33, -176.28, 37.81, -174.25);
            ctx.bcurve_to(47.45, -169.82, 56.68, -165.19, 65.50, -160.31);
            ctx.bcurve_to(85.48, -164.46, 105.90, -167.22, 126.91, -167.94);
            ctx.line_to(127.53, -149.28);
            ctx.bcurve_to(114.77, -148.85, 102.19, -147.59, 89.72, -145.69);
            ctx.bcurve_to(100.48, -138.62, 110.54, -131.19, 119.88, -123.44);
            ctx.bcurve_to(139.03, -126.34, 158.55, -128.18, 178.56, -128.47);
            ctx.line_to(178.85, -109.78);
            ctx.bcurve_to(165.10, -109.58, 151.54, -108.61, 138.13, -107.03);
            ctx.bcurve_to(146.65, -98.70, 154.45, -90.07, 161.53, -81.16);
            ctx.bcurve_to(176.57, -82.67, 191.64, -83.48, 206.72, -83.50);
            ctx.line_to(206.72, -64.81);
            ctx.bcurve_to(195.94, -64.80, 185.15, -64.36, 174.35, -63.53);
            ctx.bcurve_to(178.70, -57.00, 182.72, -50.36, 186.38, -43.59);
            ctx.bcurve_to(188.32, -39.99, 190.16, -36.35, 191.91, -32.68);
            ctx.bcurve_to(200.12, -32.12, 208.33, -31.35, 216.53, -30.40);
            ctx.line_to(214.41, -11.84);
            ctx.bcurve_to(209.66, -12.39, 204.90, -12.86, 200.16, -13.28);
            ctx.bcurve_to(204.99, -0.35, 208.65, 12.87, 211.19, 26.25);
            ctx.bcurve_to(220.15, 28.94, 229.14, 32.00, 238.19, 35.53);
            ctx.line_to(231.38, 52.94);
            ctx.bcurve_to(225.61, 50.69, 219.89, 48.60, 214.19, 46.72);
            ctx.bcurve_to(215.56, 59.97, 215.87, 73.34, 215.16, 86.75);
            ctx.bcurve_to(222.29, 90.45, 229.37, 94.66, 236.35, 99.53);
            ctx.line_to(225.66, 114.84);
            ctx.bcurve_to(221.59, 112.00, 217.47, 109.40, 213.31, 107.00);
            ctx.bcurve_to(211.78, 118.85, 209.49, 130.70, 206.41, 142.47);
            ctx.bcurve_to(215.97, 151.68, 224.08, 162.38, 230.03, 174.41);
            ctx.line_to(213.28, 182.69);
            ctx.bcurve_to(209.82, 175.69, 205.38, 169.15, 200.19, 163.16);
            ctx.bcurve_to(196.27, 174.68, 191.60, 186.11, 186.25, 197.37);
            ctx.bcurve_to(188.93, 207.61, 191.06, 217.85, 192.63, 228.09);
            ctx.line_to(174.16, 230.91);
            ctx.bcurve_to(172.83, 222.25, 171.08, 213.58, 168.91, 204.91);
            ctx.bcurve_to(160.54, 204.65, 152.23, 204.20, 144.00, 203.56);
            ctx.bcurve_to(143.81, 210.75, 143.82, 218.00, 144.00, 225.31);
            ctx.line_to(125.31, 225.75);
            ctx.bcurve_to(125.12, 217.76, 125.12, 209.77, 125.34, 201.81);
            ctx.bcurve_to(115.61, 200.71, 106.02, 199.31, 96.53, 197.66);
            ctx.bcurve_to(95.94, 204.19, 95.46, 210.76, 95.16, 217.34);
            ctx.line_to(76.50, 216.47);
            ctx.bcurve_to(76.85, 208.96, 77.40, 201.51, 78.09, 194.10);
            ctx.bcurve_to(66.82, 191.67, 55.75, 188.82, 44.94, 185.60);
            ctx.bcurve_to(43.99, 190.82, 43.04, 196.05, 42.06, 201.28);
            ctx.line_to(23.69, 197.84);
            ctx.bcurve_to(24.80, 191.86, 25.88, 185.85, 26.97, 179.84);
            ctx.bcurve_to(15.32, 175.79, 3.99, 171.23, -6.97, 166.22);
            ctx.bcurve_to(-8.51, 174.30, -10.01, 182.39, -11.50, 190.47);
            ctx.line_to(-29.88, 187.10);
            ctx.bcurve_to(-28.07, 177.30, -26.25, 167.49, -24.38, 157.69);
            ctx.bcurve_to(-36.05, 151.58, -47.24, 144.90, -57.91, 137.63);
            ctx.bcurve_to(-59.44, 146.09, -60.93, 154.57, -62.38, 163.06);
            ctx.line_to(-80.81, 159.94);
            ctx.bcurve_to(-78.85, 148.41, -76.80, 136.89, -74.66, 125.38);
            ctx.bcurve_to(-84.83, 117.40, -94.38, 108.82, -103.31, 99.66);
            ctx.bcurve_to(-105.78, 111.96, -108.18, 124.28, -110.56, 136.59);
            ctx.line_to(-128.91, 133.03);
            ctx.bcurve_to(-125.64, 116.14, -122.30, 99.24, -118.84, 82.31);
            ctx.bcurve_to(-126.58, 72.92, -133.71, 63.02, -140.13, 52.59);
            ctx.bcurve_to(-141.79, 62.81, -143.39, 73.06, -145.00, 83.31);
            ctx.line_to(-163.47, 80.41);
            ctx.bcurve_to(-160.61, 62.22, -157.70, 43.99, -154.56, 25.78);
            ctx.bcurve_to(-162.84, 8.07, -169.33, -10.85, -173.78, -30.97);
            ctx.bcurve_to(-176.87, -18.73, -179.80, -6.39, -182.69, 6.00);
            ctx.line_to(-200.88, 1.75);
            ctx.bcurve_to(-194.65, -24.93, -188.20, -51.67, -180.28, -78.00);
            ctx.bcurve_to(-180.34, -78.89, -180.39, -79.77, -180.44, -80.66);
            ctx.line_to(-179.47, -80.72);
            ctx.bcurve_to(-172.47, -103.66, -164.33, -126.26, -154.19, -148.19);
            ctx.bcurve_to(-179.16, -158.00, -204.57, -165.14, -230.34, -169.53);
            ctx.move_to(-25.91, -178.72);
            ctx.bcurve_to(-39.40, -182.96, -53.58, -186.83, -68.44, -190.28);
            ctx.line_to(-68.44, -190.28);
            ctx.bcurve_to(-78.68, -186.81, -89.00, -182.86, -99.38, -178.66);
            ctx.bcurve_to(-90.90, -173.48, -82.42, -168.10, -74.00, -162.50);
            ctx.bcurve_to(-57.97, -168.72, -41.93, -174.11, -25.91, -178.72);
            ctx.move_to(5.64, -167.66);
            ctx.bcurve_to(5.64, -167.66, 5.63, -167.65, 5.63, -167.66);
            ctx.line_to(5.63, -167.66);
            ctx.bcurve_to(-14.67, -162.88, -34.94, -156.89, -55.22, -149.56);
            ctx.bcurve_to(-46.91, -143.66, -38.64, -137.56, -30.47, -131.22);
            ctx.bcurve_to(-8.04, -139.55, 14.63, -147.37, 37.84, -153.66);
            ctx.bcurve_to(27.65, -158.60, 16.92, -163.27, 5.66, -167.66);
            ctx.bcurve_to(5.65, -167.66, 5.65, -167.66, 5.64, -167.66);
            ctx.move_to(-136.94, -140.94);
            ctx.bcurve_to(-146.66, -119.98, -154.57, -98.17, -161.41, -75.81);
            ctx.bcurve_to(-159.70, -51.76, -155.24, -29.33, -148.31, -8.53);
            ctx.bcurve_to(-140.32, -49.57, -130.31, -90.46, -115.69, -130.75);
            ctx.bcurve_to(-122.72, -134.37, -129.82, -137.76, -136.94, -140.94);
            ctx.move_to(95.19, -119.13);
            ctx.bcurve_to(85.12, -126.62, 74.26, -133.80, 62.56, -140.56);
            ctx.bcurve_to(36.94, -134.78, 11.74, -126.62, -13.40, -117.53);
            ctx.bcurve_to(-5.49, -110.98, 2.33, -104.22, 10.03, -97.22);
            ctx.bcurve_to(37.95, -105.53, 66.19, -113.36, 95.19, -119.13);
            ctx.move_to(-71.44, -104.72);
            ctx.bcurve_to(-80.56, -110.77, -89.76, -116.46, -99.06, -121.75);
            ctx.bcurve_to(-115.97, -74.44, -126.56, -25.84, -135.19, 23.38);
            ctx.bcurve_to(-129.04, 35.76, -121.89, 47.43, -113.87, 58.44);
            ctx.bcurve_to(-102.36, 4.19, -89.13, -50.22, -71.44, -104.72);
            ctx.move_to(139.44, -78.47);
            ctx.bcurve_to(131.88, -87.13, 123.53, -95.53, 114.38, -103.59);
            ctx.line_to(114.38, -103.59);
            ctx.bcurve_to(84.47, -98.53, 55.16, -90.83, 25.88, -82.31);
            ctx.bcurve_to(33.85, -74.56, 41.70, -66.58, 49.34, -58.31);
            ctx.bcurve_to(79.28, -67.12, 109.31, -74.12, 139.44, -78.47);
            ctx.move_to(-22.37, -67.75);
            ctx.bcurve_to(-33.21, -76.95, -44.20, -85.58, -55.34, -93.62);
            ctx.bcurve_to(-73.50, -36.79, -86.93, 20.16, -98.75, 77.28);
            ctx.bcurve_to(-90.13, 87.00, -80.74, 96.09, -70.65, 104.56);
            ctx.bcurve_to(-59.15, 46.57, -44.45, -11.13, -22.37, -67.75);
            ctx.move_to(169.94, -34.69);
            ctx.bcurve_to(165.02, -43.79, 159.40, -52.71, 153.06, -61.41);
            ctx.bcurve_to(123.19, -57.76, 93.22, -51.30, 63.16, -42.84);
            ctx.bcurve_to(67.65, -37.64, 72.07, -32.34, 76.41, -26.94);
            ctx.bcurve_to(101.98, -31.44, 127.56, -33.67, 153.09, -33.88);
            ctx.bcurve_to(158.89, -33.92, 164.68, -33.84, 170.47, -33.69);
            ctx.bcurve_to(170.29, -34.02, 170.12, -34.35, 169.94, -34.69);
            ctx.move_to(21.07, -26.87);
            ctx.bcurve_to(11.71, -36.57, 2.23, -45.82, -7.41, -54.59);
            ctx.bcurve_to(-28.73, 1.60, -42.90, 59.16, -54.13, 117.44);
            ctx.bcurve_to(-43.57, 125.07, -32.39, 132.13, -20.63, 138.53);
            ctx.bcurve_to(-9.69, 83.54, 3.17, 28.40, 21.07, -26.87);
            ctx.move_to(179.50, -14.69);
            ctx.bcurve_to(171.49, -15.06, 163.47, -15.24, 155.47, -15.22);
            ctx.bcurve_to(133.43, -15.15, 111.42, -13.49, 89.41, -10.13);
            ctx.bcurve_to(94.86, -2.81, 100.19, 4.65, 105.34, 12.31);
            ctx.bcurve_to(107.42, 12.27, 109.50, 12.23, 111.56, 12.22);
            ctx.bcurve_to(138.67, 12.05, 164.85, 14.76, 191.00, 20.84);
            ctx.bcurve_to(188.19, 8.79, 184.37, -3.08, 179.50, -14.69);
            ctx.move_to(66.34, 24.97);
            ctx.bcurve_to(56.34, 12.32, 46.12, 0.28, 35.72, -11.22);
            ctx.line_to(35.72, -11.22);
            ctx.bcurve_to(19.22, 41.49, 7.12, 94.33, -3.31, 147.31);
            ctx.bcurve_to(7.52, 152.42, 18.76, 157.03, 30.37, 161.16);
            ctx.bcurve_to(38.68, 115.57, 47.86, 69.58, 66.34, 24.97);
            ctx.move_to(194.75, 41.00);
            ctx.bcurve_to(169.21, 34.32, 143.84, 31.17, 117.31, 30.94);
            ctx.line_to(117.31, 30.94);
            ctx.bcurve_to(123.40, 40.80, 129.23, 50.92, 134.78, 61.34);
            ctx.bcurve_to(155.37, 65.37, 176.27, 70.11, 196.84, 78.34);
            ctx.bcurve_to(197.13, 65.80, 196.46, 53.32, 194.75, 41.00);
            ctx.move_to(101.44, 73.03);
            ctx.bcurve_to(94.28, 62.46, 87.00, 52.21, 79.59, 42.28);
            ctx.bcurve_to(64.33, 82.45, 56.09, 124.45, 48.31, 167.06);
            ctx.bcurve_to(58.69, 170.21, 69.32, 172.96, 80.19, 175.34);
            ctx.bcurve_to(84.74, 140.42, 92.56, 106.48, 101.44, 73.03);
            ctx.move_to(195.59, 98.09);
            ctx.bcurve_to(179.43, 91.04, 162.71, 86.42, 145.56, 82.66);
            ctx.line_to(145.56, 82.66);
            ctx.bcurve_to(150.27, 92.50, 154.68, 102.61, 158.87, 112.94);
            ctx.bcurve_to(169.94, 117.04, 180.61, 122.52, 190.37, 129.31);
            ctx.bcurve_to(192.76, 118.91, 194.51, 108.49, 195.59, 98.09);
            ctx.move_to(136.28, 128.75);
            ctx.bcurve_to(129.43, 116.88, 122.41, 105.34, 115.28, 94.13);
            ctx.bcurve_to(108.22, 122.18, 102.23, 150.35, 98.56, 178.97);
            ctx.bcurve_to(107.69, 180.58, 116.96, 181.94, 126.34, 183.03);
            ctx.bcurve_to(127.83, 164.70, 130.88, 146.55, 136.28, 128.75);
            ctx.move_to(185.13, 148.81);
            ctx.bcurve_to(179.87, 144.69, 174.22, 141.02, 168.31, 137.81);
            ctx.bcurve_to(171.85, 147.90, 175.14, 158.22, 178.16, 168.75);
            ctx.bcurve_to(180.73, 162.13, 183.07, 155.48, 185.13, 148.81);
            ctx.move_to(154.56, 161.91);
            ctx.bcurve_to(152.92, 158.79, 151.26, 155.70, 149.59, 152.63);
            ctx.bcurve_to(147.39, 163.18, 145.89, 173.92, 145.00, 184.84);
            ctx.bcurve_to(151.12, 185.32, 157.26, 185.71, 163.47, 185.97);
            ctx.bcurve_to(160.88, 177.96, 157.91, 169.93, 154.56, 161.90);
            ctx.fill();
            break;
        case PetalID::kRice:
            ctx.set_stroke(0xffcfcfcf);
            ctx.set_line_width(9);
            ctx.scale(r / 13);
            ctx.begin_path();
            ctx.move_to(-8,0);
            ctx.qcurve_to(0,-3.5,8,0);
            ctx.stroke();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(5);
            ctx.stroke();
            break;
        case PetalID::kBone:
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xffcfcfcf);
            ctx.set_line_width(5);
            ctx.scale(r / 12);
            ctx.begin_path();
            ctx.move_to(-10,-4);
            ctx.qcurve_to(0,0,10,-4);
            ctx.bcurve_to(14,-10,20,-2,14,0);
            ctx.bcurve_to(20,2,14,10,10,4);
            ctx.qcurve_to(0,0,-10,4);
            ctx.bcurve_to(-14,10,-20,2,-14,0);
            ctx.bcurve_to(-20,-2,-14,-10,-10,-4);
            ctx.stroke();
            ctx.fill();
            break;
        case PetalID::kYucca:
            ctx.set_fill(0xff74b53f);
            ctx.set_stroke(0xff5e9333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(14,0);
            ctx.qcurve_to(0,-12,-14,0);
            ctx.qcurve_to(0,12,14,0);
            ctx.fill();
            ctx.stroke();
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(14,0);
            ctx.qcurve_to(0,-3,-14,0);
            ctx.stroke();
            break;
        case PetalID::kCorn:
            ctx.scale(r / 10);
            ctx.set_fill(0xffffe419);
            ctx.set_stroke(0xffcfb914);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-5,8);
            ctx.qcurve_to(-15,-8,0,-8);
            ctx.qcurve_to(15,-8,5,8);
            ctx.qcurve_to(0,2,-5,8);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kIronGuard:
            ctx.scale(r / 17.f);
            ctx.set_fill(0xffA9B8C7);
            ctx.set_stroke(0xff35495E);
            ctx.set_line_width(3);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.round_rect(-12.5f, -15, 25, 30, 4);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff35495E);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(-9, -11, 18, 22, 3);
            ctx.stroke();
            ctx.set_fill(0xffE6C56C);
            ctx.set_stroke(0xff35495E);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 5);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kRustNeedle:
            ctx.scale(r / 10.f);
            ctx.set_fill(0xffC57A45);
            ctx.set_stroke(0xff6D402C);
            ctx.set_line_width(2.5f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.arc(-8, 0, 4);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-4, 3);
            ctx.line_to(10, 0);
            ctx.line_to(-4, -3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD8DADD);
            ctx.begin_path();
            ctx.move_to(10, 0);
            ctx.line_to(6, 1.5f);
            ctx.line_to(6, -1.5f);
            ctx.close_path();
            ctx.fill();
            break;
        case PetalID::kAncientCoin:
            ctx.scale(r / 13.f);
            ctx.set_fill(0xffE8B947);
            ctx.set_stroke(0xff8E6828);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 13);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffFFF0A3);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(0, 0, 9);
            ctx.stroke();
            ctx.set_stroke(0xff8E6828);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-6, 3);
            ctx.line_to(0, -5);
            ctx.line_to(6, 3);
            ctx.stroke();
            break;
        case PetalID::kRoyalRose:
            ctx.scale(r / 14.f);
            ctx.set_fill(0xff4F9A5B);
            ctx.set_stroke(0xff28583B);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(-10, 0, 6, 10);
            ctx.ellipse(10, 0, 6, 10);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD95363);
            ctx.set_stroke(0xff7C2C42);
            ctx.set_line_width(2.5f);
            for (uint32_t i = 0; i < 5; ++i) {
                float a = (float) i * 2.f * (float) M_PI / 5.f - (float) M_PI / 2.f;
                ctx.begin_path();
                ctx.arc(cosf(a) * 9.f, sinf(a) * 9.f, 6);
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_fill(0xff7C2C42);
            ctx.begin_path();
            ctx.arc(0, 0, 5);
            ctx.fill();
            break;
        case PetalID::kRoyalSpear:
            ctx.scale(r / 17.f);
            ctx.set_stroke(0xffC9A45A);
            ctx.set_line_width(4.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-23, 0);
            ctx.line_to(18, 0);
            ctx.stroke();
            ctx.set_fill(0xffD96558);
            ctx.set_stroke(0xff813B38);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(10, 5);
            ctx.line_to(10, -5);
            ctx.line_to(4, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE5E9ED);
            ctx.set_stroke(0xff77562F);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(25, 0);
            ctx.line_to(15, 5);
            ctx.line_to(15, -5);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kCrownFragment:
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE3B95C);
            ctx.set_stroke(0xff76552C);
            ctx.set_line_width(2.5f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.line_to(-12, -4);
            ctx.line_to(0, -12);
            ctx.line_to(14, -2);
            ctx.line_to(10, 12);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD84D5B);
            ctx.set_stroke(0xff76552C);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.arc(1, 2, 4);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kRoyalBanner:
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xffD6B46A);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-12, -18);
            ctx.line_to(-12, 18);
            ctx.stroke();
            ctx.set_fill(0xffC94D4C);
            ctx.set_stroke(0xff6E3034);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-12, -14);
            ctx.line_to(12, -10);
            ctx.line_to(8, 0);
            ctx.line_to(12, 10);
            ctx.line_to(-12, 14);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffF1D68A);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-4, -4);
            ctx.line_to(0, -8);
            ctx.line_to(4, -4);
            ctx.line_to(0, 0);
            ctx.close_path();
            ctx.stroke();
            break;
        case PetalID::kStoneHeart:
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff969C9A);
            ctx.set_stroke(0xff444D4A);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(0, 12);
            ctx.qcurve_to(-14, 4, -10, -6);
            ctx.qcurve_to(-5, -14, 0, -8);
            ctx.qcurve_to(5, -14, 10, -6);
            ctx.qcurve_to(14, 4, 0, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff6C7470);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-2, -2);
            ctx.line_to(4, 6);
            ctx.stroke();
            break;
        case PetalID::kPhantomEye:
            ctx.scale(r / 12.f);
            ctx.set_fill(0xffC8B5E8);
            ctx.set_stroke(0xff5D447D);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 14, 9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff58317F);
            ctx.begin_path();
            ctx.arc(2, 0, 6);
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(4, -2, 2);
            ctx.fill();
            break;
        case PetalID::kGuardEgg:
            ctx.scale(r / 14.f);
            ctx.set_fill(0xffD4DBE2);
            ctx.set_stroke(0xff697C8E);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 11, 15);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff527DB0);
            ctx.set_stroke(0xff697C8E);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(-5, -7, 10, 12, 3);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kThornTrap:
            ctx.scale(r / 14.f);
            ctx.set_fill(0xff75B96C);
            ctx.set_stroke(0xff28583B);
            ctx.set_line_width(2);
            ctx.round_line_join();
            for (uint32_t i = 0; i < 4; ++i) {
                float a = (float) i * (float) M_PI / 2.f;
                float bx = cosf(a) * 10.f;
                float by = sinf(a) * 10.f;
                float tx = cosf(a) * 18.f;
                float ty = sinf(a) * 18.f;
                float px = -sinf(a) * 4.f;
                float py = cosf(a) * 4.f;
                ctx.begin_path();
                ctx.move_to(bx + px, by + py);
                ctx.line_to(tx, ty);
                ctx.line_to(bx - px, by - py);
                ctx.close_path();
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff4D915B);
            ctx.set_stroke(0xff28583B);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 10);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA0C77A);
            ctx.begin_path();
            ctx.arc(0, 0, 5);
            ctx.fill();
            break;
        case PetalID::kKingsOath:
            ctx.scale(r / 16.f);
            ctx.set_fill(0xff913C4C);
            ctx.set_stroke(0xffE0B762);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(0, 0, 16);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffF1D68A);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-5, 3);
            ctx.line_to(0, -6);
            ctx.line_to(5, 3);
            ctx.stroke();
            break;
        case PetalID::kLeafBlade:
            ctx.scale(r / 12.f);
            ctx.set_fill(0xff65B85C);
            ctx.set_stroke(0xff2C743D);
            ctx.set_line_width(2.5f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 0);
            ctx.line_to(2, -7);
            ctx.line_to(14, 0);
            ctx.line_to(2, 7);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff2C743D);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-6, 0);
            ctx.line_to(10, 0);
            ctx.stroke();
            break;
        case PetalID::kJungleNeedle:
            ctx.scale(r / 10.f);
            ctx.set_fill(0xffB7C95A);
            ctx.set_stroke(0xff617A34);
            ctx.set_line_width(2);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 3);
            ctx.line_to(-10, -3);
            ctx.line_to(12, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff2C743D);
            ctx.begin_path();
            ctx.arc(-9, 0, 2.5f);
            ctx.fill();
            break;
        case PetalID::kEmeraldShell:
            ctx.scale(r / 17.f);
            ctx.set_fill(0xff4B9C74);
            ctx.set_stroke(0xff28604C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 2, 15, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff7BC99A);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(5, 0);
            ctx.line_to(0, 6);
            ctx.line_to(-5, 0);
            ctx.close_path();
            ctx.fill();
            break;
        case PetalID::kVineWhip:
            ctx.scale(r / 16.f);
            ctx.set_stroke(0xff245D35);
            ctx.set_fill(0xff438C49);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-12, 4);
            ctx.line_to(-2, -2);
            ctx.line_to(8, 2);
            ctx.stroke();
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.ellipse(12, 0, 6, 4);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kSpiderFang:
            ctx.scale(r / 13.f);
            ctx.set_fill(0xffE6E6D8);
            ctx.set_stroke(0xff777D72);
            ctx.set_line_width(2.5f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-8, 6);
            ctx.qcurve_to(-2, -10, 12, -2);
            ctx.line_to(4, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kPoisonSpore:
            ctx.scale(r / 12.f);
            ctx.set_fill(0xff9D79B6);
            ctx.set_stroke(0xff56456F);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC4A3D6);
            ctx.begin_path();
            ctx.arc(8, -6, 3);
            ctx.arc(-7, -7, 2.5f);
            ctx.arc(2, 9, 2.5f);
            ctx.fill();
            break;
        case PetalID::kThornRing:
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff5E9F55);
            ctx.set_stroke(0xff2C6135);
            ctx.set_line_width(2);
            ctx.round_line_join();
            for (uint32_t i = 0; i < 6; ++i) {
                float a = (float)i * (float)M_PI / 3.f;
                float bx = cosf(a) * 8.f, by = sinf(a) * 8.f;
                float tx = cosf(a) * 15.f, ty = sinf(a) * 15.f;
                float px = -sinf(a) * 2.5f, py = cosf(a) * 2.5f;
                ctx.begin_path();
                ctx.move_to(bx + px, by + py);
                ctx.line_to(tx, ty);
                ctx.line_to(bx - px, by - py);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.stroke();
            break;
        case PetalID::kMantisClaw:
            ctx.scale(r / 17.f);
            ctx.set_fill(0xff7CCB61);
            ctx.set_stroke(0xff347B3C);
            ctx.set_line_width(3);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 8);
            ctx.qcurve_to(-4, -12, 14, -4);
            ctx.line_to(10, 2);
            ctx.qcurve_to(0, -2, -6, 10);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kToxicHeart:
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff5FAF8B);
            ctx.set_stroke(0xff2D6D59);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(0, 12);
            ctx.bcurve_to(-16, 2, -12, -10, 0, -4);
            ctx.bcurve_to(12, -10, 16, 2, 0, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE6D35A);
            ctx.begin_path();
            ctx.arc(0, 0, 4);
            ctx.fill();
            break;
        case PetalID::kSporeCloud:
            ctx.scale(r / 16.f);
            ctx.set_fill(0xffA98DC2);
            ctx.set_stroke(0xff635077);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(-6, -2, 8);
            ctx.arc(5, -4, 7);
            ctx.arc(0, 5, 9);
            ctx.arc(8, 4, 5);
            ctx.arc(-8, 5, 5);
            ctx.fill();
            ctx.stroke();
            break;
        case PetalID::kAncientSeed:
            ctx.scale(r / 14.f);
            ctx.set_fill(0xff8B6D47);
            ctx.set_stroke(0xff4F402C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 2, 9, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff65B85C);
            ctx.begin_path();
            ctx.arc(0, -2, 3);
            ctx.fill();
            ctx.set_stroke(0xff2C743D);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-4, -6);
            ctx.line_to(0, -10);
            ctx.line_to(4, -6);
            ctx.stroke();
            break;
        case PetalID::kHeartwood:
            ctx.scale(r / 18.f);
            ctx.set_fill(0xff8A633F);
            ctx.set_stroke(0xff493823);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 16);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff6B4A2E);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 11);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0, 0, 7);
            ctx.stroke();
            ctx.set_fill(0xff7CCB61);
            ctx.begin_path();
            ctx.arc(0, 0, 4);
            ctx.fill();
            break;
        case PetalID::kCoin:
            ctx.scale(r / 12.f);
            ctx.set_fill(0xffF6D56A);
            ctx.set_stroke(0xff8A5A12);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE2B23A);
            ctx.begin_path();
            ctx.arc(0, 0, 8.5f);
            ctx.fill();
            ctx.set_fill(0xffFFF4C2);
            ctx.begin_path();
            ctx.ellipse(0, -2.2f, 3.2f, 5.2f);
            ctx.fill();
            ctx.rotate(2.2f);
            ctx.begin_path();
            ctx.ellipse(0, -2.2f, 3.2f, 5.2f);
            ctx.fill();
            ctx.rotate(2.2f);
            ctx.begin_path();
            ctx.ellipse(0, -2.2f, 3.2f, 5.2f);
            ctx.fill();
            ctx.set_fill(0xffC98418);
            ctx.begin_path();
            ctx.arc(0, 0, 1.6f);
            ctx.fill();
            break;
        case PetalID::kMonstera: {
            // Fenestrated monstera leaf — deep jungle green with oval holes.
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff2F8A4A);
            ctx.set_stroke(0xff1B5A32);
            ctx.set_line_width(2.8f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-12, 2);
            ctx.bcurve_to(-10, -14, 2, -18, 12, -10);
            ctx.bcurve_to(16, -4, 14, 8, 6, 14);
            ctx.bcurve_to(-2, 18, -12, 12, -12, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Midrib
            ctx.set_stroke(0xff1B5A32);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-8, 2);
            ctx.qcurve_to(0, -2, 10, -6);
            ctx.stroke();
            // Fenestrations (characteristic holes)
            ctx.set_fill(0xff1E6B3A);
            ctx.begin_path();
            ctx.ellipse(-2, -4, 2.6f, 3.8f);
            ctx.fill();
            ctx.begin_path();
            ctx.ellipse(5, -2, 2.2f, 3.2f);
            ctx.fill();
            ctx.begin_path();
            ctx.ellipse(1, 6, 2.4f, 2.8f);
            ctx.fill();
            // Edge notches
            ctx.set_stroke(0xff3FA85C);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(8, -8);
            ctx.line_to(11, -5);
            ctx.move_to(-6, 10);
            ctx.line_to(-3, 13);
            ctx.stroke();
            break;
        }
        // --- Batch 1 materials (01–20) from todo/gpt给出的缺失花瓣美术.txt ---
        case PetalID::kMatLeaf: {
            // Torn leaf scrap — not a full Leaf petal
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff65AE59);
            ctx.set_stroke(0xff315D35);
            ctx.set_line_width(2.6f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 2);
            ctx.bcurve_to(-8, -12, 4, -14, 11, -4);
            ctx.line_to(9, 0);
            ctx.bcurve_to(6, 10, -6, 12, -10, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffC5E58D);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-6, 1);
            ctx.qcurve_to(0, -2, 7, -3);
            ctx.move_to(-2, -4);
            ctx.line_to(1, 2);
            ctx.move_to(2, -1);
            ctx.line_to(4, 4);
            ctx.stroke();
            break;
        }
        case PetalID::kMatTwig: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff63442E);
            ctx.set_fill(0xff765136);
            ctx.set_line_width(4.2f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 8);
            ctx.qcurve_to(-2, 2, 10, -8);
            ctx.stroke();
            ctx.set_line_width(3.0f);
            ctx.begin_path();
            ctx.move_to(1, -1);
            ctx.line_to(6, 4);
            ctx.move_to(4, -4);
            ctx.line_to(8, -1);
            ctx.stroke();
            ctx.set_fill(0xffB38A5D);
            ctx.begin_path();
            ctx.arc(10, -8, 1.8f);
            ctx.fill();
            break;
        }
        case PetalID::kMatBeeBarb: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE7B647);
            ctx.set_stroke(0xff5D4732);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(12, 0);
            ctx.line_to(-6, -5);
            ctx.line_to(-10, -2);
            ctx.line_to(-6, 1);
            ctx.line_to(-9, 4);
            ctx.line_to(-5, 5);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff34332C);
            ctx.begin_path();
            ctx.arc(-9, 0, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatPollenDust: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffF3CC57);
            ctx.set_stroke(0xff9D6A26);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.arc(-2, -1, 5.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD69A35);
            ctx.begin_path();
            ctx.arc(3, 2, 4.2f);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(-5, 4, 2.6f);
            ctx.fill();
            ctx.set_fill(0xffF3CC57);
            ctx.begin_path();
            ctx.arc(7, -4, 2.4f);
            ctx.arc(8, 5, 1.8f);
            ctx.arc(-8, -5, 1.6f);
            ctx.fill();
            ctx.set_fill(0x99ffffff);
            ctx.begin_path();
            ctx.arc(-3, -3, 1.4f);
            ctx.fill();
            break;
        }
        case PetalID::kMatRoyalJelly: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB95448);
            ctx.set_stroke(0xff693C39);
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE8AC4D);
            ctx.begin_path();
            ctx.arc(0, 0, 7.5f);
            ctx.fill();
            ctx.set_fill(0xffF7D576);
            ctx.begin_path();
            ctx.ellipse(-2.5f, -3.5f, 3.2f, 2.4f);
            ctx.fill();
            break;
        }
        case PetalID::kMatAncientHeart: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff77533D);
            ctx.set_stroke(0xff3B3D2C);
            ctx.set_line_width(2.6f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, 11);
            ctx.bcurve_to(12, 4, 11, -8, 0, -6);
            ctx.bcurve_to(-11, -8, -12, 4, 0, 11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff76AD62);
            ctx.begin_path();
            ctx.move_to(0, 6);
            ctx.bcurve_to(7, 1, 6, -5, 0, -3);
            ctx.bcurve_to(-6, -5, -7, 1, 0, 6);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffC4E49B);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(0, -2);
            ctx.line_to(0, 5);
            ctx.move_to(0, 1);
            ctx.line_to(-3, 3);
            ctx.move_to(0, 1);
            ctx.line_to(3, 3);
            ctx.stroke();
            break;
        }
        case PetalID::kMatSand: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE8BC72);
            ctx.set_stroke(0xffA87946);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-8, -2);
            ctx.line_to(-4, -9);
            ctx.line_to(5, -8);
            ctx.line_to(10, -1);
            ctx.line_to(6, 8);
            ctx.line_to(-5, 9);
            ctx.line_to(-10, 3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF6DB9A);
            ctx.begin_path();
            ctx.move_to(-4, -6);
            ctx.line_to(2, -5);
            ctx.line_to(-1, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatVenomBarb: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC99A53);
            ctx.set_stroke(0xff70472E);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(11, -2);
            ctx.line_to(-4, -6);
            ctx.line_to(-8, -1);
            ctx.line_to(-5, 1);
            ctx.line_to(-9, 5);
            ctx.line_to(-4, 6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff7E9C4B);
            ctx.begin_path();
            ctx.move_to(-5, 1);
            ctx.line_to(-9, 5);
            ctx.line_to(-4, 6);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatCactusSap: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff79AE65);
            ctx.set_stroke(0xff3F7046);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.bcurve_to(8, -4, 9, 6, 0, 11);
            ctx.bcurve_to(-9, 6, -8, -4, 0, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD6ECA3);
            ctx.begin_path();
            ctx.ellipse(-3, -3, 2.2f, 4.5f);
            ctx.fill();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.arc(2, 2, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kMatCarapace: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA9764B);
            ctx.set_stroke(0xff604332);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-10, 4);
            ctx.bcurve_to(-10, -8, 10, -8, 10, 4);
            ctx.line_to(6, 9);
            ctx.line_to(-2, 6);
            ctx.line_to(-8, 9);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff855B42);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-3, -4);
            ctx.line_to(-3, 5);
            ctx.move_to(3, -4);
            ctx.line_to(3, 5);
            ctx.stroke();
            ctx.set_fill(0xffD3A16D);
            ctx.begin_path();
            ctx.ellipse(0, -4, 6.f, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSandstormEye: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC89451);
            ctx.set_stroke(0xffA86D38);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffF4D18B);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 7.5f, 0.4f, 4.5f, 0);
            ctx.stroke();
            ctx.begin_path();
            ctx.partial_arc(0, 0, 4.5f, 2.2f, 5.8f, 0);
            ctx.stroke();
            ctx.set_fill(0xff4F453C);
            ctx.begin_path();
            ctx.arc(0, 0, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSandwormCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff9C6A43);
            ctx.set_stroke(0xff5A3D30);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9.f, 11.f);
            ctx.fill();
            ctx.stroke();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                ctx.begin_path();
                ctx.arc(std::cos(a) * 8.2f, std::sin(a) * 9.5f, 2.4f);
                ctx.set_fill(0xff9C6A43);
                ctx.fill();
                ctx.set_stroke(0xff5A3D30);
                ctx.set_line_width(1.6f);
                ctx.stroke();
            }
            ctx.set_fill(0xffE4B66D);
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.line_to(4, 0);
            ctx.line_to(0, 5);
            ctx.line_to(-4, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF6D996);
            ctx.begin_path();
            ctx.arc(0, 0, 2.f);
            ctx.fill();
            break;
        }
        case PetalID::kMatWaterDrop: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff48B9D5);
            ctx.set_stroke(0xff267A9C);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.bcurve_to(8, -2, 8, 8, 0, 11);
            ctx.bcurve_to(-8, 8, -8, -2, 0, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD0F4F6);
            ctx.begin_path();
            ctx.ellipse(-2.5f, -3.5f, 2.0f, 4.0f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSeaSalt: {
            ctx.scale(r / 15.f);
            auto salt = [&](float x, float y, float s) {
                ctx.set_fill(0xffE4F1EB);
                ctx.set_stroke(0xff79A6AD);
                ctx.set_line_width(1.8f);
                ctx.begin_path();
                ctx.move_to(x, y - s);
                ctx.line_to(x + s * 0.85f, y - s * 0.3f);
                ctx.line_to(x + s * 0.55f, y + s * 0.75f);
                ctx.line_to(x - s * 0.55f, y + s * 0.75f);
                ctx.line_to(x - s * 0.85f, y - s * 0.3f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
                ctx.set_fill(0xffffffff);
                ctx.begin_path();
                ctx.move_to(x - s * 0.2f, y - s * 0.4f);
                ctx.line_to(x + s * 0.25f, y - s * 0.15f);
                ctx.line_to(x - s * 0.05f, y + s * 0.1f);
                ctx.close_path();
                ctx.fill();
            };
            salt(0, -4, 5.5f);
            salt(-5, 5, 4.2f);
            salt(5, 6, 3.8f);
            break;
        }
        case PetalID::kMatSeashell: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffF2B7A2);
            ctx.set_stroke(0xff956F8E);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, 9);
            ctx.bcurve_to(-12, 2, -10, -8, 0, -10);
            ctx.bcurve_to(10, -8, 12, 2, 0, 9);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffC67F9D);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(0, 7);
            ctx.qcurve_to(-6, 0, -4, -6);
            ctx.move_to(0, 7);
            ctx.qcurve_to(-2, -2, 0, -8);
            ctx.move_to(0, 7);
            ctx.qcurve_to(2, -2, 0, -8);
            ctx.move_to(0, 7);
            ctx.qcurve_to(6, 0, 4, -6);
            ctx.stroke();
            ctx.set_fill(0xffFFE1C9);
            ctx.begin_path();
            ctx.ellipse(-2, -4, 2.5f, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kMatCoral: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff7D507E);
            ctx.set_fill(0xffD891B4);
            ctx.set_line_width(5.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(0, 10);
            ctx.line_to(0, -2);
            ctx.move_to(0, 2);
            ctx.line_to(-7, -6);
            ctx.move_to(0, 0);
            ctx.line_to(7, -5);
            ctx.move_to(0, -2);
            ctx.line_to(-3, -10);
            ctx.move_to(0, -2);
            ctx.line_to(4, -11);
            ctx.stroke();
            ctx.set_fill(0xffF4BED0);
            ctx.begin_path();
            ctx.arc(-7, -6, 2.4f);
            ctx.arc(7, -5, 2.4f);
            ctx.arc(-3, -10, 2.2f);
            ctx.arc(4, -11, 2.2f);
            ctx.arc(0, -2, 2.0f);
            ctx.fill();
            break;
        }
        case PetalID::kMatAbyssPearl: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff244D70);
            ctx.set_stroke(0xff244D70);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.set_fill(0xffB6C7DF);
            ctx.begin_path();
            ctx.arc(0, 0, 8.5f);
            ctx.fill();
            ctx.set_fill(0xff718AB7);
            ctx.begin_path();
            ctx.ellipse(2.5f, 3.5f, 3.5f, 2.5f);
            ctx.fill();
            ctx.set_fill(0xffF3F4FF);
            ctx.begin_path();
            ctx.ellipse(-3, -3.5f, 3.0f, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatTideCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff226A8C);
            ctx.set_stroke(0xff123D5B);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f;
                float py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py);
                else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff42B7C8);
            ctx.begin_path();
            ctx.move_to(0, -6);
            ctx.line_to(5, 0);
            ctx.line_to(0, 6);
            ctx.line_to(-5, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffB4EDF0);
            ctx.begin_path();
            ctx.arc(0, 0, 2.8f);
            ctx.fill();
            break;
        }
        case PetalID::kMatIceCrystal: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff83C4DC);
            ctx.set_stroke(0xff467F9F);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -12);
            ctx.line_to(5, -4);
            ctx.line_to(5, 4);
            ctx.line_to(0, 12);
            ctx.line_to(-5, 4);
            ctx.line_to(-5, -4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD2F0F8);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(3, -3);
            ctx.line_to(0, 2);
            ctx.line_to(-1, -3);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-2, -8);
            ctx.line_to(1, -6);
            ctx.move_to(1, -9);
            ctx.line_to(3, -7);
            ctx.stroke();
            break;
        }
        case PetalID::kMatSnowFluff: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffEAF5F8);
            ctx.set_stroke(0xff9EC8D8);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.arc(-3, -2, 6.5f);
            ctx.arc(4, -1, 5.5f);
            ctx.arc(0, 4, 6.0f);
            ctx.arc(-6, 3, 4.0f);
            ctx.arc(6, 4, 3.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.ellipse(-4, -4, 2.8f, 1.8f);
            ctx.ellipse(3, -3, 2.2f, 1.4f);
            ctx.fill();
            ctx.set_fill(0xffC3E2EC);
            ctx.begin_path();
            ctx.ellipse(0, 6, 4.5f, 1.6f);
            ctx.fill();
            break;
        }
        // --- Batch 2 materials (21–40) ---
        case PetalID::kMatColdIron: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff748DA1);
            ctx.set_stroke(0xff364D63);
            ctx.set_line_width(2.6f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-9, -8);
            ctx.line_to(4, -10);
            ctx.line_to(10, -2);
            ctx.line_to(7, 9);
            ctx.line_to(-4, 10);
            ctx.line_to(-11, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff536F88);
            ctx.begin_path();
            ctx.move_to(4, -10);
            ctx.line_to(10, -2);
            ctx.line_to(7, 9);
            ctx.line_to(1, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffB8D2E1);
            ctx.begin_path();
            ctx.move_to(-7, -6);
            ctx.line_to(1, -7);
            ctx.line_to(-2, -1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffE7F7FC);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-5, 2);
            ctx.line_to(2, -1);
            ctx.move_to(5, -4);
            ctx.line_to(8, -1);
            ctx.stroke();
            break;
        }
        case PetalID::kMatIceScale: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8FC7DC);
            ctx.set_stroke(0xff427C9D);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.bcurve_to(9, -6, 10, 4, 0, 11);
            ctx.bcurve_to(-10, 4, -9, -6, 0, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD2F0F7);
            ctx.begin_path();
            ctx.move_to(0, -9);
            ctx.bcurve_to(-6, -4, -5, 2, 0, 4);
            ctx.bcurve_to(-1, -2, -1, -6, 0, -9);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff6CA6C5);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.bcurve_to(5, 0, 5, 6, 0, 9);
            ctx.bcurve_to(1, 4, 1, 0, 0, -4);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff427C9D);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(0, -8);
            ctx.line_to(0, 8);
            ctx.move_to(-3, 4);
            ctx.qcurve_to(0, 6, 3, 4);
            ctx.move_to(-2, 7);
            ctx.qcurve_to(0, 8.5f, 2, 7);
            ctx.stroke();
            break;
        }
        case PetalID::kMatPermafrostHeart: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff5A91B2);
            ctx.set_stroke(0xff294D73);
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.move_to(0, 11);
            ctx.bcurve_to(12, 4, 10, -8, 0, -7);
            ctx.bcurve_to(-10, -8, -12, 4, 0, 11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB9E8F2);
            ctx.begin_path();
            ctx.move_to(0, 6);
            ctx.bcurve_to(7, 1, 6, -4, 0, -3);
            ctx.bcurve_to(-6, -4, -7, 1, 0, 6);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffF6FFFF);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(0, -2);
            ctx.line_to(0, 5);
            ctx.move_to(0, 1);
            ctx.line_to(-3, 3);
            ctx.move_to(0, 1);
            ctx.line_to(3, 3);
            ctx.move_to(-4, -5);
            ctx.line_to(-2, -3);
            ctx.move_to(4, -5);
            ctx.line_to(2, -3);
            ctx.stroke();
            break;
        }
        case PetalID::kMatBlizzardEye: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff72B6D1);
            ctx.set_line_width(4.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-9, 2);
            ctx.bcurve_to(-8, -10, 8, -10, 9, 0);
            ctx.stroke();
            ctx.set_stroke(0xff9BD9E8);
            ctx.set_line_width(3.2f);
            ctx.begin_path();
            ctx.move_to(8, -1);
            ctx.bcurve_to(10, 10, -8, 10, -8, 1);
            ctx.stroke();
            ctx.set_stroke(0xffEAF9FC);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-6, 0);
            ctx.bcurve_to(-4, -6, 4, -6, 6, -1);
            ctx.stroke();
            ctx.set_fill(0xff426C9B);
            ctx.begin_path();
            ctx.arc(0, 0, 2.4f);
            ctx.fill();
            ctx.set_fill(0xffEAF9FC);
            ctx.begin_path();
            ctx.arc(-8, -6, 1.2f);
            ctx.arc(7, 6, 1.0f);
            ctx.arc(5, -7, 0.9f);
            ctx.fill();
            break;
        }
        case PetalID::kMatVine: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff285B3E);
            ctx.set_line_width(5.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 6, -2, 8, -10);
            ctx.stroke();
            ctx.set_stroke(0xff63A764);
            ctx.set_line_width(3.2f);
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 6, -2, 8, -10);
            ctx.stroke();
            ctx.set_fill(0xff86C777);
            ctx.set_stroke(0xff285B3E);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-2, 0);
            ctx.bcurve_to(-6, -4, -2, -8, 2, -4);
            ctx.bcurve_to(0, -1, 0, 1, -2, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(3, -4);
            ctx.bcurve_to(0, -8, 6, -11, 8, -7);
            ctx.bcurve_to(7, -4, 5, -3, 3, -4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kMatResin: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffD79540);
            ctx.set_stroke(0xff7C4D2C);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-2, -11);
            ctx.line_to(3, -8);
            ctx.bcurve_to(10, -2, 9, 8, 0, 11);
            ctx.bcurve_to(-10, 8, -9, -2, -2, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFE5A1);
            ctx.begin_path();
            ctx.ellipse(-3, -2, 2.4f, 5.0f);
            ctx.fill();
            ctx.set_fill(0xffB96B31);
            ctx.begin_path();
            ctx.ellipse(3, 3, 2.0f, 4.0f);
            ctx.fill();
            ctx.set_fill(0x88FFE5A1);
            ctx.begin_path();
            ctx.arc(1, 1, 1.8f);
            ctx.fill();
            break;
        }
        case PetalID::kMatVenomSac: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff7750A0);
            ctx.set_stroke(0xff3E345D);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-2, -10);
            ctx.line_to(2, -10);
            ctx.line_to(3, -6);
            ctx.bcurve_to(11, -2, 10, 8, 0, 11);
            ctx.bcurve_to(-10, 8, -11, -2, -3, -6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB1CF58);
            ctx.begin_path();
            ctx.ellipse(0, 2, 5.5f, 6.5f);
            ctx.fill();
            ctx.set_fill(0xffE3F5A0);
            ctx.begin_path();
            ctx.ellipse(-2.5f, -1, 1.8f, 3.5f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSpore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA6C878);
            ctx.set_stroke(0xff426B4D);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(-2, -1, 5.8f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(3, 2, 4.5f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(-5, 4, 3.2f);
            ctx.fill();
            ctx.set_fill(0xffD9EAA5);
            ctx.begin_path();
            ctx.arc(7, -4, 2.2f);
            ctx.arc(8, 5, 1.6f);
            ctx.arc(-8, -5, 1.8f);
            ctx.arc(-7, 7, 1.4f);
            ctx.arc(1, -7, 1.5f);
            ctx.fill();
            break;
        }
        case PetalID::kMatHeartwoodChip: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff865B3F);
            ctx.set_stroke(0xff3C4935);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-9, -6);
            ctx.line_to(2, -10);
            ctx.line_to(10, -3);
            ctx.line_to(8, 8);
            ctx.line_to(-3, 10);
            ctx.line_to(-11, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB78A58);
            ctx.begin_path();
            ctx.move_to(-4, -4);
            ctx.line_to(5, -5);
            ctx.line_to(4, 5);
            ctx.line_to(-5, 4);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffD2AF72);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 5.5f, 0.3f, 4.8f, 0);
            ctx.stroke();
            ctx.begin_path();
            ctx.partial_arc(0, 0, 3.2f, 1.0f, 5.2f, 0);
            ctx.stroke();
            ctx.set_stroke(0xff76AD62);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-2, 2);
            ctx.line_to(3, -1);
            ctx.stroke();
            break;
        }
        case PetalID::kMatCorruptSpore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff633F80);
            ctx.set_stroke(0xff322943);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 10.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff8A62A0);
            ctx.begin_path();
            ctx.arc(0, 0, 7.5f);
            ctx.fill();
            ctx.set_fill(0xffB4D66D);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(4, -1);
            ctx.line_to(3, 4);
            ctx.line_to(-3, 4);
            ctx.line_to(-4, -1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff322943);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-1, -1);
            ctx.line_to(2, 2);
            ctx.stroke();
            ctx.set_fill(0xff8A62A0);
            ctx.begin_path();
            ctx.arc(-9, -5, 1.5f);
            ctx.arc(8, 6, 1.3f);
            ctx.arc(7, -7, 1.1f);
            ctx.fill();
            break;
        }
        case PetalID::kMatAntShell: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff9E6841);
            ctx.set_stroke(0xff51382C);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-10, 2);
            ctx.bcurve_to(-10, -10, 10, -10, 10, 2);
            ctx.line_to(7, 8);
            ctx.line_to(0, 5);
            ctx.line_to(-7, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff764B37);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(0, -6);
            ctx.line_to(0, 5);
            ctx.move_to(-4, 2);
            ctx.line_to(-6, 7);
            ctx.move_to(4, 2);
            ctx.line_to(6, 7);
            ctx.stroke();
            ctx.set_fill(0xffC68B53);
            ctx.begin_path();
            ctx.ellipse(0, -5, 5.5f, 2.0f);
            ctx.fill();
            break;
        }
        case PetalID::kMatAntenna: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff51392E);
            ctx.set_line_width(4.0f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-2, 8);
            ctx.bcurve_to(-8, 2, -6, -6, -4, -10);
            ctx.move_to(2, 8);
            ctx.bcurve_to(8, 2, 6, -6, 4, -10);
            ctx.stroke();
            ctx.set_stroke(0xffA6754C);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-2, 8);
            ctx.bcurve_to(-8, 2, -6, -6, -4, -10);
            ctx.move_to(2, 8);
            ctx.bcurve_to(8, 2, 6, -6, 4, -10);
            ctx.stroke();
            ctx.set_fill(0xffC99B67);
            ctx.begin_path();
            ctx.arc(-4, -10, 2.2f);
            ctx.arc(4, -10, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatFormicAcid: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB59A76);
            ctx.set_stroke(0xff4D6746);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-4, -11, 8, 4, 1.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB8D76E);
            ctx.begin_path();
            ctx.move_to(-7, -6);
            ctx.line_to(7, -6);
            ctx.line_to(8, 9);
            ctx.line_to(-8, 9);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff6E9B51);
            ctx.begin_path();
            ctx.rect(-6, 2, 12, 6);
            ctx.fill();
            ctx.set_fill(0x88ffffff);
            ctx.begin_path();
            ctx.ellipse(-3, -2, 1.4f, 3.5f);
            ctx.arc(2, 0, 1.2f);
            ctx.arc(3, 4, 1.0f);
            ctx.fill();
            break;
        }
        case PetalID::kMatAntRoe: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff77543A);
            ctx.begin_path();
            ctx.ellipse(0, 8, 8.f, 2.2f);
            ctx.fill();
            auto egg = [&](float x, float y, float rx, float ry, float ang) {
                ctx.set_fill(0xffF1D5A1);
                ctx.set_stroke(0xff99734F);
                ctx.set_line_width(1.6f);
                ctx.begin_path();
                ctx.ellipse(x, y, rx, ry, ang);
                ctx.fill();
                ctx.stroke();
                ctx.set_fill(0xffFFF3CE);
                ctx.begin_path();
                ctx.ellipse(x - rx * 0.2f, y - ry * 0.25f, rx * 0.35f, ry * 0.3f, ang);
                ctx.fill();
            };
            egg(-3, -3, 4.5f, 6.0f, -0.35f);
            egg(4, -2, 4.0f, 5.5f, 0.4f);
            egg(-1, 4, 3.8f, 5.0f, 0.1f);
            egg(5, 5, 3.2f, 4.5f, -0.5f);
            break;
        }
        case PetalID::kMatQueenPheromone: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8E5D65);
            ctx.set_stroke(0xff523947);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.bcurve_to(8, -4, 8, 7, 0, 11);
            ctx.bcurve_to(-8, 7, -8, -4, 0, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD7A16C);
            ctx.begin_path();
            ctx.ellipse(0, 0, 4.5f, 7.0f);
            ctx.fill();
            ctx.set_fill(0xffF3D28A);
            ctx.begin_path();
            ctx.ellipse(-1.5f, -2, 1.6f, 3.0f);
            ctx.fill();
            ctx.set_stroke(0xff523947);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-7, -2);
            ctx.line_to(-11, -3);
            ctx.move_to(7, -2);
            ctx.line_to(11, -3);
            ctx.move_to(-5, 5);
            ctx.line_to(-9, 8);
            ctx.move_to(5, 5);
            ctx.line_to(9, 8);
            ctx.stroke();
            break;
        }
        case PetalID::kMatHiveHeart: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff764C38);
            ctx.set_stroke(0xff3E2D2B);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f;
                float py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py);
                else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffBA7D4A);
            ctx.begin_path();
            ctx.arc(0, 0, 7.f);
            ctx.fill();
            ctx.set_fill(0xffE4B16C);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                float px = std::cos(a) * 4.2f;
                float py = std::sin(a) * 4.2f;
                if (i == 0) ctx.move_to(px, py);
                else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(0, 0, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kMatFireCrystal: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE56A3C);
            ctx.set_stroke(0xff713B3A);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -12);
            ctx.line_to(6, -3);
            ctx.line_to(5, 4);
            ctx.line_to(0, 12);
            ctx.line_to(-5, 4);
            ctx.line_to(-4, -3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFBC58);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(3, -2);
            ctx.line_to(0, 3);
            ctx.line_to(-1, -2);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffB84538);
            ctx.begin_path();
            ctx.move_to(1, -1);
            ctx.line_to(5, 2);
            ctx.line_to(0, 10);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatSulfur: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC6B454);
            ctx.set_stroke(0xff77653A);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-9, -3);
            ctx.line_to(-4, -9);
            ctx.line_to(5, -8);
            ctx.line_to(10, -1);
            ctx.line_to(7, 8);
            ctx.line_to(-3, 9);
            ctx.line_to(-10, 3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE5D57A);
            ctx.begin_path();
            ctx.move_to(-4, -6);
            ctx.line_to(2, -5);
            ctx.line_to(-1, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffAA9548);
            ctx.begin_path();
            ctx.move_to(3, 0);
            ctx.line_to(7, 2);
            ctx.line_to(4, 7);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatLavaRock: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff373543);
            ctx.set_stroke(0xff171A26);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-9, -2);
            ctx.line_to(-3, -9);
            ctx.line_to(5, -8);
            ctx.line_to(10, 0);
            ctx.line_to(6, 8);
            ctx.line_to(-4, 9);
            ctx.line_to(-10, 3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffA94439);
            ctx.set_line_width(2.8f);
            ctx.begin_path();
            ctx.move_to(-4, -4);
            ctx.line_to(1, 0);
            ctx.line_to(-1, 5);
            ctx.line_to(4, 7);
            ctx.stroke();
            ctx.set_stroke(0xffFF9D46);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-3, -3);
            ctx.line_to(1, 1);
            ctx.line_to(0, 5);
            ctx.stroke();
            break;
        }
        case PetalID::kMatObsidian: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff292735);
            ctx.set_stroke(0xff11121C);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-2, -11);
            ctx.line_to(6, -6);
            ctx.line_to(9, 2);
            ctx.line_to(3, 11);
            ctx.line_to(-7, 6);
            ctx.line_to(-9, -2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff555166);
            ctx.begin_path();
            ctx.move_to(-2, -11);
            ctx.line_to(6, -6);
            ctx.line_to(1, 0);
            ctx.line_to(-7, -1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffA09BB1);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(-4, -4);
            ctx.line_to(2, -1);
            ctx.move_to(1, 2);
            ctx.line_to(5, 5);
            ctx.move_to(-2, 3);
            ctx.line_to(1, 7);
            ctx.stroke();
            break;
        }
        // --- Batch 3 materials + base craft (41–60) ---
        case PetalID::kMatLeylineCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff332A35);
            ctx.set_stroke(0xff191923);
            ctx.set_line_width(2.6f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA84132);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f;
                float px = std::cos(a) * 7.5f, py = std::sin(a) * 7.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffFFAA39);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-7, 0); ctx.line_to(7, 0);
            ctx.move_to(0, -7); ctx.line_to(0, 7);
            ctx.stroke();
            ctx.set_fill(0xffFFE08A);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(3.5f, 0);
            ctx.line_to(0, 4);
            ctx.line_to(-3.5f, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatPhoenixEmber: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC54837);
            ctx.set_stroke(0xff6E2E36);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, 10);
            ctx.bcurve_to(-10, 4, -8, -4, -4, -10);
            ctx.bcurve_to(-2, -4, -4, 2, 0, 4);
            ctx.bcurve_to(2, -6, 0, -12, 3, -11);
            ctx.bcurve_to(8, -6, 10, 2, 4, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFF9C42);
            ctx.begin_path();
            ctx.move_to(0, 6);
            ctx.bcurve_to(-5, 2, -3, -4, 0, -7);
            ctx.bcurve_to(3, -3, 4, 2, 0, 6);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffFFE29A);
            ctx.begin_path();
            ctx.ellipse(0, 4, 2.2f, 3.5f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSteel: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff7D8C9D);
            ctx.set_stroke(0xff3D4B5E);
            ctx.set_line_width(2.6f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-8, -4);
            ctx.line_to(2, -10);
            ctx.line_to(10, -4);
            ctx.line_to(8, 8);
            ctx.line_to(-2, 10);
            ctx.line_to(-10, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD3DFE6);
            ctx.begin_path();
            ctx.move_to(-6, -3);
            ctx.line_to(2, -8);
            ctx.line_to(4, -2);
            ctx.line_to(-3, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff66788B);
            ctx.begin_path();
            ctx.move_to(4, -2);
            ctx.line_to(9, -3);
            ctx.line_to(7, 7);
            ctx.line_to(2, 6);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatGear: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8796A7);
            ctx.set_stroke(0xff39485B);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a0 = i * (float)M_PI / 4.f;
                float a1 = a0 + 0.22f;
                float a2 = a0 + (float)M_PI / 4.f - 0.22f;
                float a3 = a0 + (float)M_PI / 4.f;
                float o = 11.f, inn = 8.2f;
                if (i == 0) ctx.move_to(std::cos(a0) * inn, std::sin(a0) * inn);
                ctx.line_to(std::cos(a1) * o, std::sin(a1) * o);
                ctx.line_to(std::cos(a2) * o, std::sin(a2) * o);
                ctx.line_to(std::cos(a3) * inn, std::sin(a3) * inn);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff56697D);
            ctx.begin_path();
            ctx.arc(0, 0, 4.5f);
            ctx.fill();
            ctx.set_fill(0xffB7C6D1);
            ctx.begin_path();
            ctx.arc(0, 0, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatHolyShard: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE8C66D);
            ctx.set_stroke(0xff97703B);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-2, -11);
            ctx.line_to(6, -7);
            ctx.line_to(9, 2);
            ctx.line_to(2, 11);
            ctx.line_to(-8, 5);
            ctx.line_to(-9, -3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFF1B5);
            ctx.begin_path();
            ctx.move_to(-1, -8);
            ctx.line_to(4, -5);
            ctx.line_to(1, 1);
            ctx.line_to(-4, -1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffFFF1B5);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(0, -6);
            ctx.line_to(0, 6);
            ctx.move_to(-4, 0);
            ctx.line_to(4, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kMatCapitalBadge: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB8464A);
            ctx.set_stroke(0xff663A3E);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, -8);
            ctx.line_to(8, -8);
            ctx.line_to(9, 2);
            ctx.line_to(0, 11);
            ctx.line_to(-9, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD56A5C);
            ctx.begin_path();
            ctx.move_to(-5, -5);
            ctx.line_to(5, -5);
            ctx.line_to(6, 1);
            ctx.line_to(0, 7);
            ctx.line_to(-6, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF0D18A);
            ctx.begin_path();
            ctx.move_to(-3, -2);
            ctx.line_to(-4, 1);
            ctx.line_to(-1, 1);
            ctx.line_to(0, 3);
            ctx.line_to(1, 1);
            ctx.line_to(4, 1);
            ctx.line_to(3, -2);
            ctx.line_to(0, -1);
            ctx.close_path();
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0, -4, 1.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMatSovereignCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff9A393F);
            ctx.set_stroke(0xff512B36);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD9A65A);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f;
                float px = std::cos(a) * 7.f, py = std::sin(a) * 7.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF6E3A5);
            ctx.begin_path();
            ctx.move_to(0, -3.5f);
            ctx.line_to(3, 0);
            ctx.line_to(0, 3.5f);
            ctx.line_to(-3, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMatRoyalSeal: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB84B53);
            ctx.set_stroke(0xff71343E);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            for (int i = 0; i < 10; ++i) {
                float a = i * (float)M_PI / 5.f;
                float rad = (i % 2 == 0) ? 11.f : 9.5f;
                float px = std::cos(a) * rad, py = std::sin(a) * rad;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD36C67);
            ctx.begin_path();
            ctx.arc(0, 0, 6.5f);
            ctx.fill();
            ctx.set_fill(0xffF3D08B);
            ctx.begin_path();
            ctx.move_to(-2.5f, -1);
            ctx.line_to(-3, 1.5f);
            ctx.line_to(-0.5f, 1.5f);
            ctx.line_to(0, 3);
            ctx.line_to(0.5f, 1.5f);
            ctx.line_to(3, 1.5f);
            ctx.line_to(2.5f, -1);
            ctx.line_to(0, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kBeeSting: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE6AE47);
            ctx.set_stroke(0xff6C4B30);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-11, 4);
            ctx.line_to(4, -8);
            ctx.line_to(10, -4);
            ctx.line_to(6, 0);
            ctx.line_to(9, 3);
            ctx.line_to(4, 4);
            ctx.line_to(7, 7);
            ctx.line_to(2, 6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFE29A);
            ctx.begin_path();
            ctx.move_to(-6, 2);
            ctx.line_to(2, -4);
            ctx.line_to(3, -1);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kHealingBloom: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffF5B6B9);
            ctx.set_stroke(0xffB97591);
            ctx.set_line_width(1.8f);
            for (int i = 0; i < 5; ++i) {
                float a = i * (float)M_PI * 2.f / 5.f - (float)M_PI / 2.f;
                float cx = std::cos(a) * 5.5f, cy = std::sin(a) * 5.5f;
                ctx.begin_path();
                ctx.ellipse(cx, cy, 4.2f, 5.5f, a);
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_fill(0xffE7D77C);
            ctx.begin_path();
            ctx.arc(0, 0, 4.2f);
            ctx.fill();
            ctx.set_fill(0xffFFF3BC);
            ctx.begin_path();
            ctx.arc(-1, -1.2f, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kSandSpike: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC99B5B);
            ctx.set_stroke(0xff805B39);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -12);
            ctx.line_to(7, 4);
            ctx.line_to(5, 11);
            ctx.line_to(-5, 11);
            ctx.line_to(-7, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF3D391);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(4, 3);
            ctx.line_to(0, 6);
            ctx.line_to(-1, 2);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffB77F45);
            ctx.begin_path();
            ctx.move_to(1, 2);
            ctx.line_to(6, 5);
            ctx.line_to(4, 10);
            ctx.line_to(0, 9);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kCraftCactus: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff66A65A);
            ctx.set_stroke(0xff386C41);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-3.5f, -9, 7, 18, 3.5f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.round_rect(-10, -2, 7, 5, 2.5f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.round_rect(3, 1, 7, 5, 2.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffA9D88B);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-1, -7);
            ctx.line_to(-1, 7);
            ctx.move_to(-8, -1);
            ctx.line_to(-5, 0);
            ctx.move_to(5, 2);
            ctx.line_to(8, 3);
            ctx.move_to(1, -4);
            ctx.line_to(3, -5);
            ctx.move_to(-2, 3);
            ctx.line_to(-4, 4);
            ctx.stroke();
            break;
        }
        case PetalID::kQuicksandOrb: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC49A5C);
            ctx.set_stroke(0xff7C5C3A);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffE8C584);
            ctx.set_line_width(2.8f);
            ctx.begin_path();
            ctx.move_to(-6, -4);
            ctx.bcurve_to(-2, -8, 8, -4, 5, 2);
            ctx.stroke();
            ctx.set_stroke(0xff986C43);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(4, -2);
            ctx.bcurve_to(6, 4, -4, 6, -3, 1);
            ctx.stroke();
            ctx.set_fill(0xffE8C584);
            ctx.begin_path();
            ctx.arc(-7, 5, 1.2f);
            ctx.arc(7, -6, 1.0f);
            ctx.arc(2, 7, 0.9f);
            ctx.fill();
            break;
        }
        case PetalID::kWaterBubble: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff358CA9);
            ctx.set_fill(0x668CD9E7);
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x55B7EBEF);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f);
            ctx.fill();
            ctx.set_stroke(0xffF5FFFF);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.partial_arc(-1, -2, 7.f, 3.6f, 5.2f, 0);
            ctx.stroke();
            ctx.set_fill(0xffF5FFFF);
            ctx.begin_path();
            ctx.arc(-3, -4, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kStarfish: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffED9A7D);
            ctx.set_stroke(0xffA86566);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            for (int i = 0; i < 5; ++i) {
                float a = i * (float)M_PI * 2.f / 5.f - (float)M_PI / 2.f;
                float a2 = a + (float)M_PI / 5.f;
                float ox = std::cos(a) * 11.f, oy = std::sin(a) * 11.f;
                float ix = std::cos(a2) * 4.f, iy = std::sin(a2) * 4.f;
                if (i == 0) ctx.move_to(ox, oy);
                else ctx.line_to(ox, oy);
                ctx.line_to(ix, iy);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE8B68D);
            ctx.begin_path();
            ctx.arc(0, 0, 2.8f);
            ctx.fill();
            ctx.set_fill(0xffF8C6A0);
            ctx.begin_path();
            ctx.ellipse(0, -6, 1.6f, 2.8f);
            ctx.ellipse(5, 2, 1.4f, 2.4f);
            ctx.ellipse(-5, 2, 1.4f, 2.4f);
            ctx.fill();
            break;
        }
        case PetalID::kTidePearl: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff4E9DB8);
            ctx.set_stroke(0xff245773);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC2E5EB);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f);
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.ellipse(-3, -3.5f, 2.8f, 2.0f);
            ctx.fill();
            ctx.set_stroke(0xff3F9EBB);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.partial_arc(0, 2, 6.5f, 0.4f, 2.8f, 0);
            ctx.stroke();
            ctx.begin_path();
            ctx.partial_arc(0, 3.5f, 4.5f, 0.5f, 2.6f, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kIceSpike: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff77BCD7);
            ctx.set_stroke(0xff3F789C);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -12);
            ctx.line_to(5, 2);
            ctx.line_to(6, 10);
            ctx.line_to(0, 8);
            ctx.line_to(-6, 10);
            ctx.line_to(-5, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD9F4FB);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(3, 1);
            ctx.line_to(0, 4);
            ctx.line_to(-1, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff5B9FBE);
            ctx.begin_path();
            ctx.move_to(1, 2);
            ctx.line_to(5, 4);
            ctx.line_to(4, 9);
            ctx.line_to(0, 7);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kSnowball: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE8F3F8);
            ctx.set_stroke(0xff9CC6D8);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.ellipse(-3, -4, 3.5f, 2.4f);
            ctx.fill();
            ctx.set_fill(0xffC8E1EB);
            ctx.begin_path();
            ctx.ellipse(1, 5, 5.5f, 2.4f);
            ctx.fill();
            ctx.set_stroke(0xffC8E1EB);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.move_to(-4, 1);
            ctx.line_to(2, 0);
            ctx.move_to(3, -2);
            ctx.line_to(6, 1);
            ctx.stroke();
            break;
        }
        case PetalID::kIceShield: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff77B7D0);
            ctx.set_stroke(0xff376C90);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, -8);
            ctx.line_to(8, -8);
            ctx.line_to(9, 1);
            ctx.line_to(0, 11);
            ctx.line_to(-9, 1);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC6EBF5);
            ctx.begin_path();
            ctx.move_to(-5, -5);
            ctx.line_to(5, -5);
            ctx.line_to(6, 1);
            ctx.line_to(0, 7);
            ctx.line_to(-6, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(0, 5);
            ctx.move_to(-3, 0);
            ctx.line_to(3, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kThornVine: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff285A3B);
            ctx.set_line_width(5.8f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 6, -2, 9, -10);
            ctx.stroke();
            ctx.set_stroke(0xff6AAB63);
            ctx.set_line_width(3.0f);
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 6, -2, 9, -10);
            ctx.stroke();
            ctx.set_fill(0xffB9CF7A);
            ctx.set_stroke(0xff386D44);
            ctx.set_line_width(1.4f);
            auto thorn = [&](float x, float y, float dx, float dy) {
                ctx.begin_path();
                ctx.move_to(x, y);
                ctx.line_to(x + dx, y + dy);
                ctx.line_to(x + dy * 0.4f, y - dx * 0.4f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            };
            thorn(-4, 4, -5, -1);
            thorn(0, -1, 5, 2);
            thorn(4, -6, -3, -5);
            break;
        }

        // --- Batch 4 base craft + cross epic (61–80) ---
        case PetalID::kRotwood: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff70563E);
            ctx.set_stroke(0xff37432F);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-9, -4);
            ctx.line_to(-3, -10);
            ctx.line_to(6, -8);
            ctx.line_to(10, -1);
            ctx.line_to(7, 8);
            ctx.line_to(-2, 10);
            ctx.line_to(-10, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA17A50);
            ctx.begin_path();
            ctx.move_to(-5, -5);
            ctx.line_to(2, -7);
            ctx.line_to(0, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff594935);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-3, -2);
            ctx.line_to(1, 1);
            ctx.line_to(-1, 5);
            ctx.stroke();
            ctx.set_fill(0xffB9CF68);
            ctx.begin_path();
            ctx.arc(3, 2, 1.6f);
            ctx.arc(-4, 5, 1.3f);
            ctx.fill();
            break;
        }
        case PetalID::kAntFang: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE5D5AC);
            ctx.set_stroke(0xff675746);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, 6);
            ctx.bcurve_to(-6, -2, 4, -10, 10, -8);
            ctx.bcurve_to(4, -4, -2, 2, -4, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFF1D0);
            ctx.begin_path();
            ctx.move_to(-4, 2);
            ctx.bcurve_to(0, -4, 5, -6, 7, -5);
            ctx.bcurve_to(2, -2, -1, 2, -3, 4);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff836247);
            ctx.begin_path();
            ctx.move_to(-8, 4);
            ctx.line_to(-4, 8);
            ctx.line_to(-9, 9);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kAcidFlask: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff92744F);
            ctx.set_stroke(0xff45655B);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-3.5f, -11, 7, 3.5f, 1.2f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB9D7BE);
            ctx.begin_path();
            ctx.move_to(-4, -7);
            ctx.line_to(4, -7);
            ctx.line_to(8, -2);
            ctx.line_to(7, 10);
            ctx.line_to(-7, 10);
            ctx.line_to(-8, -2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA8CE55);
            ctx.begin_path();
            ctx.move_to(-6, 1);
            ctx.line_to(6, -1);
            ctx.line_to(6, 9);
            ctx.line_to(-6, 9);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffDDF28B);
            ctx.begin_path();
            ctx.arc(-2, 4, 1.2f);
            ctx.arc(2, 6, 1.0f);
            ctx.fill();
            break;
        }
        case PetalID::kSoldierEgg: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB98456);
            ctx.set_stroke(0xff5E4438);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.bcurve_to(7, -8, 9, 2, 6, 10);
            ctx.bcurve_to(2, 12, -2, 12, -6, 10);
            ctx.bcurve_to(-9, 2, -7, -8, 0, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE4BF86);
            ctx.begin_path();
            ctx.ellipse(0, 0, 5.5f, 8.f);
            ctx.fill();
            ctx.set_fill(0xffA56646);
            ctx.begin_path();
            ctx.ellipse(0, 1, 3.2f, 5.f);
            ctx.fill();
            ctx.set_stroke(0xff5E4438);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-2, -1);
            ctx.line_to(2, -1);
            ctx.move_to(-2, 2);
            ctx.line_to(2, 2);
            ctx.move_to(-1.5f, 5);
            ctx.line_to(1.5f, 5);
            ctx.stroke();
            ctx.set_fill(0xffFFF0C5);
            ctx.begin_path();
            ctx.ellipse(-2, -5, 1.8f, 2.5f);
            ctx.fill();
            break;
        }
        case PetalID::kFireball: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC84937);
            ctx.set_stroke(0xff71313A);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-2, 8);
            ctx.bcurve_to(-10, 4, -8, -4, -3, -10);
            ctx.bcurve_to(-1, -4, 0, -8, 2, -11);
            ctx.bcurve_to(4, -6, 3, -2, 5, -8);
            ctx.bcurve_to(10, -2, 9, 6, 2, 10);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFF963E);
            ctx.begin_path();
            ctx.arc(0, 3, 5.5f);
            ctx.fill();
            ctx.set_fill(0xffFFD26C);
            ctx.begin_path();
            ctx.arc(0, 4, 3.2f);
            ctx.fill();
            ctx.set_fill(0xffFFF1B0);
            ctx.begin_path();
            ctx.ellipse(-1.5f, 2, 1.4f, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kMagmaCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff302B36);
            ctx.set_stroke(0xff151721);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB54435);
            ctx.begin_path();
            ctx.arc(0, 0, 7.5f);
            ctx.fill();
            ctx.set_fill(0xffF07935);
            ctx.begin_path();
            ctx.arc(0, 0, 5.f);
            ctx.fill();
            ctx.set_fill(0xffFFCA65);
            ctx.begin_path();
            ctx.arc(0, 0, 2.6f);
            ctx.fill();
            break;
        }
        case PetalID::kObsidianBlade: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff302C3A);
            ctx.set_stroke(0xff151620);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(8, -10);
            ctx.line_to(10, -4);
            ctx.line_to(2, 6);
            ctx.line_to(-2, 4);
            ctx.line_to(2, -6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff696276);
            ctx.begin_path();
            ctx.move_to(7, -8);
            ctx.line_to(8, -4);
            ctx.line_to(3, 2);
            ctx.line_to(2, -4);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffA27961);
            ctx.begin_path();
            ctx.move_to(-1, 3);
            ctx.line_to(3, 7);
            ctx.line_to(1, 9);
            ctx.line_to(-3, 5);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kSteelSword: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffAAB9C8);
            ctx.set_stroke(0xff41536A);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(6, -11);
            ctx.line_to(9, -8);
            ctx.line_to(2, 4);
            ctx.line_to(-2, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE4EDF3);
            ctx.begin_path();
            ctx.move_to(6, -10);
            ctx.line_to(7.5f, -8);
            ctx.line_to(1, 2);
            ctx.line_to(0, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff41536A);
            ctx.set_line_width(3.5f);
            ctx.begin_path();
            ctx.move_to(-4, 1);
            ctx.line_to(4, 5);
            ctx.stroke();
            ctx.set_stroke(0xffD1AF6C);
            ctx.set_line_width(2.8f);
            ctx.begin_path();
            ctx.move_to(-1, 4);
            ctx.line_to(-4, 9);
            ctx.stroke();
            ctx.set_fill(0xffD1AF6C);
            ctx.begin_path();
            ctx.arc(-4.5f, 10, 1.8f);
            ctx.fill();
            break;
        }
        case PetalID::kHolyOrb: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE3BE69);
            ctx.set_stroke(0xff987240);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFFF0B7);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f);
            ctx.fill();
            ctx.set_stroke(0xffD8AA54);
            ctx.set_line_width(3.2f);
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.line_to(0, 5);
            ctx.move_to(-5, 0);
            ctx.line_to(5, 0);
            ctx.stroke();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(0, 4);
            ctx.move_to(-4, 0);
            ctx.line_to(4, 0);
            ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.ellipse(-3, -4, 2.2f, 1.5f);
            ctx.fill();
            break;
        }
        case PetalID::kMechGear: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff728A9E);
            ctx.set_stroke(0xff34475A);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a0 = i * (float)M_PI / 4.f;
                float a1 = a0 + 0.2f;
                float a2 = a0 + (float)M_PI / 4.f - 0.2f;
                float a3 = a0 + (float)M_PI / 4.f;
                float o = 11.5f, inn = 8.5f;
                if (i == 0) ctx.move_to(std::cos(a0) * inn, std::sin(a0) * inn);
                ctx.line_to(std::cos(a1) * o, std::sin(a1) * o);
                ctx.line_to(std::cos(a2) * o, std::sin(a2) * o);
                ctx.line_to(std::cos(a3) * inn, std::sin(a3) * inn);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff566F83);
            ctx.begin_path();
            ctx.arc(0, 0, 5.5f);
            ctx.fill();
            ctx.set_stroke(0xffB3C4D0);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(-5, 0); ctx.line_to(5, 0);
            ctx.move_to(0, -5); ctx.line_to(0, 5);
            ctx.stroke();
            ctx.set_fill(0xffE3B969);
            ctx.begin_path();
            ctx.arc(0, 0, 2.4f);
            ctx.fill();
            break;
        }
        case PetalID::kThornPalm: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff5C9654);
            ctx.set_stroke(0xff28543A);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -8);
            ctx.bcurve_to(8, -6, 10, 4, 4, 9);
            ctx.bcurve_to(0, 11, -4, 9, -8, 4);
            ctx.bcurve_to(-10, -4, -6, -8, 0, -8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff92C879);
            ctx.begin_path();
            ctx.ellipse(0, 0, 5.5f, 6.5f);
            ctx.fill();
            ctx.set_fill(0xffD7BD76);
            ctx.set_stroke(0xff28543A);
            ctx.set_line_width(1.4f);
            auto spike = [&](float x, float y, float dx, float dy) {
                ctx.begin_path();
                ctx.move_to(x, y);
                ctx.line_to(x + dx, y + dy);
                ctx.line_to(x - dy * 0.35f, y + dx * 0.35f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            };
            spike(0, -7, 0, -5);
            spike(8, -1, 5, -1);
            spike(-8, 0, -5, 1);
            spike(2, 8, 2, 4);
            ctx.set_stroke(0xffC8E6A0);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(0, 5);
            ctx.move_to(-2, 0);
            ctx.line_to(2, 1);
            ctx.stroke();
            break;
        }
        case PetalID::kPollenSandstorm: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xffB78E52);
            ctx.set_line_width(4.2f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-9, 1);
            ctx.bcurve_to(-8, -10, 8, -9, 9, 1);
            ctx.stroke();
            ctx.set_stroke(0xffD8B86B);
            ctx.set_line_width(3.0f);
            ctx.begin_path();
            ctx.move_to(8, 0);
            ctx.bcurve_to(9, 9, -7, 10, -8, 1);
            ctx.stroke();
            ctx.set_stroke(0xffF1D58B);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-5, -1);
            ctx.bcurve_to(-2, -6, 5, -5, 6, 0);
            ctx.stroke();
            ctx.set_fill(0xffE8B85D);
            ctx.begin_path();
            ctx.arc(0, 0, 2.8f);
            ctx.fill();
            ctx.set_fill(0xffF4DF91);
            ctx.begin_path();
            ctx.arc(-8, -5, 1.2f);
            ctx.arc(7, 6, 1.1f);
            ctx.arc(6, -7, 1.0f);
            ctx.arc(-6, 7, 0.9f);
            ctx.arc(0, -8, 0.9f);
            ctx.arc(9, 0, 0.8f);
            ctx.fill();
            break;
        }
        case PetalID::kSandLeafDart: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB6A15C);
            ctx.set_stroke(0xff6B7043);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-10, 6);
            ctx.bcurve_to(-4, 0, 4, -8, 11, -9);
            ctx.bcurve_to(6, -4, 0, 2, -6, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffDCC47A);
            ctx.begin_path();
            ctx.move_to(-6, 4);
            ctx.bcurve_to(0, -2, 5, -6, 8, -6);
            ctx.bcurve_to(3, -2, -1, 2, -4, 5);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffF5DEA0);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(-7, 5);
            ctx.line_to(8, -7);
            ctx.move_to(-2, 2);
            ctx.line_to(0, -2);
            ctx.move_to(2, 0);
            ctx.line_to(4, -3);
            ctx.stroke();
            break;
        }
        case PetalID::kDesertRoot: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff795D3D);
            ctx.set_line_width(6.0f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-7, 9);
            ctx.bcurve_to(-9, 0, 4, -2, 8, -9);
            ctx.stroke();
            ctx.set_stroke(0xffB18A58);
            ctx.set_line_width(3.4f);
            ctx.begin_path();
            ctx.move_to(-7, 9);
            ctx.bcurve_to(-9, 0, 4, -2, 8, -9);
            ctx.stroke();
            ctx.set_stroke(0xff795D3D);
            ctx.set_line_width(3.5f);
            ctx.begin_path();
            ctx.move_to(-2, 2);
            ctx.line_to(-8, -4);
            ctx.move_to(2, -2);
            ctx.line_to(7, 4);
            ctx.stroke();
            ctx.set_stroke(0xffC4A26B);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-2, 2);
            ctx.line_to(-8, -4);
            ctx.move_to(2, -2);
            ctx.line_to(7, 4);
            ctx.stroke();
            ctx.set_fill(0xffE2C68A);
            ctx.begin_path();
            ctx.arc(-8, -4, 1.4f);
            ctx.arc(8, -9, 1.4f);
            ctx.fill();
            break;
        }
        case PetalID::kFrostBubble: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0x667EC9DE);
            ctx.set_stroke(0xff376F96);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 10.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x55C8EFF7);
            ctx.begin_path();
            ctx.arc(0, 0, 7.f);
            ctx.fill();
            ctx.set_stroke(0xff78B8D4);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-4, -4); ctx.line_to(4, 4);
            ctx.move_to(4, -4); ctx.line_to(-4, 4);
            ctx.move_to(0, -5); ctx.line_to(0, 5);
            ctx.move_to(-5, 0); ctx.line_to(5, 0);
            ctx.stroke();
            ctx.set_fill(0xffD5F5FB);
            ctx.set_stroke(0xff376F96);
            ctx.set_line_width(1.2f);
            auto shard = [&](float x, float y) {
                ctx.begin_path();
                ctx.move_to(x, y - 2.2f);
                ctx.line_to(x + 1.4f, y);
                ctx.line_to(x, y + 2.2f);
                ctx.line_to(x - 1.4f, y);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            };
            shard(-9, -5); shard(9, -4); shard(8, 6); shard(-8, 7);
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.ellipse(-3, -3.5f, 2.0f, 1.4f);
            ctx.fill();
            break;
        }
        case PetalID::kFrostStarfish: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff93CEE1);
            ctx.set_stroke(0xff427C9E);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            for (int i = 0; i < 5; ++i) {
                float a = i * (float)M_PI * 2.f / 5.f - (float)M_PI / 2.f;
                float a2 = a + (float)M_PI / 5.f;
                float ox = std::cos(a) * 11.f, oy = std::sin(a) * 11.f;
                float ix = std::cos(a2) * 3.5f, iy = std::sin(a2) * 3.5f;
                if (i == 0) ctx.move_to(ox, oy);
                else ctx.line_to(ox, oy);
                ctx.line_to(ix, iy);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE4F7FB);
            ctx.begin_path();
            for (int i = 0; i < 5; ++i) {
                float a = i * (float)M_PI * 2.f / 5.f - (float)M_PI / 2.f;
                float px = std::cos(a) * 5.f, py = std::sin(a) * 5.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff6CAFCB);
            ctx.begin_path();
            ctx.arc(0, 0, 2.4f);
            ctx.fill();
            break;
        }
        case PetalID::kIceTideRing: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff39799D);
            ctx.set_line_width(4.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 9.5f);
            ctx.stroke();
            ctx.set_stroke(0xff8ED4E5);
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.arc(0, 0, 9.5f);
            ctx.stroke();
            ctx.set_fill(0xffD5F5FB);
            ctx.set_stroke(0xff4F93B5);
            ctx.set_line_width(1.4f);
            for (int i = 0; i < 4; ++i) {
                float a = i * (float)M_PI / 2.f;
                float x = std::cos(a) * 11.f, y = std::sin(a) * 11.f;
                ctx.begin_path();
                ctx.move_to(x + std::cos(a) * 2.5f, y + std::sin(a) * 2.5f);
                ctx.line_to(x + std::cos(a + 1.2f) * 1.6f, y + std::sin(a + 1.2f) * 1.6f);
                ctx.line_to(x - std::cos(a) * 0.5f, y - std::sin(a) * 0.5f);
                ctx.line_to(x + std::cos(a - 1.2f) * 1.6f, y + std::sin(a - 1.2f) * 1.6f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            }
            break;
        }
        case PetalID::kDeepFrostPearl: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff376D9D);
            ctx.set_stroke(0xff203F68);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff74B7D5);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f);
            ctx.fill();
            ctx.set_fill(0xffC4EDF6);
            ctx.set_stroke(0xff4E95B7);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(0, -7);
            ctx.line_to(3, -2);
            ctx.line_to(3, 3);
            ctx.line_to(0, 7);
            ctx.line_to(-3, 3);
            ctx.line_to(-3, -2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.line_to(0, 5);
            ctx.move_to(-2, 0);
            ctx.line_to(2, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kPoisonAntNest: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff775B47);
            ctx.set_stroke(0xff403C34);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA17A53);
            ctx.begin_path();
            ctx.arc(0, 0, 8.f);
            ctx.fill();
            ctx.set_fill(0xff77529A);
            ctx.begin_path();
            ctx.ellipse(0, 1, 5.f, 4.5f);
            ctx.fill();
            ctx.set_fill(0xffB4D56C);
            ctx.begin_path();
            ctx.move_to(0, -2);
            ctx.line_to(2.5f, 0);
            ctx.line_to(1.5f, 3);
            ctx.line_to(-1.5f, 3);
            ctx.line_to(-2.5f, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffB4D56C);
            ctx.begin_path();
            ctx.arc(-8, -5, 1.3f);
            ctx.arc(8, -4, 1.2f);
            ctx.arc(7, 7, 1.1f);
            ctx.arc(-7, 6, 1.0f);
            ctx.fill();
            break;
        }
        case PetalID::kCorrodeVine: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff354D39);
            ctx.set_line_width(6.2f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 5, -2, 9, -10);
            ctx.stroke();
            ctx.set_stroke(0xff6A9A55);
            ctx.set_line_width(3.4f);
            ctx.begin_path();
            ctx.move_to(-8, 10);
            ctx.bcurve_to(-10, 0, 5, -2, 9, -10);
            ctx.stroke();
            ctx.set_fill(0xffB7CF6A);
            ctx.set_stroke(0xff354D39);
            ctx.set_line_width(1.3f);
            auto leaf = [&](float x, float y, float dx, float dy) {
                ctx.begin_path();
                ctx.move_to(x, y);
                ctx.line_to(x + dx, y + dy);
                ctx.line_to(x + dy * 0.5f, y - dx * 0.35f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            };
            leaf(-3, 4, -5, -2);
            leaf(1, -1, 5, 1);
            leaf(5, -6, -2, -5);
            ctx.set_fill(0xffC3D95C);
            ctx.set_stroke(0xff728747);
            ctx.set_line_width(1.3f);
            ctx.begin_path();
            ctx.move_to(-1, 0);
            ctx.bcurve_to(-4, 2, -3, 6, 0, 5);
            ctx.bcurve_to(1, 3, 1, 1, -1, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(4, -3);
            ctx.bcurve_to(7, -2, 7, 2, 4, 2);
            ctx.bcurve_to(3, 0, 3, -2, 4, -3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }

        // --- Batch 5–6 cross legend / legendary (81–110) ---
        case PetalID::kColonyCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff6B527E);
            ctx.set_stroke(0xff383A49);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 10.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff9A75A8);
            ctx.begin_path();
            ctx.arc(0, 0, 7.f);
            ctx.fill();
            ctx.set_fill(0xffB6D36B);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                float px = std::cos(a) * 3.8f, py = std::sin(a) * 3.8f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffE4EF9C);
            ctx.begin_path();
            ctx.arc(0, 0, 1.5f);
            ctx.fill();
            ctx.set_fill(0xffB6D36B);
            ctx.begin_path();
            ctx.arc(-9, -4, 1.5f);
            ctx.arc(9, -3, 1.4f);
            ctx.arc(7, 7, 1.3f);
            ctx.arc(-7, 7, 1.2f);
            ctx.arc(0, -10, 1.1f);
            ctx.fill();
            break;
        }
        case PetalID::kSymbioteShell: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff4D7357);
            ctx.set_stroke(0xff29443A);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, -7);
            ctx.line_to(8, -7);
            ctx.line_to(9, 2);
            ctx.line_to(0, 11);
            ctx.line_to(-9, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff88B47A);
            ctx.begin_path();
            ctx.move_to(-5, -4);
            ctx.line_to(5, -4);
            ctx.line_to(6, 1);
            ctx.line_to(0, 7);
            ctx.line_to(-6, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff38654A);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(0, 6);
            ctx.move_to(-3, -1);
            ctx.line_to(-4, 3);
            ctx.move_to(3, -1);
            ctx.line_to(4, 3);
            ctx.stroke();
            ctx.set_fill(0xffC0DAA2);
            ctx.begin_path();
            ctx.arc(0, 0, 1.8f);
            ctx.fill();
            break;
        }
        case PetalID::kForgedHolySword: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffAAB8C5);
            ctx.set_stroke(0xff41536A);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(5, -11);
            ctx.line_to(8, -8);
            ctx.line_to(1, 4);
            ctx.line_to(-2, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE6EAF0);
            ctx.begin_path();
            ctx.move_to(5, -10);
            ctx.line_to(6.5f, -8);
            ctx.line_to(0, 2);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF28A3D);
            ctx.begin_path();
            ctx.move_to(6, -6);
            ctx.bcurve_to(11, -4, 10, 2, 5, 3);
            ctx.bcurve_to(7, 0, 7, -3, 6, -6);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffD4A65E);
            ctx.set_line_width(3.2f);
            ctx.begin_path();
            ctx.move_to(-4, 1);
            ctx.line_to(3, 5);
            ctx.stroke();
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.move_to(-1, 4);
            ctx.line_to(-4, 9);
            ctx.stroke();
            break;
        }
        case PetalID::kFlameGear: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff4B4652);
            ctx.set_stroke(0xff24242E);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a0 = i * (float)M_PI / 4.f;
                float a1 = a0 + 0.2f, a2 = a0 + (float)M_PI / 4.f - 0.2f, a3 = a0 + (float)M_PI / 4.f;
                float o = 11.5f, inn = 8.5f;
                if (i == 0) ctx.move_to(std::cos(a0) * inn, std::sin(a0) * inn);
                ctx.line_to(std::cos(a1) * o, std::sin(a1) * o);
                ctx.line_to(std::cos(a2) * o, std::sin(a2) * o);
                ctx.line_to(std::cos(a3) * inn, std::sin(a3) * inn);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA64C3D);
            ctx.begin_path();
            ctx.arc(0, 0, 6.f);
            ctx.fill();
            ctx.set_fill(0xffF08A3F);
            ctx.begin_path();
            ctx.arc(0, 0, 4.f);
            ctx.fill();
            ctx.set_fill(0xffFFE08B);
            ctx.begin_path();
            ctx.arc(0, 0, 2.f);
            ctx.fill();
            break;
        }
        case PetalID::kObsidianKingShield: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff292631);
            ctx.set_stroke(0xff11131E);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, -8);
            ctx.line_to(8, -8);
            ctx.line_to(9, 2);
            ctx.line_to(0, 11);
            ctx.line_to(-9, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff595368);
            ctx.begin_path();
            ctx.move_to(-5, -5);
            ctx.line_to(5, -5);
            ctx.line_to(6, 1);
            ctx.line_to(0, 7);
            ctx.line_to(-6, 1);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffE5C986);
            ctx.begin_path();
            ctx.move_to(-2.5f, -1);
            ctx.line_to(-3, 1.5f);
            ctx.line_to(-0.5f, 1.5f);
            ctx.line_to(0, 3);
            ctx.line_to(0.5f, 1.5f);
            ctx.line_to(3, 1.5f);
            ctx.line_to(2.5f, -1);
            ctx.line_to(0, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kHolyFlameCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA34747);
            ctx.set_stroke(0xff62363D);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE5BC70);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f;
                float px = std::cos(a) * 7.f, py = std::sin(a) * 7.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF58D44);
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.bcurve_to(-4, 0, -2, 5, 0, 6);
            ctx.bcurve_to(2, 5, 4, 0, 0, -5);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffFFF1B3);
            ctx.begin_path();
            ctx.ellipse(0, 1, 1.6f, 2.8f);
            ctx.fill();
            break;
        }
        case PetalID::kAnemoneBloom: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff77C9C7);
            ctx.set_stroke(0xff327F91);
            ctx.set_line_width(1.6f);
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f;
                float cx = std::cos(a) * 6.5f, cy = std::sin(a) * 6.5f;
                ctx.begin_path();
                ctx.ellipse(cx, cy, 2.2f, 5.0f, a);
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_fill(0xffE9A5A1);
            ctx.begin_path();
            ctx.arc(0, 0, 4.5f);
            ctx.fill();
            ctx.set_fill(0xffF7D7A5);
            ctx.begin_path();
            ctx.arc(0, 0, 2.4f);
            ctx.fill();
            break;
        }
        case PetalID::kWetLeaf: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff66B78B);
            ctx.set_stroke(0xff2E746B);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-9, 5);
            ctx.bcurve_to(-4, -4, 4, -10, 10, -8);
            ctx.bcurve_to(6, -2, 0, 4, -5, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffD5F2C0);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(-6, 4);
            ctx.line_to(7, -6);
            ctx.move_to(-2, 2);
            ctx.line_to(1, -2);
            ctx.move_to(2, 0);
            ctx.line_to(4, -3);
            ctx.stroke();
            ctx.set_stroke(0xff65C6DD);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.partial_arc(6, 2, 5.f, -0.4f, 1.4f, 0);
            ctx.stroke();
            ctx.set_fill(0xff65C6DD);
            ctx.begin_path();
            ctx.arc(8, 5, 1.5f);
            ctx.fill();
            break;
        }
        case PetalID::kCoralHive: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC98673);
            ctx.set_stroke(0xff7C5B58);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                float px = std::cos(a) * 10.5f, py = std::sin(a) * 10.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA76B5D);
            auto hex = [&](float x, float y, float s) {
                ctx.begin_path();
                for (int i = 0; i < 6; ++i) {
                    float a = i * (float)M_PI / 3.f;
                    float px = x + std::cos(a) * s, py = y + std::sin(a) * s;
                    if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
                }
                ctx.close_path();
                ctx.fill();
            };
            hex(0, -2.5f, 2.2f);
            hex(-2.5f, 2.f, 2.0f);
            hex(2.5f, 2.f, 2.0f);
            ctx.set_fill(0xffE5A86C);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(0, -13);
            ctx.line_to(1.5f, -11);
            ctx.move_to(8, 4);
            ctx.line_to(12, 5);
            ctx.line_to(9, 7);
            ctx.move_to(-8, 4);
            ctx.line_to(-12, 5);
            ctx.line_to(-9, 7);
            ctx.fill();
            break;
        }
        case PetalID::kLifeLotus: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8FCB99);
            ctx.set_stroke(0xff2E746B);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.ellipse(-4, 6, 4.5f, 2.2f, -0.4f);
            ctx.ellipse(4, 6, 4.5f, 2.2f, 0.4f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF4B9CB);
            ctx.set_stroke(0xffB87E9D);
            ctx.set_line_width(1.6f);
            for (int i = 0; i < 5; ++i) {
                float a = i * (float)M_PI * 2.f / 5.f - (float)M_PI / 2.f;
                float cx = std::cos(a) * 4.5f, cy = std::sin(a) * 4.5f - 1.f;
                ctx.begin_path();
                ctx.ellipse(cx, cy, 3.5f, 6.f, a);
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_fill(0xffF0D17E);
            ctx.begin_path();
            ctx.arc(0, -1, 3.2f);
            ctx.fill();
            ctx.set_fill(0xffFFF2BC);
            ctx.begin_path();
            ctx.arc(-0.8f, -2, 1.3f);
            ctx.fill();
            break;
        }
        case PetalID::kMoltenSandBlade: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffBC7846);
            ctx.set_stroke(0xff6C4539);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(8, -10);
            ctx.line_to(10, -4);
            ctx.line_to(2, 6);
            ctx.line_to(-2, 4);
            ctx.line_to(2, -6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE9B66B);
            ctx.begin_path();
            ctx.move_to(7, -8);
            ctx.line_to(8, -4);
            ctx.line_to(3, 2);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffFFAE48);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(6, -6);
            ctx.line_to(1, 3);
            ctx.stroke();
            ctx.set_stroke(0xff7B4B3D);
            ctx.set_line_width(2.8f);
            ctx.begin_path();
            ctx.move_to(-1, 4);
            ctx.line_to(-4, 9);
            ctx.stroke();
            break;
        }
        case PetalID::kVolcanoWorm: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff74453A);
            ctx.set_line_width(7.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-9, 6);
            ctx.bcurve_to(-6, -4, 4, -6, 9, -2);
            ctx.stroke();
            ctx.set_stroke(0xffBD7951);
            ctx.set_line_width(4.5f);
            ctx.begin_path();
            ctx.move_to(-9, 6);
            ctx.bcurve_to(-6, -4, 4, -6, 9, -2);
            ctx.stroke();
            ctx.set_stroke(0xffE9AD68);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-5, 2);
            ctx.line_to(-3, -1);
            ctx.move_to(0, -2);
            ctx.line_to(2, -4);
            ctx.move_to(4, -3);
            ctx.line_to(6, -1);
            ctx.stroke();
            ctx.set_fill(0xffA45B45);
            ctx.begin_path();
            ctx.arc(9, -2, 3.2f);
            ctx.fill();
            ctx.set_fill(0xffFF9B49);
            ctx.begin_path();
            ctx.move_to(11, -2);
            ctx.line_to(14, -3);
            ctx.line_to(11, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kScorchPalm: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB86E42);
            ctx.set_stroke(0xff703D36);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -8);
            ctx.bcurve_to(9, -5, 10, 5, 3, 10);
            ctx.bcurve_to(0, 11, -4, 9, -8, 4);
            ctx.bcurve_to(-10, -4, -6, -8, 0, -8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE7A45A);
            ctx.begin_path();
            ctx.ellipse(0, 0, 5.f, 6.f);
            ctx.fill();
            ctx.set_stroke(0xff9D4937);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-1, -4);
            ctx.line_to(1, 1);
            ctx.line_to(-2, 5);
            ctx.stroke();
            ctx.set_stroke(0xffFFCD72);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.move_to(-1, -3);
            ctx.line_to(1, 1);
            ctx.stroke();
            break;
        }
        case PetalID::kGlassStorm: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xffA67A58);
            ctx.set_line_width(4.2f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-9, 1);
            ctx.bcurve_to(-8, -10, 8, -9, 9, 1);
            ctx.stroke();
            ctx.set_stroke(0xffC8A277);
            ctx.set_line_width(2.8f);
            ctx.begin_path();
            ctx.move_to(8, 0);
            ctx.bcurve_to(9, 9, -7, 10, -8, 1);
            ctx.stroke();
            ctx.set_fill(0xffD9E7E7);
            ctx.set_stroke(0xff7F9A9E);
            ctx.set_line_width(1.2f);
            auto glass = [&](float x, float y) {
                ctx.begin_path();
                ctx.move_to(x, y - 2);
                ctx.line_to(x + 1.5f, y);
                ctx.line_to(x, y + 2);
                ctx.line_to(x - 1.5f, y);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            };
            glass(-8, -5); glass(8, -4); glass(7, 6); glass(-7, 6); glass(0, -9);
            break;
        }
        case PetalID::kFrostKnightSword: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8FBFD5);
            ctx.set_stroke(0xff3F6886);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(6, -11);
            ctx.line_to(9, -8);
            ctx.line_to(2, 4);
            ctx.line_to(-2, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE1F6FC);
            ctx.begin_path();
            ctx.move_to(6, -10);
            ctx.line_to(7.5f, -8);
            ctx.line_to(1, 2);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff6FA5C2);
            ctx.set_line_width(3.2f);
            ctx.begin_path();
            ctx.move_to(-5, 0);
            ctx.line_to(4, 5);
            ctx.move_to(-3, 3);
            ctx.line_to(2, -1);
            ctx.stroke();
            ctx.set_stroke(0xff526178);
            ctx.set_line_width(2.6f);
            ctx.begin_path();
            ctx.move_to(-1, 4);
            ctx.line_to(-4, 9);
            ctx.stroke();
            break;
        }
        case PetalID::kHolySnowflake: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff477F9F);
            ctx.set_line_width(3.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            for (int i = 0; i < 3; ++i) {
                float a = i * (float)M_PI / 3.f;
                ctx.move_to(std::cos(a) * 10.f, std::sin(a) * 10.f);
                ctx.line_to(std::cos(a + (float)M_PI) * 10.f, std::sin(a + (float)M_PI) * 10.f);
            }
            ctx.stroke();
            ctx.set_stroke(0xffD5F0F7);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            for (int i = 0; i < 3; ++i) {
                float a = i * (float)M_PI / 3.f;
                ctx.move_to(std::cos(a) * 10.f, std::sin(a) * 10.f);
                ctx.line_to(std::cos(a + (float)M_PI) * 10.f, std::sin(a + (float)M_PI) * 10.f);
            }
            ctx.stroke();
            ctx.set_stroke(0xff8CC9DE);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                float x = std::cos(a) * 6.f, y = std::sin(a) * 6.f;
                ctx.move_to(x, y);
                ctx.line_to(x + std::cos(a + 0.8f) * 2.5f, y + std::sin(a + 0.8f) * 2.5f);
                ctx.move_to(x, y);
                ctx.line_to(x + std::cos(a - 0.8f) * 2.5f, y + std::sin(a - 0.8f) * 2.5f);
            }
            ctx.stroke();
            ctx.set_fill(0xffF4E2A4);
            ctx.set_stroke(0xffB69A5C);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 3.2f);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kPermafrostGear: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff6A94AB);
            ctx.set_stroke(0xff345C78);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a0 = i * (float)M_PI / 4.f;
                float a1 = a0 + 0.2f, a2 = a0 + (float)M_PI / 4.f - 0.2f, a3 = a0 + (float)M_PI / 4.f;
                float o = 11.5f, inn = 8.5f;
                if (i == 0) ctx.move_to(std::cos(a0) * inn, std::sin(a0) * inn);
                ctx.line_to(std::cos(a1) * o, std::sin(a1) * o);
                ctx.line_to(std::cos(a2) * o, std::sin(a2) * o);
                ctx.line_to(std::cos(a3) * inn, std::sin(a3) * inn);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff4E88A6);
            ctx.begin_path();
            ctx.arc(0, 0, 5.5f);
            ctx.fill();
            ctx.set_stroke(0xffE7F8FC);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            for (int i = 0; i < 3; ++i) {
                float a = i * (float)M_PI / 3.f;
                ctx.move_to(std::cos(a) * 4.f, std::sin(a) * 4.f);
                ctx.line_to(std::cos(a + (float)M_PI) * 4.f, std::sin(a + (float)M_PI) * 4.f);
            }
            ctx.stroke();
            ctx.set_fill(0xffBCE2EE);
            ctx.begin_path();
            ctx.arc(0, 0, 2.f);
            ctx.fill();
            break;
        }
        case PetalID::kFrostCrown: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8CBBD4);
            ctx.set_stroke(0xff3D6E90);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-10, 4);
            ctx.line_to(-8, -2);
            ctx.line_to(-5, 2);
            ctx.line_to(-2, -8);
            ctx.line_to(0, 0);
            ctx.line_to(2, -8);
            ctx.line_to(5, 2);
            ctx.line_to(8, -2);
            ctx.line_to(10, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE0F5FB);
            ctx.begin_path();
            ctx.move_to(-2, -6);
            ctx.line_to(0, -1);
            ctx.line_to(2, -6);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffC6A866);
            ctx.begin_path();
            ctx.rect(-8, 4, 16, 3.5f);
            ctx.fill();
            ctx.set_fill(0xffF7E4A5);
            ctx.begin_path();
            ctx.move_to(-4, 5); ctx.line_to(-3, 7); ctx.line_to(-5, 7); ctx.close_path();
            ctx.move_to(0, 5); ctx.line_to(1, 7); ctx.line_to(-1, 7); ctx.close_path();
            ctx.move_to(4, 5); ctx.line_to(5, 7); ctx.line_to(3, 7); ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kRoyalThorn: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffB7C77C);
            ctx.set_stroke(0xff4E704A);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0, -11);
            ctx.line_to(3, 2);
            ctx.line_to(2, 11);
            ctx.line_to(-2, 11);
            ctx.line_to(-3, 2);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE0E7A3);
            ctx.begin_path();
            ctx.move_to(0, -9);
            ctx.line_to(1.5f, 2);
            ctx.line_to(0, 8);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffD5AD67);
            ctx.begin_path();
            ctx.move_to(-3, -2); ctx.line_to(-7, -1); ctx.line_to(-3, 1); ctx.close_path();
            ctx.move_to(3, 0); ctx.line_to(7, 1); ctx.line_to(3, 2); ctx.close_path();
            ctx.move_to(-3, 4); ctx.line_to(-6, 6); ctx.line_to(-2, 6); ctx.close_path();
            ctx.move_to(3, 5); ctx.line_to(6, 7); ctx.line_to(2, 7); ctx.close_path();
            ctx.fill();
            ctx.begin_path();
            ctx.move_to(-2, -10);
            ctx.line_to(-2.5f, -7);
            ctx.line_to(0, -8);
            ctx.line_to(2.5f, -7);
            ctx.line_to(2, -10);
            ctx.line_to(0, -9);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMechPoisonBee: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA5C9D2);
            ctx.set_stroke(0xff4A5960);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.ellipse(-7, -2, 5.f, 3.5f, -0.3f);
            ctx.ellipse(7, -2, 5.f, 3.5f, 0.3f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC6A65B);
            ctx.begin_path();
            ctx.ellipse(0, 0, 4.5f, 8.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff4A5960);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.move_to(-3, -3); ctx.line_to(3, -3);
            ctx.move_to(-3, 0); ctx.line_to(3, 0);
            ctx.move_to(-3, 3); ctx.line_to(3, 3);
            ctx.stroke();
            ctx.set_fill(0xffA5D66B);
            ctx.begin_path();
            ctx.move_to(0, 7);
            ctx.line_to(1.5f, 12);
            ctx.line_to(-1.5f, 12);
            ctx.close_path();
            ctx.fill();
            ctx.begin_path();
            ctx.arc(-1.5f, -6, 1.0f);
            ctx.arc(1.5f, -6, 1.0f);
            ctx.fill();
            break;
        }
        case PetalID::kHolySpore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff80639A);
            ctx.set_stroke(0xff4C405D);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 10.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB393C1);
            ctx.begin_path();
            ctx.arc(0, 0, 7.f);
            ctx.fill();
            ctx.set_fill(0xffC9D67D);
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.line_to(3, 0);
            ctx.line_to(0, 5);
            ctx.line_to(-3, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffFFF4C3);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(0, -3); ctx.line_to(0, 3);
            ctx.move_to(-3, 0); ctx.line_to(3, 0);
            ctx.stroke();
            ctx.set_fill(0xffA6CF72);
            ctx.begin_path();
            ctx.arc(-9, -4, 1.4f);
            ctx.arc(9, -3, 1.3f);
            ctx.arc(7, 7, 1.2f);
            ctx.arc(-7, 7, 1.1f);
            ctx.fill();
            break;
        }
        case PetalID::kTreantGuard: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff775B42);
            ctx.set_stroke(0xff3D4736);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-8, -6);
            ctx.line_to(-5, -10);
            ctx.line_to(5, -10);
            ctx.line_to(8, -6);
            ctx.line_to(9, 4);
            ctx.line_to(0, 11);
            ctx.line_to(-9, 4);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB18A5D);
            ctx.begin_path();
            ctx.move_to(-5, -4);
            ctx.line_to(5, -4);
            ctx.line_to(6, 3);
            ctx.line_to(0, 8);
            ctx.line_to(-6, 3);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff654C3A);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(0, -4); ctx.line_to(0, 7);
            ctx.move_to(-3, 0); ctx.line_to(-4, 4);
            ctx.move_to(3, 0); ctx.line_to(4, 4);
            ctx.stroke();
            ctx.set_fill(0xff78A66A);
            ctx.begin_path();
            ctx.move_to(-4, -10); ctx.line_to(-6, -13); ctx.line_to(-2, -11); ctx.close_path();
            ctx.move_to(4, -10); ctx.line_to(6, -13); ctx.line_to(2, -11); ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kHeartOfNature: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff5B9D65);
            ctx.set_stroke(0xff2F6048);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, 10);
            ctx.bcurve_to(11, 3, 10, -8, 0, -6);
            ctx.bcurve_to(-10, -8, -11, 3, 0, 10);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA7D48B);
            ctx.begin_path();
            ctx.move_to(0, 6);
            ctx.bcurve_to(7, 1, 6, -4, 0, -3);
            ctx.bcurve_to(-6, -4, -7, 1, 0, 6);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffE7D58C);
            ctx.begin_path();
            ctx.move_to(0, 3);
            ctx.bcurve_to(4, 0, 3, -2, 0, -1.5f);
            ctx.bcurve_to(-3, -2, -4, 0, 0, 3);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffFFF5BC);
            ctx.begin_path();
            ctx.arc(0, 0, 1.4f);
            ctx.fill();
            break;
        }
        case PetalID::kAbyssalTide: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff123D60);
            ctx.set_line_width(5.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 9.5f);
            ctx.stroke();
            ctx.set_stroke(0xff2D91B2);
            ctx.set_line_width(3.0f);
            ctx.begin_path();
            ctx.arc(0, 0, 9.5f);
            ctx.stroke();
            ctx.set_fill(0xff60C6D5);
            for (int i = 0; i < 4; ++i) {
                float a = i * (float)M_PI / 2.f;
                float x = std::cos(a) * 11.f, y = std::sin(a) * 11.f;
                ctx.begin_path();
                ctx.move_to(x, y);
                ctx.bcurve_to(x + std::cos(a) * 4.f, y + std::sin(a) * 4.f,
                              x + std::cos(a + 0.8f) * 3.f, y + std::sin(a + 0.8f) * 3.f, x, y);
                ctx.fill();
            }
            ctx.set_fill(0xff1A577B);
            ctx.begin_path();
            ctx.arc(0, 0, 2.f);
            ctx.fill();
            break;
        }
        case PetalID::kInfernalCore: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff25232E);
            ctx.set_stroke(0xff11121A);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 11.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff913E37);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f;
                float px = std::cos(a) * 8.f, py = std::sin(a) * 8.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffF07539);
            ctx.begin_path();
            ctx.arc(0, 0, 5.f);
            ctx.fill();
            ctx.set_fill(0xffFFE3A0);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(2.5f, 0);
            ctx.line_to(0, 4);
            ctx.line_to(-2.5f, 0);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kChronoVortex: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8497AF);
            ctx.set_stroke(0xff252F48);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff49566F);
            ctx.begin_path();
            ctx.arc(0, 0, 8.5f);
            ctx.fill();
            ctx.set_stroke(0xffE6E8DD);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            for (int i = 0; i < 12; ++i) {
                float a = i * (float)M_PI / 6.f;
                ctx.move_to(std::cos(a) * 9.f, std::sin(a) * 9.f);
                ctx.line_to(std::cos(a) * 10.5f, std::sin(a) * 10.5f);
            }
            ctx.stroke();
            ctx.set_stroke(0xffD7C48D);
            ctx.set_line_width(2.8f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(0, -5);
            ctx.bcurve_to(6, -4, 5, 4, 0, 5);
            ctx.bcurve_to(-4, 3, -3, -2, 0, -1);
            ctx.stroke();
            ctx.set_fill(0xffFFF0BD);
            ctx.begin_path();
            ctx.arc(0, 0, 1.5f);
            ctx.fill();
            break;
        }
        case PetalID::kAntKing: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE5BF71);
            ctx.set_stroke(0xff8A6944);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-5, -8);
            ctx.line_to(-6, -11);
            ctx.line_to(-2, -9);
            ctx.line_to(0, -12);
            ctx.line_to(2, -9);
            ctx.line_to(6, -11);
            ctx.line_to(5, -8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff9D6C47);
            ctx.set_stroke(0xff503E36);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 5.5f, 8.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC99A63);
            ctx.begin_path();
            ctx.ellipse(0, 2, 3.5f, 5.f);
            ctx.fill();
            ctx.set_stroke(0xff503E36);
            ctx.set_line_width(2.0f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-5, -2); ctx.line_to(-10, -4);
            ctx.move_to(-5, 2); ctx.line_to(-10, 3);
            ctx.move_to(5, -2); ctx.line_to(10, -4);
            ctx.move_to(5, 2); ctx.line_to(10, 3);
            ctx.stroke();
            break;
        }
        case PetalID::kWorldTreeSeed: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff6F9C5D);
            ctx.set_stroke(0xff365F43);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 7.5f, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB8D78A);
            ctx.begin_path();
            ctx.ellipse(0, 0, 5.f, 8.f);
            ctx.fill();
            ctx.set_stroke(0xff5D9862);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(0, 5);
            ctx.line_to(0, -4);
            ctx.move_to(0, -1);
            ctx.line_to(-3, -3);
            ctx.move_to(0, -1);
            ctx.line_to(3, -3);
            ctx.move_to(0, 2);
            ctx.line_to(-2, 0);
            ctx.move_to(0, 2);
            ctx.line_to(2, 0);
            ctx.stroke();
            ctx.set_fill(0xffE5C77D);
            ctx.begin_path();
            ctx.arc(0, 1, 2.2f);
            ctx.fill();
            ctx.set_fill(0xffD8E9AC);
            ctx.begin_path();
            ctx.move_to(-1, -10);
            ctx.line_to(-2, -13);
            ctx.move_to(1, -10);
            ctx.line_to(2, -13);
            ctx.set_stroke(0xff365F43);
            ctx.set_line_width(1.6f);
            ctx.stroke();
            break;
        }
        case PetalID::kSkyblade: {
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff7FB8BD);
            ctx.set_stroke(0xff356D79);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(7, -11);
            ctx.line_to(9, -8);
            ctx.line_to(1, 5);
            ctx.line_to(-2, 3);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD8F3E9);
            ctx.begin_path();
            ctx.move_to(7, -10);
            ctx.line_to(8, -8);
            ctx.line_to(1, 3);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffE2DCA3);
            ctx.set_line_width(3.0f);
            ctx.begin_path();
            ctx.move_to(-4, 2);
            ctx.line_to(3, 6);
            ctx.stroke();
            ctx.set_stroke(0xffA7BD78);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-1, 5);
            ctx.line_to(-4, 10);
            ctx.stroke();
            ctx.set_stroke(0xffD8F3E9);
            ctx.set_line_width(1.4f);
            ctx.begin_path();
            ctx.partial_arc(5, -4, 6.f, -1.2f, 0.2f, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kChaosPrism: {
            ctx.scale(r / 15.f);
            ctx.set_stroke(0xff303C60);
            ctx.set_line_width(2.0f);
            uint32_t cols[6] = { 0xffA6D9E9, 0xffE69D75, 0xff79C18B, 0xffE7BD7D, 0xff9A8BD0, 0xffD78FAD };
            for (int i = 0; i < 6; ++i) {
                float a0 = i * (float)M_PI / 3.f - (float)M_PI / 6.f;
                float a1 = a0 + (float)M_PI / 3.f;
                ctx.set_fill(cols[i]);
                ctx.begin_path();
                ctx.move_to(0, 0);
                ctx.line_to(std::cos(a0) * 10.f, std::sin(a0) * 10.f);
                ctx.line_to(std::cos(a1) * 10.f, std::sin(a1) * 10.f);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
            }
            ctx.set_fill(0xff303C60);
            ctx.begin_path();
            ctx.move_to(0, -4);
            ctx.line_to(3, 0);
            ctx.line_to(0, 4);
            ctx.line_to(-3, 0);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(0, 0, 1.4f);
            ctx.fill();
            break;
        }

        // ---- Ant Hell craft exclusives (anthell.txt G01–G12) ----
        case PetalID::kChitinNeedle: {
            // Long straight chitin spike — thinner than Ant Fang
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffD9BC8C);
            ctx.set_stroke(0xff80634C);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -13);
            ctx.bcurve_to(3.2f, -4, 2.6f, 6, 1.4f, 12);
            ctx.line_to(-1.4f, 12);
            ctx.bcurve_to(-2.6f, 6, -3.2f, -4, 0, -13);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF3DDB0);
            ctx.begin_path();
            ctx.move_to(0, -10);
            ctx.line_to(1.1f, 2);
            ctx.line_to(0, 8);
            ctx.line_to(-1.1f, 2);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kShellPlate: {
            // Hex shell plate with segment lines
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff9B7857);
            ctx.set_stroke(0xff574B3E);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f - (float)M_PI / 6.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 12.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD4B18A);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f - (float)M_PI / 6.f;
                float px = std::cos(a) * 6.5f, py = std::sin(a) * 7.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff574B3E);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(0, -7);
            ctx.line_to(0, 7);
            ctx.move_to(-5.5f, -2.5f);
            ctx.line_to(5.5f, -2.5f);
            ctx.move_to(-5.5f, 2.5f);
            ctx.line_to(5.5f, 2.5f);
            ctx.stroke();
            break;
        }
        case PetalID::kAcidDroplet: {
            // Single venom drop (not a flask)
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8AB45E);
            ctx.set_stroke(0xff4E7846);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0, -12);
            ctx.bcurve_to(7, -4, 8, 6, 0, 11);
            ctx.bcurve_to(-8, 6, -7, -4, 0, -12);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffCDE58B);
            ctx.begin_path();
            ctx.ellipse(0, 2, 4.5f, 5.5f);
            ctx.fill();
            ctx.set_fill(0xaaffffff);
            ctx.begin_path();
            ctx.ellipse(-2, -3, 1.8f, 2.6f);
            ctx.fill();
            break;
        }
        case PetalID::kBroodPearl: {
            // Smooth brood egg pearl
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffE7D6B1);
            ctx.set_stroke(0xff99866A);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 8.f, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffC5B08D);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-5, -2);
            ctx.qcurve_to(0, -5, 5, -1);
            ctx.move_to(-4.5f, 3);
            ctx.qcurve_to(0, 1, 4.5f, 4);
            ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(-2.5f, -4, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kPheromoneStar: {
            // Regular 5-point star + dual core
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC58C5C);
            ctx.set_stroke(0xff75513C);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 10; ++i) {
                float a = i * (float)M_PI / 5.f - (float)M_PI / 2.f;
                float rr = (i & 1) ? 5.2f : 11.5f;
                float px = std::cos(a) * rr, py = std::sin(a) * rr;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE9BE7C);
            ctx.begin_path();
            ctx.arc(0, 0, 4.2f);
            ctx.fill();
            ctx.set_fill(0xffFFF0C8);
            ctx.begin_path();
            ctx.arc(0, 0, 2.0f);
            ctx.fill();
            break;
        }
        case PetalID::kTunnelStone: {
            // Rounded underground rock (no lava cracks)
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff8A674C);
            ctx.set_stroke(0xff493E35);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-3, -11);
            ctx.bcurve_to(6, -12, 11, -4, 10, 3);
            ctx.bcurve_to(10, 10, 3, 12, -4, 11);
            ctx.bcurve_to(-11, 9, -12, 1, -10, -5);
            ctx.bcurve_to(-8, -10, -6, -11, -3, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC6A077);
            ctx.begin_path();
            ctx.move_to(-2, -7);
            ctx.bcurve_to(4, -8, 6, -2, 5, 2);
            ctx.bcurve_to(3, 5, -1, 4, -3, 1);
            ctx.bcurve_to(-5, -2, -4, -5, -2, -7);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xaaffffff);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-4, -5);
            ctx.qcurve_to(0, -8, 4, -3);
            ctx.stroke();
            break;
        }
        case PetalID::kIronMandible: {
            // Hardened mandible plate with Y ridge
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff7A6A5B);
            ctx.set_stroke(0xff403F3A);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 10.f, py = std::sin(a) * 12.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB5A085);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 6.2f, py = std::sin(a) * 7.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff403F3A);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(0, -6);
            ctx.line_to(0, 2);
            ctx.line_to(-4, 7);
            ctx.move_to(0, 2);
            ctx.line_to(4, 7);
            ctx.stroke();
            break;
        }
        case PetalID::kHiveDisc: {
            // Thick disc with 6 spokes (not a gear)
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA17B59);
            ctx.set_stroke(0xff614938);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, 11.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE2C18D);
            ctx.begin_path();
            ctx.arc(0, 0, 7.5f);
            ctx.fill();
            ctx.set_stroke(0xff614938);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                ctx.move_to(0, 0);
                ctx.line_to(std::cos(a) * 10.f, std::sin(a) * 10.f);
            }
            ctx.stroke();
            ctx.set_fill(0xff614938);
            ctx.begin_path();
            ctx.arc(0, 0, 3.2f);
            ctx.fill();
            ctx.set_fill(0xffE2C18D);
            ctx.begin_path();
            ctx.arc(0, 0, 1.6f);
            ctx.fill();
            break;
        }
        case PetalID::kVenomCocoon: {
            // Closed cocoon with toxic core
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff785F85);
            ctx.set_stroke(0xff453E59);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9.f, 12.f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB59BC4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 6.5f, 9.f);
            ctx.fill();
            ctx.set_fill(0xffD6E19A);
            ctx.begin_path();
            ctx.arc(0, 1, 3.5f);
            ctx.fill();
            ctx.set_fill(0x88ffffff);
            ctx.begin_path();
            ctx.ellipse(-2.5f, -4, 2.0f, 3.2f);
            ctx.fill();
            break;
        }
        case PetalID::kRoyalCarapace: {
            // Oct royal plate + gold inner + cross ridge
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffA8794F);
            ctx.set_stroke(0xff604635);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 12.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE2BA78);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 7.f, py = std::sin(a) * 8.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff604635);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(0, -7);
            ctx.line_to(0, 7);
            ctx.move_to(-6, 0);
            ctx.line_to(6, 0);
            ctx.stroke();
            ctx.set_fill(0xffFFF0C0);
            ctx.begin_path();
            ctx.arc(0, 0, 2.2f);
            ctx.fill();
            break;
        }
        case PetalID::kQueensEmblem: {
            // Dual oct badge + crown lines + amber core
            ctx.scale(r / 15.f);
            ctx.set_fill(0xffC49A5F);
            ctx.set_stroke(0xff75543A);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 11.f, py = std::sin(a) * 12.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF0D28C);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 7.5f, py = std::sin(a) * 8.2f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff75543A);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-4, -1);
            ctx.line_to(-2.5f, -5);
            ctx.line_to(0, -2);
            ctx.line_to(2.5f, -5);
            ctx.line_to(4, -1);
            ctx.stroke();
            ctx.set_fill(0xffE8A84A);
            ctx.begin_path();
            ctx.arc(0, 3, 2.8f);
            ctx.fill();
            break;
        }
        case PetalID::kColonyHeart: {
            // Amber colony core (not purple Colony Core)
            ctx.scale(r / 15.f);
            ctx.set_fill(0xff76583F);
            ctx.set_stroke(0xff3F382E);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 12.f, py = std::sin(a) * 12.f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB78C57);
            ctx.begin_path();
            for (int i = 0; i < 8; ++i) {
                float a = i * (float)M_PI / 4.f - (float)M_PI / 8.f;
                float px = std::cos(a) * 8.5f, py = std::sin(a) * 8.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xffE9C77F);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                float px = std::cos(a) * 4.5f, py = std::sin(a) * 4.5f;
                if (i == 0) ctx.move_to(px, py); else ctx.line_to(px, py);
            }
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xff3F382E);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            for (int i = 0; i < 4; ++i) {
                float a = i * (float)M_PI / 2.f + (float)M_PI / 4.f;
                ctx.move_to(std::cos(a) * 5.5f, std::sin(a) * 5.5f);
                ctx.line_to(std::cos(a) * 9.5f, std::sin(a) * 9.5f);
            }
            ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(0, 0, 1.6f);
            ctx.fill();
            break;
        }

        case PetalID::kHoney: {
            // Rounded amber hex — thick sticky honeycomb candy (icon + projectile)
            ctx.scale(r / 12.f);
            ctx.round_line_join();
            ctx.round_line_cap();
            float vx[6], vy[6];
            float const R = 13.5f;
            float const t = 0.28f;
            for (int i = 0; i < 6; ++i) {
                float a = i * (float)M_PI / 3.f;
                vx[i] = std::cos(a) * R;
                vy[i] = std::sin(a) * R;
            }
            auto path_rounded_hex = [&](float sx, float sy, float scale) {
                ctx.begin_path();
                for (int i = 0; i < 6; ++i) {
                    int ip = (i + 5) % 6;
                    int in = (i + 1) % 6;
                    float cx = vx[i] * scale + sx;
                    float cy = vy[i] * scale + sy;
                    float px = (vx[i] + t * (vx[ip] - vx[i])) * scale + sx;
                    float py = (vy[i] + t * (vy[ip] - vy[i])) * scale + sy;
                    float qx = (vx[i] + t * (vx[in] - vx[i])) * scale + sx;
                    float qy = (vy[i] + t * (vy[in] - vy[i])) * scale + sy;
                    if (i == 0) ctx.move_to(px, py);
                    else ctx.line_to(px, py);
                    ctx.qcurve_to(cx, cy, qx, qy);
                }
                ctx.close_path();
            };
            path_rounded_hex(0, 0, 1.f);
            ctx.set_fill(0xffF7CF2F);
            ctx.fill();
            path_rounded_hex(1.2f, 1.6f, 0.78f);
            ctx.set_fill(0xffC8A826);
            ctx.fill();
            ctx.set_fill(0xffE7B647);
            ctx.begin_path();
            ctx.move_to(-2.5f, 1.5f);
            ctx.qcurve_to(-4.5f, 5.5f, 0.5f, 6.2f);
            ctx.qcurve_to(5.0f, 5.0f, 3.5f, 1.0f);
            ctx.qcurve_to(1.0f, -1.5f, -2.5f, 1.5f);
            ctx.close_path();
            ctx.fill();
            path_rounded_hex(0, 0, 1.f);
            ctx.set_stroke(0xff8A6A1E);
            ctx.set_line_width(2.6f);
            ctx.stroke();
            ctx.set_stroke(0xffFFF1A0);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.partial_arc(-2.5f, -4.5f, 5.5f, 3.6f, 5.1f, 0);
            ctx.stroke();
            ctx.set_fill(0x66FFF8D0);
            ctx.begin_path();
            ctx.arc(-3.5f, -1.5f, 1.4f);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(4.0f, 2.5f, 1.0f);
            ctx.fill();
            ctx.set_stroke(0xffC8A826);
            ctx.set_line_width(0.7f);
            ctx.begin_path();
            ctx.arc(-3.5f, -1.5f, 1.4f);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(4.0f, 2.5f, 1.0f);
            ctx.stroke();
            break;
        }
        case PetalID::kMeteor: {
            // Top-down hollow crater ring — standable center, rocky blocking rim
            ctx.scale(r / 15.f);
            ctx.round_line_join();
            ctx.round_line_cap();
            float const orad[8] = {
                14.6f, 13.4f, 14.9f, 13.2f, 14.5f, 13.7f, 15.0f, 13.6f
            };
            float const hole = 6.3f;
            auto path_outer = [&]() {
                ctx.begin_path();
                for (int i = 0; i < 8; ++i) {
                    float a = i * (float)M_PI / 4.f - 0.18f;
                    float x = std::cos(a) * orad[i];
                    float y = std::sin(a) * orad[i];
                    if (i == 0) ctx.move_to(x, y);
                    else ctx.line_to(x, y);
                }
                ctx.close_path();
            };
            auto path_ring = [&]() {
                path_outer();
                ctx.reverse_arc(0, 0, hole);
            };
            ctx.set_fill(0x28000000);
            ctx.begin_path();
            ctx.ellipse(0.5f, 1.8f, 15.5f, 14.2f);
            ctx.fill();
            path_ring();
            ctx.set_fill(0xff4A4558);
            ctx.fill(1);
            ctx.begin_path();
            ctx.move_to(1.5f, 1.0f);
            ctx.line_to(12.0f, 3.5f);
            ctx.line_to(11.2f, 12.5f);
            ctx.line_to(2.5f, 13.2f);
            ctx.line_to(-0.5f, 8.0f);
            ctx.close_path();
            ctx.reverse_arc(0, 0, hole);
            ctx.set_fill(0xff2E2B38);
            ctx.fill(1);
            ctx.set_stroke(0xff1A1822);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, 0, hole + 0.35f);
            ctx.stroke();
            ctx.set_stroke(0xff8B3A22);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(0, 0, hole + 1.5f);
            ctx.stroke();
            path_outer();
            ctx.set_stroke(0xff14131A);
            ctx.set_line_width(2.6f);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0, 0, hole);
            ctx.stroke();
            ctx.set_stroke(0xffC45A32);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(8.5f, -6.5f);
            ctx.line_to(11.8f, -4.0f);
            ctx.move_to(-10.5f, 3.0f);
            ctx.line_to(-13.2f, 5.5f);
            ctx.move_to(4.0f, 11.0f);
            ctx.line_to(7.2f, 13.0f);
            ctx.stroke();
            ctx.set_stroke(0xff8A8499);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.partial_arc(-6.5f, -7.5f, 4.5f, 3.4f, 5.0f, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kCorruption: {
            // Faded gray skull — Corruption
            float s = fmaxf(8.f, r);
            ctx.set_fill(0x99b0b0b0);
            ctx.set_stroke(0xff6a6a6a);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.ellipse(0, -1.f, s * 0.85f, s * 0.95f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff2a2a2a);
            ctx.begin_path();
            ctx.ellipse(-s * 0.28f, -s * 0.15f, s * 0.18f, s * 0.22f);
            ctx.ellipse(s * 0.28f, -s * 0.15f, s * 0.18f, s * 0.22f);
            ctx.fill();
            ctx.set_stroke(0xff2a2a2a);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-s * 0.35f, s * 0.35f);
            ctx.line_to(-s * 0.12f, s * 0.55f);
            ctx.line_to(0, s * 0.35f);
            ctx.line_to(s * 0.12f, s * 0.55f);
            ctx.line_to(s * 0.35f, s * 0.35f);
            ctx.stroke();
            break;
        }
        case PetalID::kBloodSacrifice: {
            // Blood ritual drop
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffb02030);
            ctx.set_stroke(0xff701018);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(0, -s * 1.05f);
            ctx.qcurve_to(s * 0.95f, -s * 0.15f, 0, s * 0.95f);
            ctx.qcurve_to(-s * 0.95f, -s * 0.15f, 0, -s * 1.05f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x66ff8890);
            ctx.begin_path();
            ctx.ellipse(-s * 0.18f, -s * 0.25f, s * 0.22f, s * 0.3f);
            ctx.fill();
            break;
        }
        case PetalID::kPharaohCrown: {
            // Cartoon nemes: gold plate + deep blue stripes + uraeus on the brow.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8c04a);
            ctx.set_stroke(0xff8a6a20);
            ctx.set_line_width(2.2f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-s * 0.95f, -s * 0.15f);
            ctx.line_to(-s * 0.75f, -s * 1.05f);
            ctx.line_to(s * 0.75f, -s * 1.05f);
            ctx.line_to(s * 0.95f, -s * 0.15f);
            ctx.line_to(s * 0.55f, s * 0.95f);
            ctx.line_to(s * 0.2f, s * 0.35f);
            ctx.line_to(-s * 0.2f, s * 0.35f);
            ctx.line_to(-s * 0.55f, s * 0.95f);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff1e3a8a);
            ctx.set_line_width(s * 0.18f);
            ctx.round_line_cap();
            for (int i = 0; i < 4; ++i) {
                float t = -0.55f + i * 0.35f;
                ctx.begin_path();
                ctx.move_to(s * t, -s * 0.95f);
                ctx.line_to(s * t * 1.05f, s * 0.2f);
                ctx.stroke();
            }
            ctx.set_fill(0xff2a8a3a);
            ctx.set_stroke(0xff145020);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(0, -s * 1.05f);
            ctx.line_to(s * 0.12f, -s * 0.7f);
            ctx.line_to(0, -s * 0.55f);
            ctx.line_to(-s * 0.12f, -s * 0.7f);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xfff0d878);
            ctx.begin_path();
            ctx.arc(0, -s * 0.35f, s * 0.16f);
            ctx.fill();
            break;
        }
        case PetalID::kUranium: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff7ecf4a);
            ctx.set_stroke(0xff4a8a28);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, s);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffc8ff70);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.4f);
            ctx.fill();
            ctx.set_stroke(0xffe8ff90);
            ctx.set_line_width(1.8f);
            for (int i = 0; i < 6; ++i) {
                float a = i * 1.0472f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * s * 0.5f, sinf(a) * s * 0.5f);
                ctx.line_to(cosf(a) * s * 0.92f, sinf(a) * s * 0.92f);
                ctx.stroke();
            }
            break;
        }
        case PetalID::kBattery: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff3a3a3a);
            ctx.set_stroke(0xff1a1a1a);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s * 0.7f, -s * 0.9f, s * 1.4f, s * 1.7f, 2.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffffcc33);
            ctx.begin_path();
            ctx.rect(-s * 0.25f, -s * 1.15f, s * 0.5f, s * 0.3f);
            ctx.fill();
            ctx.set_stroke(0xffffee66);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, s * 0.15f);
            ctx.line_to(s * 0.05f, -s * 0.25f);
            ctx.line_to(-s * 0.05f, -s * 0.05f);
            ctx.line_to(s * 0.2f, -s * 0.55f);
            ctx.stroke();
            break;
        }
        case PetalID::kCarrot: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffff8c2a);
            ctx.set_stroke(0xffcc6010);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0, -s * 1.1f);
            ctx.line_to(s * 0.55f, s * 0.7f);
            ctx.line_to(0, s * 1.05f);
            ctx.line_to(-s * 0.55f, s * 0.7f);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3a9a30);
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, -s * 0.95f);
            ctx.line_to(0, -s * 1.45f);
            ctx.line_to(s * 0.15f, -s * 0.95f);
            ctx.fill();
            break;
        }
        case PetalID::kPrivetBerry: {
            float s = fmaxf(7.f, r);
            ctx.set_fill(0xff5a2060);
            ctx.set_stroke(0xff3a1040);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.arc(0, s * 0.08f, s * 0.85f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff8a40a0);
            ctx.begin_path();
            ctx.arc(-s * 0.22f, -s * 0.15f, s * 0.25f);
            ctx.fill();
            ctx.set_stroke(0xff3a9028);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.55f);
            ctx.line_to(0, -s * 0.95f);
            ctx.stroke();
            ctx.set_fill(0xff4aaa38);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.7f);
            ctx.qcurve_to(s * 0.45f, -s * 0.85f, s * 0.35f, -s * 1.1f);
            ctx.qcurve_to(s * 0.1f, -s * 0.85f, 0, -s * 0.7f);
            ctx.fill();
            break;
        }

        case PetalID::kDomino: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xfff5f0e6);
            ctx.set_stroke(0xffc4b8a0);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s*0.7f, -s*1.0f, s*1.4f, s*2.0f, 2.f);
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, 0); ctx.line_to(s * 0.55f, 0); ctx.stroke();
            ctx.set_fill(0xff303028);
            ctx.begin_path(); ctx.arc(0, -s * 0.45f, s * 0.12f); ctx.fill();
            ctx.begin_path(); ctx.arc(-s * 0.22f, s * 0.4f, s * 0.1f); ctx.fill();
            ctx.begin_path(); ctx.arc(s * 0.22f, s * 0.4f, s * 0.1f); ctx.fill();
            break;
        }
        case PetalID::kCard: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xfff8f8ff);
            ctx.set_stroke(0xff334466);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s*0.75f, -s*1.05f, s*1.5f, s*2.1f, 2.f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffcc2030);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.35f);
            ctx.line_to(s * 0.28f, 0);
            ctx.line_to(0, s * 0.35f);
            ctx.line_to(-s * 0.28f, 0);
            ctx.close_path();
            ctx.fill();
            ctx.begin_path();
            ctx.move_to(-s * 0.45f, -s * 0.75f);
            ctx.line_to(-s * 0.28f, -s * 0.55f);
            ctx.line_to(-s * 0.45f, -s * 0.35f);
            ctx.line_to(-s * 0.62f, -s * 0.55f);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kPokerChip: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffcc2222);
            ctx.set_stroke(0xff881111);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,0,s); ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path(); ctx.arc(0,0,s*0.45f); ctx.fill();
            ctx.set_stroke(0xffffffff);
            ctx.set_line_width(1.6f);
            for (int i = 0; i < 8; ++i) {
                float a = i * 0.7854f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * s * 0.72f, sinf(a) * s * 0.72f);
                ctx.line_to(cosf(a) * s * 0.92f, sinf(a) * s * 0.92f);
                ctx.stroke();
            }
            break;
        }
        case PetalID::kBean: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc8a050);
            ctx.set_stroke(0xff8a6830);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s * 0.9f, 0);
            ctx.qcurve_to(-s * 0.2f, -s * 0.95f, s * 0.55f, -s * 0.55f);
            ctx.qcurve_to(s * 1.05f, 0, s * 0.55f, s * 0.55f);
            ctx.qcurve_to(-s * 0.2f, s * 0.95f, -s * 0.9f, 0);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xffa88850);
            ctx.begin_path();
            ctx.move_to(-s * 0.35f, -s * 0.15f);
            ctx.qcurve_to(0, 0, s * 0.35f, s * 0.1f);
            ctx.stroke();
            break;
        }
        case PetalID::kClay: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc47a4a);
            ctx.set_stroke(0xff8a4a28);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s, -s * 0.2f);
            ctx.line_to(s * 0.2f, -s * 0.85f);
            ctx.line_to(s, s * 0.15f);
            ctx.line_to(-s * 0.15f, s * 0.85f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.move_to(-s * 0.35f, -s * 0.15f);
            ctx.line_to(s * 0.05f, -s * 0.45f);
            ctx.line_to(s * 0.15f, -s * 0.15f);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kTomato: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe03030);
            ctx.set_stroke(0xffa01818);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, s * 0.08f, s * 0.9f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff3a9028);
            ctx.set_stroke(0xff286018);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.55f);
            ctx.line_to(-s * 0.35f, -s * 0.95f);
            ctx.line_to(-s * 0.1f, -s * 0.65f);
            ctx.line_to(s * 0.15f, -s * 1.0f);
            ctx.line_to(s * 0.3f, -s * 0.6f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.arc(-s * 0.28f, -s * 0.05f, s * 0.22f);
            ctx.fill();
            break;
        }
        case PetalID::kDust: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffd8d0b8);
            ctx.set_stroke(0xffa09878);
            ctx.set_line_width(2.0f);
            float ox[4] = {-0.45f, 0.35f, -0.1f, 0.55f};
            float oy[4] = {-0.25f, -0.35f, 0.35f, 0.2f};
            float orr[4] = {0.42f, 0.36f, 0.38f, 0.3f};
            for (int i = 0; i < 4; ++i) {
                ctx.begin_path();
                ctx.arc(ox[i] * s, oy[i] * s, orr[i] * s);
                ctx.fill(); ctx.stroke();
            }
            ctx.set_stroke(0xffb8b098);
            ctx.begin_path();
            ctx.move_to(-s * 0.7f, s * 0.55f);
            ctx.qcurve_to(0, s * 0.85f, s * 0.75f, s * 0.4f);
            ctx.stroke();
            break;
        }
        case PetalID::kBroccoli: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff6a4820);
            ctx.set_stroke(0xff403018);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, s * 0.2f);
            ctx.line_to(s * 0.15f, s * 0.2f);
            ctx.line_to(s * 0.12f, s);
            ctx.line_to(-s * 0.12f, s);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff3a9a40);
            ctx.set_stroke(0xff206028);
            for (int i = 0; i < 5; ++i) {
                float a = -1.2f + i * 0.6f;
                ctx.begin_path();
                ctx.arc(cosf(a) * s * 0.35f, -s * 0.25f + sinf(a) * s * 0.15f, s * 0.38f);
                ctx.fill(); ctx.stroke();
            }
            break;
        }
        case PetalID::kOrange: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffff8c20);
            ctx.set_stroke(0xffcc6010);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.95f);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xffe07018);
            ctx.set_line_width(1.5f);
            for (int i = 0; i < 6; ++i) {
                float a = i * 1.0472f;
                ctx.begin_path();
                ctx.move_to(0, 0);
                ctx.line_to(cosf(a) * s * 0.85f, sinf(a) * s * 0.85f);
                ctx.stroke();
            }
            ctx.set_fill(0xff3a9020);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.85f);
            ctx.line_to(-s * 0.18f, -s * 1.05f);
            ctx.line_to(s * 0.18f, -s * 1.05f);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kPlank: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc49a5a);
            ctx.set_stroke(0xff8a6a38);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s*1.1f, -s*0.45f, s*2.2f, s*0.9f, 2.f);
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.9f, -s * 0.15f);
            ctx.line_to(s * 0.9f, -s * 0.15f);
            ctx.move_to(-s * 0.9f, s * 0.15f);
            ctx.line_to(s * 0.9f, s * 0.15f);
            ctx.stroke();
            ctx.set_fill(0xff6a5030);
            ctx.begin_path(); ctx.arc(-s * 0.7f, 0, s * 0.08f); ctx.fill();
            ctx.begin_path(); ctx.arc(s * 0.7f, 0, s * 0.08f); ctx.fill();
            break;
        }
        case PetalID::kBur: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff8a6a40);
            ctx.set_stroke(0xff5a4028);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,0,s*0.55f); ctx.fill(); ctx.stroke();
            for (int i = 0; i < 8; ++i) {
                float a = i * 0.7854f;
                ctx.begin_path();
                ctx.move_to(cosf(a)*s*0.45f,sinf(a)*s*0.45f);
                ctx.line_to(cosf(a)*s,sinf(a)*s); ctx.stroke();
            }
            break;
        }
        case PetalID::kSoil: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff6a4a28);
            ctx.set_stroke(0xff403018);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s, s * 0.2f);
            ctx.qcurve_to(-s * 0.4f, -s * 0.85f, s * 0.2f, -s * 0.55f);
            ctx.qcurve_to(s, -s * 0.2f, s * 0.85f, s * 0.35f);
            ctx.qcurve_to(0, s * 0.95f, -s, s * 0.2f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff8a6838);
            ctx.begin_path(); ctx.arc(-s * 0.25f, 0, s * 0.18f); ctx.fill();
            ctx.begin_path(); ctx.arc(s * 0.3f, s * 0.15f, s * 0.14f); ctx.fill();
            break;
        }
        case PetalID::kAir: {
            float s = fmaxf(8.f, r);
            ctx.set_stroke(0xff88b0d0);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-s * 0.85f, -s * 0.35f);
            ctx.qcurve_to(-s * 0.2f, -s * 0.85f, s * 0.4f, -s * 0.4f);
            ctx.qcurve_to(s * 0.95f, 0, s * 0.35f, s * 0.45f);
            ctx.qcurve_to(-s * 0.15f, s * 0.9f, -s * 0.75f, s * 0.25f);
            ctx.stroke();
            ctx.set_fill(0x55c8e8ff);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 0.15f);
            ctx.qcurve_to(0, -s * 0.55f, s * 0.45f, -s * 0.1f);
            ctx.qcurve_to(s * 0.15f, s * 0.45f, -s * 0.45f, s * 0.25f);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xaaffffff);
            ctx.begin_path(); ctx.arc(s * 0.15f, -s * 0.25f, s * 0.18f); ctx.fill();
            break;
        }
        case PetalID::kBasil: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff4aaa38);
            ctx.set_stroke(0xff287820);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0,-s); ctx.qcurve_to(s*0.9f,0,0,s); ctx.qcurve_to(-s*0.9f,0,0,-s);
            ctx.fill(); ctx.stroke();
            ctx.begin_path(); ctx.move_to(0,-s*0.7f); ctx.line_to(0,s*0.7f); ctx.stroke();
            break;
        }
        case PetalID::kCotton: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xfff0f0f8);
            ctx.set_stroke(0xffc0c0d0);
            ctx.set_line_width(2.2f);
            float ox[5] = {0, -0.5f, 0.48f, -0.25f, 0.28f};
            float oy[5] = {0.05f, 0.1f, 0.08f, -0.4f, -0.38f};
            float orr[5] = {0.55f, 0.42f, 0.4f, 0.36f, 0.34f};
            for (int i = 0; i < 5; ++i) {
                ctx.begin_path();
                ctx.arc(ox[i] * s, oy[i] * s, orr[i] * s);
                ctx.fill(); ctx.stroke();
            }
            ctx.set_stroke(0xffa0a0b0);
            ctx.begin_path();
            ctx.move_to(0, s * 0.55f);
            ctx.line_to(0, s * 0.95f);
            ctx.stroke();
            break;
        }
        case PetalID::kRubber: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff404050);
            ctx.set_stroke(0xff202028);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,0,s); ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xff707080);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 0.2f);
            ctx.qcurve_to(0, -s * 0.55f, s * 0.55f, -s * 0.15f);
            ctx.move_to(-s * 0.5f, s * 0.25f);
            ctx.qcurve_to(0, s * 0.55f, s * 0.5f, s * 0.2f);
            ctx.stroke();
            ctx.set_fill(0x88ffffff);
            ctx.begin_path(); ctx.arc(-s * 0.25f, -s * 0.3f, s * 0.2f); ctx.fill();
            break;
        }
        case PetalID::kGrapes: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff7040a0);
            ctx.set_stroke(0xff482070);
            ctx.set_line_width(2.2f);
            for (int i = 0; i < 5; ++i) {
                float a = i * 1.2566f - 0.4f;
                float rr = (i < 3) ? 0.4f : 0.32f;
                ctx.begin_path();
                ctx.arc(cosf(a) * s * 0.38f, sinf(a) * s * 0.38f + s * 0.1f, s * rr);
                ctx.fill(); ctx.stroke();
            }
            ctx.set_stroke(0xff3a9030);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.55f);
            ctx.line_to(0, -s * 0.95f);
            ctx.stroke();
            break;
        }
        case PetalID::kDeadLeaf: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffb07030);
            ctx.set_stroke(0xff804818);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0,-s); ctx.qcurve_to(s*0.9f,0,0,s); ctx.qcurve_to(-s*0.9f,0,0,-s);
            ctx.fill(); ctx.stroke();
            ctx.begin_path(); ctx.move_to(0,-s*0.7f); ctx.line_to(0,s*0.7f); ctx.stroke();
            break;
        }
        case PetalID::kClover: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff30a048);
            ctx.set_stroke(0xff187830);
            ctx.set_line_width(2.2f);
            for (int i = 0; i < 3; ++i) {
                float a = -1.5708f + i * 2.094395f;
                ctx.begin_path();
                ctx.ellipse(cosf(a)*s*0.35f, sinf(a)*s*0.35f, s*0.38f, s*0.5f);
                ctx.fill();
            }
            ctx.begin_path(); ctx.move_to(0,0); ctx.line_to(0,s); ctx.stroke();
            break;
        }
        case PetalID::kTotem: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc8a060);
            ctx.set_stroke(0xff8a6838);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.round_rect(-s*0.45f,-s,s*0.9f,s*2.f,2.f); ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.3f, -s * 0.35f);
            ctx.line_to(s * 0.3f, -s * 0.35f);
            ctx.move_to(-s * 0.3f, s * 0.15f);
            ctx.line_to(s * 0.3f, s * 0.15f);
            ctx.stroke();
            ctx.set_fill(0xff403020);
            ctx.begin_path(); ctx.arc(0, -s * 0.55f, s * 0.14f); ctx.fill();
            ctx.begin_path();
            ctx.move_to(-s * 0.18f, s * 0.45f);
            ctx.line_to(0, s * 0.7f);
            ctx.line_to(s * 0.18f, s * 0.45f);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kRoot: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff8a6038);
            ctx.set_stroke(0xff5a3820);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.move_to(0,-s); ctx.line_to(s*0.35f,s); ctx.line_to(0,s*0.6f); ctx.line_to(-s*0.35f,s);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kLaser: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff606070);
            ctx.set_stroke(0xff303038);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-s * 0.35f, -s * 0.35f, s * 0.7f, s * 1.25f, 2.f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffff4040);
            ctx.set_stroke(0xffaa2020);
            ctx.begin_path();
            ctx.move_to(-s * 0.18f, -s * 0.35f);
            ctx.line_to(0, -s * 1.05f);
            ctx.line_to(s * 0.18f, -s * 0.35f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffffffff);
            ctx.begin_path(); ctx.arc(0, s * 0.15f, s * 0.14f); ctx.fill();
            break;
        }
        case PetalID::kElectricWeb: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff60d0ff);
            ctx.set_stroke(0xff2080c0);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,0,s); ctx.stroke();
            for (int i = 0; i < 6; ++i) {
                float a = i * 1.0472f;
                ctx.begin_path(); ctx.move_to(0,0); ctx.line_to(cosf(a)*s,sinf(a)*s); ctx.stroke();
            }
            break;
        }
        case PetalID::kSponge: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffffd060);
            ctx.set_stroke(0xffcc9830);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s * 0.9f, -s * 0.2f);
            ctx.qcurve_to(-s * 0.3f, -s * 0.95f, s * 0.4f, -s * 0.55f);
            ctx.qcurve_to(s * 1.0f, 0, s * 0.7f, s * 0.55f);
            ctx.qcurve_to(0, s * 0.95f, -s * 0.85f, s * 0.35f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffe8b848);
            for (int i = 0; i < 5; ++i) {
                float a = i * 1.2566f;
                ctx.begin_path();
                ctx.arc(cosf(a) * s * 0.35f, sinf(a) * s * 0.28f, s * 0.12f);
                ctx.fill();
            }
            break;
        }
        case PetalID::kFang: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8e8e8);
            ctx.set_stroke(0xff909090);
            ctx.set_line_width(2.2f);
            ctx.round_line_join(); ctx.begin_path();
            ctx.move_to(s,0); ctx.line_to(-s*0.55f,s*0.75f); ctx.line_to(-s*0.55f,-s*0.75f);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kClaw: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffd0a070);
            ctx.set_stroke(0xff8a6038);
            ctx.set_line_width(2.2f);
            ctx.round_line_join(); ctx.begin_path();
            ctx.move_to(s,0); ctx.line_to(-s*0.55f,s*0.75f); ctx.line_to(-s*0.55f,-s*0.75f);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kLightBulb: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xfffff0a0);
            ctx.set_stroke(0xffd0b040);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, -s * 0.15f, s * 0.75f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffc0c0c8);
            ctx.set_stroke(0xff808088);
            ctx.begin_path();
            ctx.round_rect(-s * 0.28f, s * 0.4f, s * 0.56f, s * 0.45f, 1.5f);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xffe0c860);
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, 0);
            ctx.line_to(0, s * 0.25f);
            ctx.line_to(s * 0.12f, -s * 0.05f);
            ctx.line_to(s * 0.05f, s * 0.35f);
            ctx.stroke();
            ctx.set_fill(0x88ffffff);
            ctx.begin_path();
            ctx.arc(-s * 0.25f, -s * 0.35f, s * 0.2f);
            ctx.fill();
            break;
        }
        case PetalID::kPoo: {
            // Classic swirly drop — stacked blobs, not a flat rarity oval.
            float s = fmaxf(8.f, r);
            ctx.set_line_width(2.0f);
            ctx.round_line_cap();
            ctx.round_line_join();
            // Base puddle
            ctx.set_fill(0xff6b4420);
            ctx.set_stroke(0xff3d2810);
            ctx.begin_path();
            ctx.ellipse(0, s * 0.35f, s * 1.05f, s * 0.42f);
            ctx.fill(); ctx.stroke();
            // Mid scoop
            ctx.set_fill(0xff8a5a28);
            ctx.begin_path();
            ctx.ellipse(-s * 0.12f, s * 0.05f, s * 0.78f, s * 0.55f);
            ctx.fill(); ctx.stroke();
            // Top swirl
            ctx.set_fill(0xffa87030);
            ctx.begin_path();
            ctx.ellipse(s * 0.08f, -s * 0.35f, s * 0.55f, s * 0.48f);
            ctx.fill(); ctx.stroke();
            // Tip curl
            ctx.set_fill(0xffc09048);
            ctx.begin_path();
            ctx.arc(s * 0.18f, -s * 0.72f, s * 0.22f);
            ctx.fill();
            // Soft highlight
            ctx.set_fill(0x66ffe0a0);
            ctx.begin_path();
            ctx.ellipse(-s * 0.28f, -s * 0.15f, s * 0.22f, s * 0.14f);
            ctx.fill();
            break;
        }
        case PetalID::kAnkh: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8c040);
            ctx.set_stroke(0xffa08020);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,-s*0.45f,s*0.35f); ctx.stroke();
            ctx.begin_path(); ctx.move_to(0,-s*0.1f); ctx.line_to(0,s);
            ctx.move_to(-s*0.45f,s*0.15f); ctx.line_to(s*0.45f,s*0.15f); ctx.stroke();
            break;
        }
        case PetalID::kShovel: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff909098);
            ctx.set_stroke(0xff505058);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.round_rect(-s*0.12f,-s*0.2f,s*0.24f,s*1.3f,1.f); ctx.fill(); ctx.stroke();
            ctx.begin_path(); ctx.move_to(-s*0.45f,-s*0.2f); ctx.line_to(0,-s); ctx.line_to(s*0.45f,-s*0.2f);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kBandage: {
            // Classic adhesive bandage — reads clearly at inventory size.
            float const s = fmaxf(8.f, r);
            float const hw = s * 1.05f;  // half-width of strip
            float const hh = s * 0.52f;  // half-height
            float const pad_w = s * 0.42f;
            float const pad_h = s * 0.38f;
            ctx.round_line_cap();
            ctx.round_line_join();
            // Soft drop shadow
            ctx.set_fill(0x28000000);
            ctx.begin_path();
            ctx.round_rect(-hw + s * 0.04f, -hh + s * 0.1f, hw * 2.f, hh * 2.f, hh);
            ctx.fill();
            // Adhesive wings (warm tan)
            ctx.set_fill(0xfff0c878);
            ctx.set_stroke(0xffc89840);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.round_rect(-hw, -hh, hw * 2.f, hh * 2.f, hh);
            ctx.fill();
            ctx.stroke();
            // Top highlight on wings
            ctx.set_stroke(0x66ffffff);
            ctx.set_line_width(2.0f);
            ctx.begin_path();
            ctx.move_to(-hw + s * 0.12f, -hh * 0.55f);
            ctx.line_to(-pad_w - s * 0.05f, -hh * 0.55f);
            ctx.move_to(pad_w + s * 0.05f, -hh * 0.55f);
            ctx.line_to(hw - s * 0.12f, -hh * 0.55f);
            ctx.stroke();
            // Perforation dots on left / right adhesive
            ctx.set_fill(0xffd4a858);
            for (int side = -1; side <= 1; side += 2) {
                float bx = side * (hw * 0.62f);
                for (int row = -1; row <= 1; ++row) {
                    for (int col = -1; col <= 1; ++col) {
                        ctx.begin_path();
                        ctx.arc(bx + col * s * 0.14f, row * s * 0.18f, s * 0.055f);
                        ctx.fill();
                    }
                }
            }
            // Center gauze pad (ivory)
            ctx.set_fill(0xfffff8ee);
            ctx.set_stroke(0xffd0b888);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.round_rect(-pad_w, -pad_h, pad_w * 2.f, pad_h * 2.f, s * 0.1f);
            ctx.fill();
            ctx.stroke();
            // Weave grid on pad
            ctx.set_stroke(0xffe8dcc0);
            ctx.set_line_width(1.1f);
            ctx.begin_path();
            ctx.move_to(-pad_w * 0.55f, -pad_h * 0.7f);
            ctx.line_to(-pad_w * 0.55f, pad_h * 0.7f);
            ctx.move_to(0, -pad_h * 0.7f);
            ctx.line_to(0, pad_h * 0.7f);
            ctx.move_to(pad_w * 0.55f, -pad_h * 0.7f);
            ctx.line_to(pad_w * 0.55f, pad_h * 0.7f);
            ctx.move_to(-pad_w * 0.7f, -pad_h * 0.35f);
            ctx.line_to(pad_w * 0.7f, -pad_h * 0.35f);
            ctx.move_to(-pad_w * 0.7f, pad_h * 0.35f);
            ctx.line_to(pad_w * 0.7f, pad_h * 0.35f);
            ctx.stroke();
            // Red medical cross on pad
            float cr = s * 0.2f;
            ctx.set_fill(0xffe04848);
            ctx.set_stroke(0xffa02828);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.round_rect(-cr * 0.35f, -cr, cr * 0.7f, cr * 2.f, cr * 0.2f);
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.round_rect(-cr, -cr * 0.35f, cr * 2.f, cr * 0.7f, cr * 0.2f);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case PetalID::kWax: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffffe080);
            ctx.set_stroke(0xffccb040);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * 1.0472f; float x = cosf(a)*s, y = sinf(a)*s;
                if (i == 0) ctx.move_to(x,y); else ctx.line_to(x,y);
            }
            ctx.close_path(); ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffffffff); ctx.begin_path(); ctx.arc(0,0,s*0.28f); ctx.fill();
            break;
        }
        case PetalID::kAmulet: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8c040);
            ctx.set_stroke(0xff887020);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.arc(0,-s*0.15f,s*0.7f); ctx.fill(); ctx.stroke();
            ctx.begin_path(); ctx.move_to(0,s*0.4f); ctx.line_to(0,s); ctx.stroke();
            break;
        }
        case PetalID::kBlueberries: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff4060c0);
            ctx.set_stroke(0xff203888);
            ctx.set_line_width(2.2f);
            for (int i = 0; i < 4; ++i) {
                float a = i * 1.5708f + 0.4f;
                ctx.begin_path();
                ctx.arc(cosf(a) * s * 0.32f, sinf(a) * s * 0.32f + s * 0.08f, s * 0.38f);
                ctx.fill(); ctx.stroke();
            }
            ctx.set_stroke(0xff3a9028);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.45f);
            ctx.line_to(0, -s * 0.9f);
            ctx.stroke();
            break;
        }
        case PetalID::kFragment: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffa0a8b0);
            ctx.set_stroke(0xff606870);
            ctx.set_line_width(2.2f);
            ctx.round_line_join(); ctx.begin_path();
            ctx.move_to(s,0); ctx.line_to(-s*0.55f,s*0.75f); ctx.line_to(-s*0.55f,-s*0.75f);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kCogwheel: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff909098);
            ctx.set_stroke(0xff505058);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            for (int i = 0; i < 6; ++i) {
                float a = i * 1.0472f; float x = cosf(a)*s, y = sinf(a)*s;
                if (i == 0) ctx.move_to(x,y); else ctx.line_to(x,y);
            }
            ctx.close_path(); ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffffffff); ctx.begin_path(); ctx.arc(0,0,s*0.28f); ctx.fill();
            break;
        }
        case PetalID::kManaOrb: {
            // Crystal mana orb: outer ring, core, rune ticks, highlight.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xaa3080d8);
            ctx.set_stroke(0xff104878);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.98f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff70d8ff);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.58f);
            ctx.fill();
            ctx.set_stroke(0xffc0f0ff);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.72f);
            ctx.stroke();
            ctx.round_line_cap();
            for (int i = 0; i < 4; ++i) {
                float a = i * 1.5707963f + 0.4f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * s * 0.78f, sinf(a) * s * 0.78f);
                ctx.line_to(cosf(a) * s * 0.92f, sinf(a) * s * 0.92f);
                ctx.stroke();
            }
            ctx.set_fill(0xaaffffff);
            ctx.begin_path();
            ctx.arc(-s * 0.28f, -s * 0.32f, s * 0.22f);
            ctx.fill();
            break;
        }
        case PetalID::kMagicLeaf: {
            // Leaf silhouette + veins + mana tip glow (vs plain Leaf green).
            float s = fmaxf(8.f, r) * 0.85f;
            ctx.set_fill(0xff48e898);
            ctx.set_stroke(0xff189060);
            ctx.set_line_width(2.6f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-s * 1.4f, 0);
            ctx.line_to(-s, 0);
            ctx.bcurve_to(-s * 0.55f, -s * 0.95f, s * 0.35f, -s * 0.95f, s * 1.15f, 0);
            ctx.bcurve_to(s * 0.35f, s * 0.95f, -s * 0.55f, s * 0.95f, -s, 0);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xff90ffe0);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, 0);
            ctx.qcurve_to(0, -s * 0.12f, s * 0.7f, 0);
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, -s * 0.05f);
            ctx.qcurve_to(s * 0.15f, -s * 0.45f, s * 0.45f, -s * 0.35f);
            ctx.move_to(-s * 0.15f, s * 0.05f);
            ctx.qcurve_to(s * 0.15f, s * 0.45f, s * 0.45f, s * 0.35f);
            ctx.stroke();
            ctx.set_fill(0x88c0ffff);
            ctx.begin_path();
            ctx.arc(s * 0.95f, 0, s * 0.22f);
            ctx.fill();
            break;
        }
        case PetalID::kMagicCactus: {
            // Spiky cactus disc (like Cactus) + mana core ring.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff38d878);
            ctx.set_stroke(0xff189048);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(s, 0);
            for (uint32_t i = 0; i < 8; ++i) {
                float base = (float)M_PI * 2.f * (float)i / 8.f;
                ctx.qcurve_to(
                    s * 0.72f * cosf(base + (float)M_PI / 8.f),
                    s * 0.72f * sinf(base + (float)M_PI / 8.f),
                    s * cosf(base + 2.f * (float)M_PI / 8.f),
                    s * sinf(base + 2.f * (float)M_PI / 8.f));
            }
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff70f0b0);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.42f);
            ctx.fill();
            ctx.set_stroke(0xffa0ffff);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.55f);
            ctx.stroke();
            break;
        }
        case PetalID::kMagicCotton: {
            // Layered cotton tufts + cool mana tint (vs plain Cotton).
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8f0ff);
            ctx.set_stroke(0xff90a8e0);
            ctx.set_line_width(2.2f);
            float ox[5] = {0, -0.55f, 0.52f, -0.28f, 0.3f};
            float oy[5] = {0, 0.08f, 0.05f, -0.42f, -0.38f};
            float orr[5] = {0.62f, 0.48f, 0.46f, 0.4f, 0.38f};
            for (int i = 0; i < 5; ++i) {
                ctx.begin_path();
                ctx.arc(ox[i] * s, oy[i] * s, orr[i] * s);
                ctx.fill(); ctx.stroke();
            }
            ctx.set_stroke(0xff70a0e0);
            ctx.begin_path();
            ctx.move_to(0, s * 0.5f);
            ctx.line_to(0, s * 0.95f);
            ctx.stroke();
            ctx.set_fill(0x66a0d8ff);
            ctx.begin_path();
            ctx.arc(0, -s * 0.05f, s * 0.22f);
            ctx.fill();
            break;
        }
        case PetalID::kMagicEye: {
            // Third-eye style lid + iris + spark (mana lightning look).
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffd8ecff);
            ctx.set_stroke(0xff4870b8);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(0, -s * 0.95f);
            ctx.qcurve_to(s * 1.05f, 0, 0, s * 0.95f);
            ctx.qcurve_to(-s * 1.05f, 0, 0, -s * 0.95f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff68a0ff);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.42f);
            ctx.fill();
            ctx.set_fill(0xff101828);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.2f);
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(s * 0.12f, -s * 0.14f, s * 0.1f);
            ctx.fill();
            ctx.set_stroke(0xffa0e8ff);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(s * 0.55f, -s * 0.55f);
            ctx.line_to(s * 0.85f, -s * 0.85f);
            ctx.move_to(-s * 0.5f, s * 0.45f);
            ctx.line_to(-s * 0.78f, s * 0.72f);
            ctx.stroke();
            break;
        }
        case PetalID::kMagicStinger: {
            // Purple fragile stinger: tip + mid band + mana notch (≠ Missile).
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff9060e0);
            ctx.set_stroke(0xff5030a0);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(s * 1.15f, 0);
            ctx.line_to(-s * 0.35f, s * 0.72f);
            ctx.line_to(-s * 0.55f, 0);
            ctx.line_to(-s * 0.35f, -s * 0.72f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffc8a0ff);
            ctx.begin_path();
            ctx.move_to(s * 0.35f, 0);
            ctx.line_to(-s * 0.15f, s * 0.35f);
            ctx.line_to(-s * 0.25f, 0);
            ctx.line_to(-s * 0.15f, -s * 0.35f);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffe0c8ff);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 0.25f);
            ctx.line_to(-s * 0.9f, 0);
            ctx.line_to(-s * 0.55f, s * 0.25f);
            ctx.stroke();
            break;
        }
        case PetalID::kMagicMissile: {
            // Projectile body + fins + cyan exhaust (distinct from Stinger).
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff6088ff);
            ctx.set_stroke(0xff2848c0);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(s * 1.1f, 0);
            ctx.line_to(-s * 0.15f, -s * 0.45f);
            ctx.line_to(-s * 0.15f, s * 0.45f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff4060d0);
            ctx.begin_path();
            ctx.move_to(-s * 0.05f, -s * 0.42f);
            ctx.line_to(-s * 0.75f, -s * 0.75f);
            ctx.line_to(-s * 0.35f, -s * 0.1f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.05f, s * 0.42f);
            ctx.line_to(-s * 0.75f, s * 0.75f);
            ctx.line_to(-s * 0.35f, s * 0.1f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff80ffff);
            ctx.begin_path();
            ctx.move_to(-s * 0.2f, -s * 0.22f);
            ctx.line_to(-s * 1.05f, 0);
            ctx.line_to(-s * 0.2f, s * 0.22f);
            ctx.close_path();
            ctx.fill();
            break;
        }
        case PetalID::kMagicStick: {
            // Forked stick (like Stick) + purple mana core at tip.
            float s = fmaxf(8.f, r);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_stroke(0xff6a4820);
            ctx.set_line_width(s * 0.42f);
            ctx.begin_path();
            ctx.move_to(0, s * 0.95f);
            ctx.line_to(0, -s * 0.15f);
            ctx.line_to(s * 0.45f, -s * 0.85f);
            ctx.move_to(0, -s * 0.15f);
            ctx.line_to(-s * 0.55f, -s * 0.95f);
            ctx.stroke();
            ctx.set_stroke(0xffa87838);
            ctx.set_line_width(s * 0.22f);
            ctx.begin_path();
            ctx.move_to(0, s * 0.9f);
            ctx.line_to(0, -s * 0.12f);
            ctx.line_to(s * 0.4f, -s * 0.78f);
            ctx.move_to(0, -s * 0.12f);
            ctx.line_to(-s * 0.5f, -s * 0.88f);
            ctx.stroke();
            ctx.set_fill(0xff9060ff);
            ctx.set_stroke(0xffc8a0ff);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.arc(-s * 0.52f, -s * 0.92f, s * 0.28f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xaaffffff);
            ctx.begin_path();
            ctx.arc(-s * 0.6f, -s * 1.02f, s * 0.1f);
            ctx.fill();
            break;
        }
        case PetalID::kMagicBubble: {
            // Bubble membrane + twin highlights + violet mana tint.
            float s = fmaxf(8.f, r);
            ctx.begin_path();
            ctx.arc(0, 0, s);
            ctx.set_stroke(0xb2c0a0ff);
            ctx.set_line_width(2.8f);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0, 0, s - 1.8f);
            ctx.set_fill(0x5590b0ff);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(s * 0.32f, -s * 0.35f, s * 0.28f);
            ctx.set_fill(0x66ffffff);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(-s * 0.25f, s * 0.2f, s * 0.16f);
            ctx.set_fill(0x44e0c0ff);
            ctx.fill();
            ctx.set_stroke(0x88ffffff);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.partial_arc(0, 0, s * 0.75f, -2.2f, -0.6f, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kTalisman: {
            // Coin medallion + cord loop + engraved mark.
            float s = fmaxf(8.f, r);
            ctx.set_stroke(0xff887038);
            ctx.set_line_width(2.2f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(0, s * 0.15f);
            ctx.line_to(0, s * 1.05f);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0, -s * 0.95f, s * 0.22f);
            ctx.stroke();
            ctx.set_fill(0xffe0b848);
            ctx.set_stroke(0xff986828);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, -s * 0.1f, s * 0.72f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xfff8e090);
            ctx.begin_path();
            ctx.arc(0, -s * 0.1f, s * 0.48f);
            ctx.fill();
            ctx.set_stroke(0xff705018);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-s * 0.22f, -s * 0.28f);
            ctx.line_to(0, s * 0.15f);
            ctx.line_to(s * 0.22f, -s * 0.28f);
            ctx.stroke();
            break;
        }
        case PetalID::kNazar: {
            // Evil-eye amulet: concentric rings + pupil.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8e0d0);
            ctx.set_stroke(0xff908878);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.ellipse(0, 0, s * 1.05f, s * 0.72f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff1e6ad0);
            ctx.begin_path();
            ctx.ellipse(0, 0, s * 0.78f, s * 0.52f);
            ctx.fill();
            ctx.set_fill(0xfff0f4ff);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.38f);
            ctx.fill();
            ctx.set_fill(0xff101828);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.18f);
            ctx.fill();
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(s * 0.08f, -s * 0.1f, s * 0.07f);
            ctx.fill();
            break;
        }
        case PetalID::kCompass: {
            // Dial + N/S needles + tick marks.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xfff0e8d0);
            ctx.set_stroke(0xff706858);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, 0, s);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xff908878);
            ctx.set_line_width(1.5f);
            for (int i = 0; i < 8; ++i) {
                float a = i * 0.785398f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * s * 0.72f, sinf(a) * s * 0.72f);
                ctx.line_to(cosf(a) * s * 0.9f, sinf(a) * s * 0.9f);
                ctx.stroke();
            }
            ctx.set_fill(0xffcc2030);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.7f);
            ctx.line_to(s * 0.16f, 0.05f * s);
            ctx.line_to(-s * 0.16f, 0.05f * s);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff405080);
            ctx.begin_path();
            ctx.move_to(0, s * 0.65f);
            ctx.line_to(s * 0.14f, -0.02f * s);
            ctx.line_to(-s * 0.14f, -0.02f * s);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff303028);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.1f);
            ctx.fill();
            break;
        }
        case PetalID::kMimic: {
            // Chest body + lid seam + latch + corner studs.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff7a9860);
            ctx.set_stroke(0xff405030);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.round_rect(-s * 0.9f, -s * 0.35f, s * 1.8f, s * 1.05f, 3.f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff98b878);
            ctx.begin_path();
            ctx.round_rect(-s * 0.9f, -s * 0.85f, s * 1.8f, s * 0.55f, 3.f);
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-s * 0.9f, -s * 0.32f);
            ctx.line_to(s * 0.9f, -s * 0.32f);
            ctx.stroke();
            ctx.set_fill(0xffd0b040);
            ctx.begin_path();
            ctx.round_rect(-s * 0.18f, -s * 0.48f, s * 0.36f, s * 0.32f, 1.5f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffc8a858);
            for (int i = 0; i < 4; ++i) {
                float x = (i < 2 ? -1.f : 1.f) * s * 0.72f;
                float y = (i % 2 == 0 ? -0.7f : 0.45f) * s;
                ctx.begin_path();
                ctx.arc(x, y, s * 0.1f);
                ctx.fill();
            }
            break;
        }
        case PetalID::kPowder: {
            // Pouch + spilling dust clouds.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc8a060);
            ctx.set_stroke(0xff806038);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 0.15f);
            ctx.line_to(-s * 0.7f, s * 0.75f);
            ctx.qcurve_to(0, s * 1.05f, s * 0.7f, s * 0.75f);
            ctx.line_to(s * 0.55f, -s * 0.15f);
            ctx.qcurve_to(0, -s * 0.45f, -s * 0.55f, -s * 0.15f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffe8d090);
            ctx.begin_path();
            ctx.ellipse(0, -s * 0.35f, s * 0.55f, s * 0.22f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xffe0d0a0);
            ctx.set_stroke(0xffa09060);
            ctx.begin_path(); ctx.arc(-s * 0.75f, -s * 0.55f, s * 0.28f); ctx.fill(); ctx.stroke();
            ctx.begin_path(); ctx.arc(-s * 0.95f, -s * 0.25f, s * 0.2f); ctx.fill();
            ctx.begin_path(); ctx.arc(s * 0.7f, -s * 0.5f, s * 0.24f); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kRelic: {
            // Stone tablet + glyph + chip highlight.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffc8b070);
            ctx.set_stroke(0xff806838);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 1.05f);
            ctx.line_to(s * 0.5f, -s * 1.0f);
            ctx.line_to(s * 0.6f, s * 1.0f);
            ctx.line_to(-s * 0.5f, s * 1.05f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xff605028);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.move_to(-s * 0.15f, -s * 0.55f);
            ctx.line_to(s * 0.2f, -s * 0.55f);
            ctx.move_to(0, -s * 0.55f);
            ctx.line_to(0, s * 0.35f);
            ctx.move_to(-s * 0.25f, s * 0.05f);
            ctx.line_to(s * 0.28f, s * 0.05f);
            ctx.stroke();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.arc(s * 0.2f, -s * 0.7f, s * 0.16f);
            ctx.fill();
            break;
        }
        case PetalID::kDarkMark: {
            // Dark pendant disc + crescent + chain.
            float s = fmaxf(8.f, r);
            ctx.set_stroke(0xff302038);
            ctx.set_line_width(2.2f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(0, s * 0.2f);
            ctx.line_to(0, s * 1.05f);
            ctx.stroke();
            ctx.set_fill(0xff302440);
            ctx.set_stroke(0xff181020);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.arc(0, -s * 0.15f, s * 0.78f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff100818);
            ctx.begin_path();
            ctx.arc(s * 0.12f, -s * 0.2f, s * 0.55f);
            ctx.fill();
            ctx.set_fill(0xff8060a0);
            ctx.begin_path();
            ctx.arc(-s * 0.2f, -s * 0.35f, s * 0.18f);
            ctx.fill();
            break;
        }
        case PetalID::kChampionCrown: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffffd040);
            ctx.set_stroke(0xffc09020);
            ctx.set_line_width(2.2f);
            ctx.begin_path(); ctx.move_to(-s,s*0.3f); ctx.line_to(-s*0.6f,-s*0.6f); ctx.line_to(-s*0.2f,0);
            ctx.line_to(0,-s); ctx.line_to(s*0.2f,0); ctx.line_to(s*0.6f,-s*0.6f); ctx.line_to(s,s*0.3f);
            ctx.close_path(); ctx.fill(); ctx.stroke();
            break;
        }
        case PetalID::kSoulSplitter: {
            // Twin gem halves with seam glow (not a flat box).
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffb090ff);
            ctx.set_stroke(0xff6840b0);
            ctx.set_line_width(2.4f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-s * 0.95f, 0);
            ctx.line_to(-s * 0.15f, -s * 0.75f);
            ctx.line_to(-s * 0.15f, s * 0.75f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.move_to(s * 0.95f, 0);
            ctx.line_to(s * 0.15f, -s * 0.75f);
            ctx.line_to(s * 0.15f, s * 0.75f);
            ctx.close_path();
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xffe8d0ff);
            ctx.set_line_width(2.f);
            ctx.begin_path();
            ctx.move_to(0, -s * 0.85f);
            ctx.line_to(0, s * 0.85f);
            ctx.stroke();
            ctx.set_fill(0x88ffffff);
            ctx.begin_path();
            ctx.arc(-s * 0.45f, -s * 0.15f, s * 0.14f);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(s * 0.45f, -s * 0.15f, s * 0.14f);
            ctx.fill();
            break;
        }
        case PetalID::kGoggles: {
            // Dual lenses + bridge + strap stubs.
            float s = fmaxf(8.f, r);
            ctx.set_stroke(0xff202028);
            ctx.set_line_width(2.4f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-s * 1.05f, -s * 0.15f);
            ctx.line_to(-s * 1.35f, -s * 0.45f);
            ctx.move_to(s * 1.05f, -s * 0.15f);
            ctx.line_to(s * 1.35f, -s * 0.45f);
            ctx.stroke();
            ctx.set_fill(0xff505868);
            ctx.begin_path();
            ctx.ellipse(-s * 0.48f, 0, s * 0.5f, s * 0.42f);
            ctx.fill(); ctx.stroke();
            ctx.begin_path();
            ctx.ellipse(s * 0.48f, 0, s * 0.5f, s * 0.42f);
            ctx.fill(); ctx.stroke();
            ctx.set_fill(0xff88d0ff);
            ctx.begin_path();
            ctx.ellipse(-s * 0.48f, 0, s * 0.28f, s * 0.24f);
            ctx.fill();
            ctx.begin_path();
            ctx.ellipse(s * 0.48f, 0, s * 0.28f, s * 0.24f);
            ctx.fill();
            ctx.set_stroke(0xff303038);
            ctx.set_line_width(3.f);
            ctx.begin_path();
            ctx.move_to(-s * 0.05f, 0);
            ctx.line_to(s * 0.05f, 0);
            ctx.stroke();
            break;
        }
        case PetalID::kSalve: {
            // Soft green ointment jar with a white dab on top.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xff6ecf7a);
            ctx.set_stroke(0xff3a8f48);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            ctx.round_rect(-s * 0.72f, -s * 0.35f, s * 1.44f, s * 1.05f, s * 0.28f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xfff5fff6);
            ctx.set_stroke(0xffc8e8cc);
            ctx.set_line_width(1.8f);
            ctx.begin_path();
            ctx.ellipse(0, -s * 0.55f, s * 0.55f, s * 0.32f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.ellipse(-s * 0.22f, -s * 0.05f, s * 0.2f, s * 0.35f);
            ctx.fill();
            break;
        }
        case PetalID::kCrater: {
            // Top-down rocky crater ring — sibling to Meteor art, flatter rim.
            float s = fmaxf(8.f, r);
            ctx.set_fill(0x30000000);
            ctx.begin_path();
            ctx.ellipse(0.4f, 1.2f, s * 1.05f, s * 0.92f);
            ctx.fill();
            ctx.set_fill(0xff5a5568);
            ctx.set_stroke(0xff2e2a38);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.arc(0, 0, s);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1a1820);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.42f);
            ctx.fill();
            ctx.set_stroke(0xff8a8498);
            ctx.set_line_width(1.6f);
            for (int i = 0; i < 5; ++i) {
                float a = i * 1.2566f - 0.3f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * s * 0.5f, sinf(a) * s * 0.5f);
                ctx.line_to(cosf(a) * s * 0.92f, sinf(a) * s * 0.92f);
                ctx.stroke();
            }
            break;
        }
        case PetalID::kHourglass: {
            // Amber glass hourglass with sand waist.
            float s = fmaxf(8.f, r);
            ctx.set_stroke(0xff8a6230);
            ctx.set_line_width(2.2f);
            ctx.set_fill(0xfff0c878);
            ctx.begin_path();
            ctx.move_to(-s * 0.55f, -s * 0.95f);
            ctx.line_to(s * 0.55f, -s * 0.95f);
            ctx.line_to(s * 0.12f, 0);
            ctx.line_to(s * 0.55f, s * 0.95f);
            ctx.line_to(-s * 0.55f, s * 0.95f);
            ctx.line_to(-s * 0.12f, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffd4a040);
            ctx.begin_path();
            ctx.move_to(-s * 0.35f, s * 0.35f);
            ctx.line_to(s * 0.35f, s * 0.35f);
            ctx.line_to(s * 0.45f, s * 0.85f);
            ctx.line_to(-s * 0.45f, s * 0.85f);
            ctx.close_path();
            ctx.fill();
            ctx.set_stroke(0xffc8a060);
            ctx.set_line_width(1.6f);
            ctx.begin_path();
            ctx.move_to(-s * 0.6f, -s * 0.95f);
            ctx.line_to(s * 0.6f, -s * 0.95f);
            ctx.move_to(-s * 0.6f, s * 0.95f);
            ctx.line_to(s * 0.6f, s * 0.95f);
            ctx.stroke();
            break;
        }

        case PetalID::kShell: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0xffe8d0a8);
            ctx.set_stroke(0xffb09060);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(s * 0.15f, s * 0.7f);
            ctx.qcurve_to(s * 1.05f, 0, s * 0.1f, -s * 0.85f);
            ctx.qcurve_to(-s * 0.95f, -s * 0.2f, -s * 0.55f, s * 0.55f);
            ctx.qcurve_to(-s * 0.1f, s * 0.95f, s * 0.15f, s * 0.7f);
            ctx.fill(); ctx.stroke();
            ctx.set_stroke(0xffc8a878);
            ctx.set_line_width(1.6f);
            for (int i = 0; i < 4; ++i) {
                float t = 0.2f + i * 0.18f;
                ctx.begin_path();
                ctx.move_to(s * (0.05f + t * 0.2f), -s * (0.7f - t * 0.3f));
                ctx.qcurve_to(s * (0.55f - t * 0.2f), s * (t - 0.3f), -s * (0.2f + t * 0.15f), s * (0.35f + t * 0.1f));
                ctx.stroke();
            }
            break;
        }
        case PetalID::kGlass: {
            float s = fmaxf(8.f, r);
            ctx.set_fill(0x88c0e8ff);
            ctx.set_stroke(0xff60a0c8);
            ctx.set_line_width(2.2f);
            ctx.begin_path();
            ctx.move_to(0, -s);
            ctx.line_to(s * 0.7f, s * 0.2f);
            ctx.line_to(0, s);
            ctx.line_to(-s * 0.7f, s * 0.2f);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
        default: {
            // Normal petal shape tinted by rarity (materials / new craft results).
            assert(id < PetalID::kNumPetals);
            float s = fmaxf(8.f, r);
            uint32_t fill = RARITY_COLORS[PETAL_DATA[id].rarity % RarityID::kNumRarities];
            uint32_t rim = Renderer::HSV(fill, 0.75f);
            ctx.set_fill(fill);
            ctx.set_stroke(rim);
            ctx.set_line_width(2.4f);
            ctx.begin_path();
            // Soft oval petal like Basic, not a flat chip
            ctx.ellipse(0, 0, s * 0.95f, s * 1.15f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x66ffffff);
            ctx.begin_path();
            ctx.ellipse(-s * 0.22f, -s * 0.35f, s * 0.32f, s * 0.45f);
            ctx.fill();
            // Inner core so mats read as petals, not UI tiles
            ctx.set_fill(0xffffffff);
            ctx.begin_path();
            ctx.arc(0, 0, s * 0.28f);
            ctx.fill();
            break;
        }
    }
}

void draw_static_petal(PetalID::T id, Renderer &ctx) {
    struct PetalData const &data = PETAL_DATA[id];
    uint32_t count = data.count > 1 ? data.count : 1;
    for (uint32_t i = 0; i < count; ++i) {
        RenderContext context(&ctx);
        float rad = 10;
        if (data.attributes.clump_radius != 0)
            rad = data.attributes.clump_radius;
        ctx.rotate(i * 2 * M_PI / count);
        if (count > 1) ctx.translate(rad, 0);
        ctx.rotate(data.attributes.icon_angle);
        draw_static_petal_single(id, ctx);
    }
}

void draw_loadout_background(Renderer &ctx, PetalID::T id, float reload, uint8_t draw_name, float health) {
    RenderContext c(&ctx);
    ctx.set_fill(Renderer::HSV(RARITY_COLORS[PETAL_DATA[id].rarity], 0.8));
    ctx.round_line_join();
    ctx.round_line_cap();
    ctx.begin_path();
    ctx.round_rect(-30, -30, 60, 60, 3);
    ctx.fill();
    ctx.set_fill(RARITY_COLORS[PETAL_DATA[id].rarity]);
    ctx.begin_path();
    ctx.rect(-25, -25, 50, 50);
    ctx.fill();
    ctx.clip();
    // Live durability: missing HP washes from the top (remaining fills upward).
    if (health < 0.995f) {
        float missing = 1.f - health;
        if (missing < 0.f) missing = 0.f;
        if (missing > 1.f) missing = 1.f;
        float h = 50.f * missing;
        uint8_t const rar = PETAL_DATA[id].rarity % RarityID::kNumRarities;
        uint32_t const base = RARITY_COLORS[rar];
        // Darken toward black while keeping hue, then opaque enough to read.
        uint32_t wash_col = Renderer::MIX(Renderer::HSV(base, 0.65f), 0xff000000, 0.42f);
        wash_col = (wash_col & 0x00ffffffu) | 0xb8000000u;
        RenderContext wash(&ctx);
        ctx.set_fill(wash_col);
        ctx.begin_path();
        ctx.rect(-25, -25, 50, h);
        ctx.fill();
    }
    if (reload < 1) {
        float rld =  1 - (float) reload;
        {
            rld = rld * rld * rld * (rld * (6.0f * rld - 15.0f) + 10.0f);
            RenderContext context(&ctx);
            ctx.set_fill(0x40000000);
            ctx.begin_path();
            ctx.move_to(0,0);
            ctx.partial_arc(0, 0, 90, -M_PI / 2 - rld * M_PI * 10, -M_PI / 2 - rld * M_PI * 8, 0);
            ctx.fill();
        }
    }
    ctx.translate(0, -5);
    {
        RenderContext r(&ctx);
        ctx.scale(0.833);
        if (PETAL_DATA[id].radius > 20) ctx.scale(20 / PETAL_DATA[id].radius);
        draw_static_petal(id, ctx);
    }
    if (!draw_name) return;
    char const *pname = I18n::petal_name(id);
    float text_width = 12 * Renderer::get_ascii_text_size(pname);
    if (text_width < 50) text_width = 12;
    else text_width = 12 * 50 / text_width;
    ctx.translate(0, 20);
    ctx.draw_text(pname, { .size = text_width });
}
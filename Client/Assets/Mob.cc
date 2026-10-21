#include <Client/Assets/Assets.hh>

#include <Client/StaticData.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <cmath>

#define SET_BASE_COLOR(set_color) { if (!BitMath::at(flags, 0)) base_color = set_color; else { base_color = FLOWER_COLORS[attr.color]; } }

void draw_static_mob(MobID::T mob_id, Renderer &ctx, MobRenderAttributes attr) {
    float radius = attr.radius;
    uint32_t flags = attr.flags;
    float animation_value = sinf(attr.animation);
    uint32_t seed = attr.seed;
    uint32_t base_color = 0xffffe763;
    switch(mob_id) {
        case MobID::kBabyAnt:
            SET_BASE_COLOR(0xff555555)
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(0, -7);
            ctx.qcurve_to(11, -10 + animation_value, 22, -5 + animation_value);
            ctx.move_to(0, 7);
            ctx.qcurve_to(11, 10 - animation_value, 22, 5 - animation_value);
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.arc(0,0,radius);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kWorkerAnt:
            SET_BASE_COLOR(0xff555555)
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.begin_path();
            ctx.arc(-12, 0, 10);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff292929);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(4, -7);
            ctx.qcurve_to(15, -10 + animation_value, 26, -5 + animation_value);
            ctx.move_to(4, 7);
            ctx.qcurve_to(15, 10 - animation_value, 26, 5 - animation_value);
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.arc(4,0,radius);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kSoldierAnt:
            SET_BASE_COLOR(0xff555555)
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.begin_path();
            ctx.arc(-12, 0, 10);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x80eeeeee);
            {
                RenderContext context(&ctx);
                ctx.begin_path();
                ctx.rotate(0.1 * animation_value);
                ctx.translate(-11, -8);
                ctx.rotate(0.1 * M_PI);
                ctx.ellipse(0,0,15,7);
                ctx.fill();
            }
            {
                RenderContext context(&ctx);
                ctx.begin_path();
                ctx.rotate(-0.1 * animation_value);
                ctx.translate(-11, 8);
                ctx.rotate(-0.1 * M_PI);
                ctx.ellipse(0,0,15,7);
                ctx.fill();
            }
            ctx.set_stroke(0xff292929);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(4, -7);
            ctx.qcurve_to(15, -10 + animation_value, 26, -5 + animation_value);
            ctx.move_to(4, 7);
            ctx.qcurve_to(15, 10 - animation_value, 26, 5 - animation_value);
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.arc(4,0,radius);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kBee:
            SET_BASE_COLOR(0xffffe763)
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-25,9);
            ctx.line_to(-37,0);
            ctx.line_to(-25,-9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.begin_path();
            ctx.ellipse(0,0,30,20);
            ctx.fill();
            {
                RenderContext context(&ctx);
                ctx.clip();
                ctx.set_fill(0xff333333);
                ctx.fill_rect(-30,-20,10,40);
                ctx.fill_rect(-10,-20,10,40);
                ctx.fill_rect(10,-20,10,40);
            }
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(0,0,30,20);
            ctx.stroke();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(25,-5);
            ctx.qcurve_to(35,-5,40,-15);
            ctx.stroke();
            ctx.set_fill(0xff333333);
            ctx.begin_path();
            ctx.arc(40,-15,5);
            ctx.fill();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(25,5);
            ctx.qcurve_to(35,5,40,15);
            ctx.stroke();
            ctx.set_fill(0xff333333);
            ctx.begin_path();
            ctx.arc(40,15,5);
            ctx.fill();
            break;
        case MobID::kLadybug:
        case MobID::kMassiveLadybug:
        case MobID::kDarkLadybug:
        case MobID::kShinyLadybug:
            ctx.scale(radius / 30);
            if (mob_id == MobID::kDarkLadybug) SET_BASE_COLOR(0xff962921)
            else if (mob_id == MobID::kShinyLadybug) SET_BASE_COLOR(0xffebeb34)
            else SET_BASE_COLOR(0xffeb4034)
            ctx.set_fill(0xff111111);
            ctx.begin_path();
            ctx.arc(15,0,18.5);
            ctx.fill();
            ctx.set_fill(base_color);
            ctx.begin_path();
            ctx.move_to(24.760068893432617,16.939273834228516);
            ctx.qcurve_to(17.74359130859375,27.195226669311523,5.530136585235596,29.485883712768555);
            ctx.qcurve_to(-6.683317184448242,31.77654457092285,-16.939273834228516,24.760068893432617);
            ctx.qcurve_to(-27.195226669311523,17.74359130859375,-29.485883712768555,5.530136585235596);
            ctx.qcurve_to(-31.77654457092285,-6.683317184448242,-24.760068893432617,-16.939273834228516);
            ctx.qcurve_to(-17.74359130859375,-27.195226669311523,-5.530136585235596,-29.485883712768555);
            ctx.qcurve_to(6.683317184448242,-31.77654457092285,16.939273834228516,-24.760068893432617);
            ctx.qcurve_to(19.241104125976562,-23.185302734375,21.213199615478516,-21.213205337524414);
            ctx.qcurve_to(23.1852970123291,-19.241111755371094,24.76006507873535,-16.939281463623047);
            ctx.qcurve_to(10,0,24.760068893432617,16.939273834228516);
            ctx.fill(1);
            {
                RenderContext context(&ctx);
                ctx.clip();
                if (mob_id == MobID::kDarkLadybug) ctx.set_fill(Renderer::HSV(base_color, 1.2));
                else ctx.set_fill(0xff111111);
                SeedGenerator gen(seed * 374572 + 46237);
                uint32_t ct = 1 + gen.next() * 7;
                for (uint32_t i = 0; i < ct; ++i) {
                    ctx.begin_path();
                    ctx.arc(gen.binext()*30,gen.binext()*30,4+gen.next()*5);
                    ctx.fill();
                }
            }
            ctx.set_fill(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.move_to(27.64874267578125,18.915523529052734);
            ctx.qcurve_to(19.813682556152344,30.36800765991211,6.175320625305176,32.925907135009766);
            ctx.qcurve_to(-7.463029861450195,35.48381042480469,-18.91551971435547,27.648746490478516);
            ctx.qcurve_to(-30.36800765991211,19.813682556152344,-32.925907135009766,6.175320625305176);
            ctx.qcurve_to(-35.48381042480469,-7.463029861450195,-27.648746490478516,-18.91551971435547);
            ctx.qcurve_to(-19.813682556152344,-30.36800765991211,-6.175320625305176,-32.925907135009766);
            ctx.qcurve_to(7.463029861450195,-35.48381042480469,18.91551971435547,-27.648746490478516);
            ctx.qcurve_to(24.10110092163086,-24.101102828979492,27.648740768432617,-18.915529251098633);
            ctx.qcurve_to(28.323867797851562,-17.928699493408203,28.25410270690918,-16.73506736755371);
            ctx.qcurve_to(28.184337615966797,-15.541434288024902,27.398849487304688,-14.639973640441895);
            ctx.qcurve_to(14.642288208007812,0,27.398853302001953,14.639965057373047);
            ctx.qcurve_to(28.184343338012695,15.541427612304688,28.254106521606445,16.735061645507812);
            ctx.qcurve_to(28.323869705200195,17.928693771362305,27.64874267578125,18.9155216217041);
            ctx.line_to(27.64874267578125,18.915523529052734);
            ctx.move_to(21.871395111083984,14.963025093078613);
            ctx.line_to(24.760068893432617,16.939273834228516);
            ctx.line_to(22.12128448486328,19.238582611083984);
            ctx.qcurve_to(5.3577117919921875,0,22.121280670166016,-19.238590240478516);
            ctx.line_to(24.76006507873535,-16.939281463623047);
            ctx.line_to(21.871389389038086,-14.963033676147461);
            ctx.qcurve_to(19.065046310424805,-19.0650577545166,14.96302318572998,-21.871395111083984);
            ctx.qcurve_to(5.903592586517334,-28.06928253173828,-4.884955406188965,-26.045866012573242);
            ctx.qcurve_to(-15.673511505126953,-24.022449493408203,-21.871395111083984,-14.96302318572998);
            ctx.qcurve_to(-28.06928253173828,-5.903592586517334,-26.045866012573242,4.884955406188965);
            ctx.qcurve_to(-24.022449493408203,15.673511505126953,-14.96302318572998,21.871395111083984);
            ctx.qcurve_to(-5.903592586517334,28.06928253173828,4.884955406188965,26.045866012573242);
            ctx.qcurve_to(15.673511505126953,24.022449493408203,21.871395111083984,14.963025093078613);
            ctx.fill(1);
            break;
        case MobID::kBeetle:
        case MobID::kMassiveBeetle:
        case MobID::kFlowerGuardian:
            ctx.scale(radius / 35);
            SET_BASE_COLOR(0xff905db0)
            ctx.begin_path();
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.translate(35,0);
            {
                RenderContext context(&ctx);
                ctx.rotate(-0.1 * animation_value);
                ctx.move_to(-10,15);
                ctx.qcurve_to(15,30,35,15);
                ctx.qcurve_to(15,20,-10,5);
                ctx.line_to(-10,15);
                ctx.fill();
                ctx.stroke();
            }
            {
                RenderContext context(&ctx);
                ctx.rotate(0.1 * animation_value);
                ctx.move_to(-10,-15);
                ctx.qcurve_to(15,-30,35,-15);
                ctx.qcurve_to(15,-20,-10,-5);
                ctx.line_to(-10,-15);
                ctx.fill();
                ctx.stroke();
            }
            ctx.translate(-35,0);
            ctx.begin_path();
            ctx.move_to(0,-30);
            ctx.qcurve_to(40,-30,40,0);
            ctx.qcurve_to(40,30,0,30);
            ctx.qcurve_to(-40,30,-40,0);
            ctx.qcurve_to(-40,-30,0,-30);
            ctx.set_fill(base_color);
            ctx.fill();
            ctx.begin_path();
            ctx.move_to(0,-33.5);
            ctx.qcurve_to(43.5,-33.5,43.5,0);
            ctx.qcurve_to(43.5,33.5,0,33.5);
            ctx.qcurve_to(-43.5,33.5,-43.5,0);
            ctx.qcurve_to(-43.5,-33.5,0,-33.5);
            ctx.move_to(0,-26.5);
            ctx.qcurve_to(-36.5,-26.5,-36.5,0);
            ctx.qcurve_to(-36.5,26.5,0,26.5);
            ctx.qcurve_to(36.5,26.5,36.5,0);
            ctx.qcurve_to(36.5,-26.5,0,-26.5);
            ctx.set_fill(Renderer::HSV(base_color, 0.8));
            ctx.fill();
            ctx.begin_path();
            ctx.move_to(-20,0);
            ctx.qcurve_to(0,-3,20,0);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.stroke();
            ctx.set_fill(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.move_to(-17,-12);
            ctx.arc(-17,-12,5);
            ctx.move_to(-17,-2);
            ctx.arc(-17,12,5);
            ctx.move_to(0,-15);
            ctx.arc(0,-15,5);
            ctx.move_to(0,15);
            ctx.arc(0,15,5);
            ctx.move_to(17,-12);
            ctx.arc(17,-12,5);
            ctx.move_to(17,12);
            ctx.arc(17,12,5);
            ctx.fill();
            break;
        case MobID::kHornet:
            SET_BASE_COLOR(0xffffe763)
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(-25,-6);
            ctx.line_to(-47,0);
            ctx.line_to(-25,6);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.begin_path();
            ctx.ellipse(0,0,30,20);
            ctx.fill();
            {
                RenderContext context(&ctx);
                ctx.clip();
                ctx.set_fill(0xff333333);
                ctx.fill_rect(-30,-20,10,40);
                ctx.fill_rect(-10,-20,10,40);
                ctx.fill_rect(10,-20,10,40);
            }
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(0,0,30,20);
            ctx.stroke();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(25, 5);
            ctx.qcurve_to(40, 10, 50, 15);
            ctx.qcurve_to(40, 5, 25, 5);
            ctx.move_to(25, -5);
            ctx.qcurve_to(40, -10, 50, -15);
            ctx.qcurve_to(40, -5, 25, -5);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kCactus: {
            SET_BASE_COLOR(0xff32a852)
            uint32_t vertices = radius / 10 + 5;
            {
                RenderContext context(&ctx);
                ctx.set_fill(0xff222222);
                ctx.begin_path();
                for (uint32_t i = 0; i < vertices; ++i) {
                    ctx.move_to(10+radius,0);
                    ctx.line_to(0.5+radius,3);
                    ctx.line_to(0.5+radius,-3);
                    ctx.line_to(10+radius,0);
                    ctx.rotate(M_PI * 2 / vertices);
                }
                ctx.fill();
            }
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(radius,0);
            for (uint32_t i = 0; i < vertices; ++i) {
                float base_angle = M_PI * 2 * i / vertices;
                ctx.qcurve_to(
                    radius*0.8*cosf(base_angle+M_PI/vertices),radius*0.8*sinf(base_angle+M_PI/vertices),
                    radius*cosf(base_angle+2*M_PI/vertices),radius*sinf(base_angle+2*M_PI/vertices));
            }
            ctx.fill();
            ctx.stroke();
            break;
        }
        case MobID::kRock:
        case MobID::kBoulder: {
            SET_BASE_COLOR(0xff777777)
            SeedGenerator gen(std::floor(radius) * 1957264 + 295726);
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            float deflection = radius * 0.1;
            ctx.move_to(radius + gen.binext() * deflection,gen.binext() * deflection);
            uint32_t sides = 4 + radius / 10;
            for (uint32_t i = 1; i < sides; ++i) {
                float angle = 2 * M_PI * i / sides;
                ctx.line_to(cosf(angle) * radius + gen.binext() * deflection, sinf(angle) * radius + gen.binext() * deflection);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
        case MobID::kCentipede:
        case MobID::kEvilCentipede:
        case MobID::kDesertCentipede:
            if (mob_id == MobID::kCentipede) SET_BASE_COLOR(0xff8ac255)
            else if (mob_id == MobID::kEvilCentipede) SET_BASE_COLOR(0xff905db0)
            else SET_BASE_COLOR(0xffd4c66e)
            ctx.set_fill(0xff333333);
            ctx.begin_path();
            ctx.arc(0,-30,15);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,30,15);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,0,35);
            ctx.set_fill(base_color);
            ctx.fill();
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(7);
            ctx.stroke();
            if (!BitMath::at(flags, 1)) {
                ctx.begin_path();
                ctx.move_to(25,-10);
                ctx.qcurve_to(45,-10,55,-30);
                ctx.set_stroke(0xff333333);
                ctx.set_line_width(3);
                ctx.stroke();
                ctx.begin_path();
                ctx.arc(55,-30,5);
                ctx.set_fill(0xff333333);
                ctx.fill();
                ctx.begin_path();
                ctx.move_to(25,10);
                ctx.qcurve_to(45,10,55,30);
                ctx.stroke();
                ctx.begin_path();
                ctx.arc(55,30,5);
                ctx.fill();
            }
            break;
        case MobID::kSpider:
            SET_BASE_COLOR(0xff4f412e);
            ctx.set_fill(base_color);
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            #define draw_leg(angle) \
            { \
                float cos = cosf(angle) * 35; \
                float sin = sinf(angle) * 35; \
                ctx.move_to(0,0); \
                ctx.qcurve_to(sin * 0.8, cos * 0.5, sin, cos); \
            }
            draw_leg(-M_PI + 0.9 + sinf(attr.animation) * 0.2)
            draw_leg(-M_PI + 0.3 + cosf(attr.animation) * 0.2)
            draw_leg(-M_PI - 0.3 + sinf(attr.animation) * 0.2)
            draw_leg(-M_PI - 0.9 - cosf(attr.animation) * 0.2)
            draw_leg(-0.9 - sinf(attr.animation) * 0.2)
            draw_leg(-0.3 + cosf(attr.animation) * 0.2)
            draw_leg(0.3 - sinf(attr.animation) * 0.2)
            draw_leg(0.9 - cosf(attr.animation) * 0.2)
            #undef draw_leg
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(0,0,radius);
            ctx.fill();
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.set_line_width(5);
            ctx.stroke();
            break;
        case MobID::kSandstorm:
            SET_BASE_COLOR(0xffd5c7a6)
            ctx.set_line_width(radius / 5);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.set_fill(base_color);
            ctx.set_stroke(base_color);
            ctx.rotate(attr.animation / 3);
            ctx.begin_path();
            ctx.move_to(radius, 0);
            for (uint32_t i = 1; i <= 6; ++i) {
                float angle = 2 * M_PI * i / 6;
                ctx.line_to(cosf(angle) * radius, sinf(angle) * radius);
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(Renderer::HSV(base_color, 0.9));
            ctx.set_stroke(Renderer::HSV(base_color, 0.9));
            ctx.rotate(attr.animation / 3);
            ctx.begin_path();
            ctx.move_to(radius*2/3, 0);
            for (uint32_t i = 1; i <= 6; ++i) {
                float angle = 2 * M_PI * i / 6;
                ctx.line_to(cosf(angle) * radius*2/3, sinf(angle) * radius*2/3);
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(Renderer::HSV(base_color, 0.8));
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.rotate(attr.animation / 3);
            ctx.begin_path();
            ctx.move_to(radius/3, 0);
            for (uint32_t i = 1; i <= 6; ++i) {
                float angle = 2 * M_PI * i / 6;
                ctx.line_to(cosf(angle) * radius/3, sinf(angle) * radius/3);
            }
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kScorpion:
            ctx.scale(radius / 35);
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.round_line_join();
            {
                RenderContext context(&ctx);
                ctx.rotate(-0.05 * animation_value);
                ctx.begin_path();
                ctx.move_to(5,10.5);
                ctx.qcurve_to(30,21.5,50,10.5);
                ctx.qcurve_to(30,14,5,3.5);
                ctx.close_path();
            }
            {
                RenderContext context(&ctx);
                ctx.rotate(0.05 * animation_value);
                ctx.move_to(5,-10.5);
                ctx.qcurve_to(30,-21.5,50,-10.5);
                ctx.qcurve_to(30,-14,5,-3.5);
                ctx.close_path();
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            #define draw_leg(angle) \
            { \
                float cos = cosf(angle) * 37; \
                float sin = sinf(angle) * 37; \
                ctx.move_to(0,0); \
                ctx.qcurve_to(sin * 0.7, cos * 0.5, sin, cos); \
            }
            draw_leg(-M_PI + 0.7 + sinf(attr.animation) * 0.15)
            draw_leg(-M_PI + 0.233 + cosf(attr.animation) * 0.15)
            draw_leg(-M_PI - 0.233 + sinf(attr.animation) * 0.15)
            draw_leg(-M_PI - 0.7 - cosf(attr.animation) * 0.15)
            draw_leg(-0.7 - sinf(attr.animation) * 0.15)
            draw_leg(-0.233 + cosf(attr.animation) * 0.15)
            draw_leg(0.233 - sinf(attr.animation) * 0.15)
            draw_leg(0.7 - cosf(attr.animation) * 0.15)
            ctx.stroke();
            SET_BASE_COLOR(0xffc69a2d);
            ctx.set_fill(base_color);
            ctx.set_stroke(Renderer::HSV(base_color, 0.8));
            ctx.begin_path();
            ctx.move_to(0,-30);
            ctx.qcurve_to(40,-20,40,0);
            ctx.qcurve_to(40,20,0,30);
            ctx.qcurve_to(-40,35,-40,0);
            ctx.qcurve_to(-40,-35,0,-30);
            ctx.fill();
            ctx.stroke();
            ctx.set_line_width(7);
            ctx.begin_path();
            ctx.move_to(22,-12);
            ctx.qcurve_to(26,0,22,12);
            ctx.move_to(7,-18);
            ctx.qcurve_to(10.5,0,7,18);
            ctx.move_to(-7,-18);
            ctx.qcurve_to(-10.5,0,-7,18);
            ctx.move_to(-22,-15);
            ctx.qcurve_to(-27,0,-22,15);
            ctx.stroke();
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(-45, 0);
            ctx.bcurve_to(-44.9098, 9.5, -41.6136, 14.25, -32.4196, 14.2);
            ctx.bcurve_to(-23.2258, 14.15, -12.0197, 9, -8.2491, 0);
            ctx.bcurve_to(-12.0197, -9, -23.2258, -14.15, -32.4196, -14.2);
            ctx.bcurve_to(-41.6136, -14.25, -44.9098, -9.5, -45, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.begin_path();
            ctx.move_to(-36,-5);
            ctx.qcurve_to(-37,0,-36,5);
            ctx.move_to(-25,5);
            ctx.qcurve_to(-27,0,-25,-5);
            ctx.stroke();
            ctx.set_fill(0xff333333);
            ctx.set_stroke(0xff222222);
            ctx.begin_path();
            ctx.move_to(-5.7491, 0);
            ctx.line_to(-12.7491, -7);
            ctx.line_to(-12.7491, 7);
            ctx.line_to(-5.7491, 0);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kAntHole:
            SET_BASE_COLOR(0xffb58500);
            ctx.begin_path();
            ctx.arc(0,0,radius);
            ctx.set_fill(base_color);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,0,radius*2/3);
            ctx.set_fill(Renderer::HSV(base_color, 0.8));
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,0,radius/3);
            ctx.set_fill(Renderer::HSV(base_color, 0.6));
            ctx.fill();
            break;
        case MobID::kQueenAnt:
            ctx.begin_path();
            ctx.arc(-25,0,33.5);
            ctx.set_fill(0xff454545);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(-25,0,26.5);
            ctx.set_fill(0xff555555);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,0,28.5);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(0,0,21.5);
            ctx.fill();
            {
                RenderContext context(&ctx);
                ctx.rotate(animation_value * 0.1);
                ctx.begin_path();
                ctx.ellipse(-14,-16,30,14,M_PI/10);
                ctx.set_fill(0x7feeeeee);
                ctx.fill();
            }
            {          
                RenderContext context(&ctx);
                ctx.rotate(-animation_value * 0.1);
                ctx.begin_path();
                ctx.ellipse(-14,16,30,14,-M_PI/10);
                ctx.set_fill(0x7feeeeee);
                ctx.fill();
            }
            ctx.set_stroke(0xff292929);
            ctx.set_line_width(7);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(25,-10.5);
            ctx.qcurve_to(41.5,-15+2*animation_value,58,-7.5+2*animation_value);
            ctx.move_to(25,10.5);
            ctx.qcurve_to(41.5,15-2*animation_value,58,7.5-2*animation_value);
            ctx.stroke();
            ctx.begin_path();
            ctx.arc(25,0,24.5);
            ctx.set_fill(0xff454545);
            ctx.fill();
            ctx.begin_path();
            ctx.arc(25,0,17.5);
            ctx.set_fill(0xff555555);
            ctx.fill();
            break;
        case MobID::kSquare:
            ctx.set_fill(0xffffe869);
            ctx.set_stroke(0xffcfbc55);
            ctx.set_line_width(0.15*radius);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.rect(-radius * 0.707, -radius * 0.707, radius * 1.414, radius * 1.414);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kDigger: {
            attr.flower_attrs.radius = attr.radius;
            BitMath::set(attr.flower_attrs.equip_flags, EquipmentFlags::kCutter);
            BitMath::set(attr.flower_attrs.face_flags, FaceFlags::kSquareEyes);
            draw_static_flower(ctx, attr.flower_attrs);
            break;
        };
        case MobID::kWanderingGuard:
            ctx.scale(radius / 27.f);
            ctx.set_fill(0xff75869A);
            ctx.set_stroke(0xff344657);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 24);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffB8C3CE);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 20, -105.f * (float) M_PI / 180.f, 105.f * (float) M_PI / 180.f, 0);
            ctx.stroke();
            ctx.set_fill(0xffD5DCE2);
            ctx.set_stroke(0xff52616E);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.arc(-5, 14, 3);
            ctx.arc(-5, -14, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff283747);
            ctx.set_stroke(0xff18232E);
            ctx.set_line_width(2);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.round_rect(9 - 6.5f, -11.5f, 13, 23, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffDCE9F1);
            ctx.set_stroke(0xff65798A);
            ctx.set_line_width(1);
            ctx.fill_rect(10.5f, -7.5f, 3, 15);
            break;
        case MobID::kStoneBeetle:
            ctx.scale(radius / 35.f);
            ctx.set_stroke(0xff414943);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.round_line_join();
            ctx.begin_path();
            ctx.move_to(27, 10);
            ctx.line_to(35, 10);
            ctx.move_to(27, -10);
            ctx.line_to(35, -10);
            ctx.stroke();
            ctx.set_fill(0xff777C75);
            ctx.set_stroke(0xff343B38);
            ctx.set_line_width(3.5f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 33, 26);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA5AAA0);
            ctx.set_stroke(0xff525A52);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(-10 - 10, -9 - 8.5f, 20, 17, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff969C92);
            ctx.begin_path();
            ctx.round_rect(-10 - 10, 10 - 8.5f, 20, 17, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB3B7AD);
            ctx.begin_path();
            ctx.round_rect(12 - 9, -14, 18, 28, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE8D9A8);
            ctx.set_stroke(0xff343B38);
            ctx.set_line_width(1);
            ctx.begin_path();
            ctx.arc(26, 7, 2.5f);
            ctx.arc(26, -7, 2.5f);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kRustMite:
            ctx.scale(radius / 17.f);
            ctx.set_stroke(0xff874729);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.begin_path();
            for (int i = -1; i <= 1; ++i) {
                ctx.move_to(-9, (float) i * 12);
                ctx.line_to(-16, (float) i * 12);
                ctx.move_to(9, (float) i * 12);
                ctx.line_to(16, (float) i * 12);
            }
            ctx.stroke();
            ctx.set_fill(0xffC66D39);
            ctx.set_stroke(0xff693C2A);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 15);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE5A15C);
            ctx.set_stroke(0xff8C502E);
            ctx.set_line_width(1);
            ctx.begin_path();
            ctx.arc(-5, -5, 3);
            ctx.arc(5, 6, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff272727);
            ctx.begin_path();
            ctx.arc(7, 5, 2.5f);
            ctx.arc(7, -5, 2.5f);
            ctx.fill();
            break;
        case MobID::kMimicChest:
            ctx.scale(radius / 29.f);
            ctx.set_fill(0xff8E5635);
            ctx.set_stroke(0xff4B2C21);
            ctx.set_line_width(3);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.round_rect(-21.5f, -19.5f, 43, 39, 4);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffE5B95C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.round_rect(-19, -17, 38, 34, 3);
            ctx.stroke();
            ctx.set_stroke(0xff4B2C21);
            ctx.begin_path();
            ctx.move_to(3, -17);
            ctx.line_to(3, 17);
            ctx.stroke();
            ctx.set_fill(0xffF0CB70);
            ctx.set_stroke(0xff87622F);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(11 - 4.5f, -6, 9, 12, 2);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF5E9CC);
            ctx.set_stroke(0xff3B2520);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.ellipse(13, 0, 3, 5);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC74E46);
            ctx.begin_path();
            ctx.arc(14, 0, 1.5f);
            ctx.fill();
            break;
        case MobID::kCoinBeetle:
            ctx.scale(radius / 20.f);
            ctx.set_stroke(0xff9A702B);
            ctx.set_line_width(2.5f);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(14, 8);
            ctx.line_to(21, 8);
            ctx.move_to(14, -8);
            ctx.line_to(21, -8);
            ctx.stroke();
            ctx.set_fill(0xffF1C44E);
            ctx.set_stroke(0xff936A27);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 18);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffFFF0A4);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(0, 0, 13);
            ctx.stroke();
            ctx.set_stroke(0xff956725);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-7.5f, 4);
            ctx.line_to(0, -4.5f);
            ctx.line_to(7.5f, 4);
            ctx.stroke();
            ctx.set_fill(0xff3D3527);
            ctx.begin_path();
            ctx.arc(14, 5, 2);
            ctx.arc(14, -5, 2);
            ctx.fill();
            break;
        case MobID::kRoyalBee:
            ctx.scale(radius / 23.f);
            SET_BASE_COLOR(0xffF5C84E);
            ctx.set_fill(0xffB7E7F0);
            ctx.set_stroke(0xff548A9C);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.ellipse(-5, 17, 11, 7);
            ctx.ellipse(-5, -17, 11, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff6D5529);
            ctx.set_stroke(0xff4B4030);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(-25, 9);
            ctx.line_to(-32, 0);
            ctx.line_to(-25, -9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(base_color);
            ctx.set_stroke(0xff6D5529);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.ellipse(0, 0, 21, 15);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff4B4030);
            ctx.fill_rect(-7 - 2.5f, -13.5f, 5, 27);
            ctx.fill_rect(2 - 2.5f, -13.5f, 5, 27);
            ctx.set_fill(0xff292929);
            ctx.begin_path();
            ctx.arc(15, 6, 2.5f);
            ctx.arc(15, -6, 2.5f);
            ctx.fill();
            break;
        case MobID::kThornBug:
            ctx.scale(radius / 28.f);
            ctx.set_fill(0xff75B96C);
            ctx.set_stroke(0xff285C36);
            ctx.set_line_width(2);
            ctx.round_line_join();
            ctx.begin_path();
            for (uint32_t i = 0; i < 6; ++i) {
                float a = (float) i * (float) M_PI / 3.f;
                float tip = 34.f + animation_value * 2.f;
                float bx = cosf(a) * 22.f;
                float by = sinf(a) * 22.f;
                float tx = cosf(a) * tip;
                float ty = sinf(a) * tip;
                float px = -sinf(a) * 4.5f;
                float py = cosf(a) * 4.5f;
                ctx.move_to(bx + px, by + py);
                ctx.line_to(tx, ty);
                ctx.line_to(bx - px, by - py);
                ctx.close_path();
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff4B9253);
            ctx.set_stroke(0xff285C36);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 22);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff8BCB75);
            ctx.set_stroke(0xff377A44);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff243A25);
            ctx.begin_path();
            ctx.arc(9, 5, 2.5f);
            ctx.arc(9, -5, 2.5f);
            ctx.fill();
            break;
        case MobID::kShieldSentinel:
            ctx.scale(radius / 39.f);
            ctx.set_fill(0xff596D82);
            ctx.set_stroke(0xff26394D);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(0, 0, 32);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff9AACBD);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 25);
            ctx.stroke();
            ctx.set_fill(0xffCBD6DF);
            ctx.set_stroke(0xff52687B);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.arc(-18, 15, 4);
            ctx.arc(-18, -15, 4);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB9C6D0);
            ctx.set_stroke(0xff30495D);
            ctx.set_line_width(3.5f);
            ctx.begin_path();
            ctx.move_to(28, -24);
            ctx.qcurve_to(44, 0, 28, 24);
            ctx.qcurve_to(20, 0, 28, -24);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE5BE61);
            ctx.set_stroke(0xff8D7137);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(30, 0, 7);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kSpearSentinel:
            ctx.scale(radius / 32.f);
            ctx.set_stroke(0xffC19A50);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(8, 0);
            ctx.line_to(38, 0);
            ctx.stroke();
            ctx.set_fill(0xffD96558);
            ctx.set_stroke(0xff813B38);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.move_to(31, 6);
            ctx.line_to(31, -6);
            ctx.line_to(23, 0);
            ctx.close_path();
            ctx.move_to(31, -6);
            ctx.line_to(31, 6);
            ctx.line_to(23, 0);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA64D4C);
            ctx.set_stroke(0xff542C34);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 26);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffE4A08D);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 19, (float) M_PI, 0.f, 0);
            ctx.stroke();
            ctx.set_fill(0xffE6E9EC);
            ctx.set_stroke(0xff6A747C);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(42, 0);
            ctx.line_to(32, 4.5f);
            ctx.line_to(32, -4.5f);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF5D9BE);
            ctx.begin_path();
            ctx.arc(14, 7, 2.5f);
            ctx.arc(14, -7, 2.5f);
            ctx.fill();
            break;
        case MobID::kPhantomOrb:
            ctx.scale(radius / 25.f);
            ctx.set_global_alpha(0.45f);
            ctx.set_fill(0xff9881BE);
            ctx.set_stroke(0xff625080);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 24);
            ctx.fill();
            ctx.stroke();
            ctx.set_global_alpha(0.8f);
            ctx.set_fill(0xff77629E);
            ctx.set_stroke(0xff4D3E6A);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(0, 0, 17);
            ctx.fill();
            ctx.stroke();
            ctx.set_global_alpha(1.f);
            ctx.set_fill(0xffE8DFFF);
            ctx.set_stroke(0xffA99BDF);
            ctx.set_line_width(1);
            ctx.begin_path();
            ctx.ellipse(8, 7, 4, 5);
            ctx.ellipse(8, -7, 4, 5);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff432D65);
            ctx.begin_path();
            ctx.arc(9, 7, 1.8f);
            ctx.arc(9, -7, 1.8f);
            ctx.fill();
            ctx.set_fill(0xffC8B8EC);
            for (uint32_t i = 0; i < 3; ++i) {
                float a = (float) i * 2.f * (float) M_PI / 3.f + animation_value * 0.2f;
                ctx.begin_path();
                ctx.arc(cosf(a) * 24.f, sinf(a) * 24.f, 2.5f);
                ctx.fill();
            }
            break;
        case MobID::kFallenKnight:
            ctx.scale(radius / 45.f);
            ctx.set_fill(0xff64606C);
            ctx.set_stroke(0xff27252E);
            ctx.set_line_width(2);
            ctx.round_line_cap();
            for (uint32_t i = 0; i < 4; ++i) {
                float a = (float) i * (float) M_PI / 2.f + (float) M_PI / 4.f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * 30.f, sinf(a) * 30.f);
                ctx.line_to(cosf(a) * 38.f, sinf(a) * 38.f);
                ctx.stroke();
            }
            ctx.set_fill(0xff34343E);
            ctx.set_stroke(0xff171923);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(0, 0, 39);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffBD4C55);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.move_to(9, -9);
            ctx.line_to(32, 0);
            ctx.line_to(9, 9);
            ctx.stroke();
            ctx.set_fill(0xff1B1A23);
            ctx.set_stroke(0xff5C343D);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(17 - 5.5f, -10, 11, 20, 2);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffFF7775);
            ctx.begin_path();
            ctx.arc(20, 6, 2.5f);
            ctx.arc(20, -6, 2.5f);
            ctx.fill();
            break;
        case MobID::kFallenKing:
            ctx.scale(radius / 92.f);
            {
                RenderContext rc(&ctx);
                ctx.rotate(animation_value * 0.08f);
                ctx.set_fill(0xffE0B15B);
                ctx.set_stroke(0xff7A572F);
                ctx.set_line_width(2.5f);
                ctx.round_line_join();
                for (uint32_t i = 0; i < 6; ++i) {
                    float a = (float) i * (float) M_PI / 3.f;
                    float bx = cosf(a) * 95.f;
                    float by = sinf(a) * 95.f;
                    float tx = cosf(a) * 110.f;
                    float ty = sinf(a) * 110.f;
                    float px = -sinf(a) * 5.5f;
                    float py = cosf(a) * 5.5f;
                    ctx.begin_path();
                    ctx.move_to(bx + px, by + py);
                    ctx.line_to(tx, ty);
                    ctx.line_to(bx - px, by - py);
                    ctx.close_path();
                    ctx.fill();
                    ctx.stroke();
                }
            }
            ctx.set_fill(0xff302B43);
            ctx.set_stroke(0xff161321);
            ctx.set_line_width(6);
            ctx.begin_path();
            ctx.arc(0, 0, 80);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xffA98747);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.arc(0, 0, 63);
            ctx.stroke();
            ctx.set_fill(0xff8B394D);
            ctx.set_stroke(0xffD3AB55);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(12, 0, 25);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE4BA59);
            ctx.set_stroke(0xff78562B);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.move_to(39, -20);
            ctx.line_to(60, -8);
            ctx.line_to(62, 12);
            ctx.line_to(48, 20);
            ctx.line_to(39, 8);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD95460);
            ctx.set_stroke(0xff722B39);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(49, 0, 9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF7D3CF);
            ctx.set_stroke(0xff9D454C);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(35, 21, 5, 8);
            ctx.ellipse(35, -21, 5, 8);
            ctx.fill();
            ctx.stroke();
            break;
        case MobID::kLeafBeetle:
            ctx.scale(radius / 22.f);
            ctx.set_fill(0xff7BC96A);
            ctx.set_stroke(0xff2F6B3C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 20, 14);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3F8B4A);
            ctx.begin_path();
            ctx.ellipse(-6, -2, 10, 12);
            ctx.ellipse(6, -2, 10, 12);
            ctx.fill();
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(12, -4, 2);
            ctx.arc(12, 4, 2);
            ctx.fill();
            break;
        case MobID::kJungleMite:
            ctx.scale(radius / 16.f);
            ctx.set_fill(0xffC5D65A);
            ctx.set_stroke(0xff6A7A34);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, 14);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff6A7A34);
            ctx.set_line_width(2);
            for (int i = 0; i < 6; ++i) {
                float a = (float)i * (float)M_PI / 3.f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * 12.f, sinf(a) * 12.f);
                ctx.line_to(cosf(a) * 18.f, sinf(a) * 18.f);
                ctx.stroke();
            }
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(5, -4, 2);
            ctx.arc(5, 4, 2);
            ctx.fill();
            break;
        case MobID::kVineBug:
            ctx.scale(radius / 28.f);
            ctx.set_fill(0xff3D8B4A);
            ctx.set_stroke(0xff245D35);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 22);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff245D35);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            for (int i = 0; i < 4; ++i) {
                float a = (float)i * (float)M_PI / 2.f + 0.4f;
                ctx.begin_path();
                ctx.move_to(cosf(a) * 18.f, sinf(a) * 18.f);
                ctx.qcurve_to(cosf(a + 0.5f) * 30.f, sinf(a + 0.5f) * 30.f,
                                       cosf(a) * 34.f, sinf(a) * 34.f);
                ctx.stroke();
            }
            ctx.set_fill(0xff7BC96A);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.fill();
            break;
        case MobID::kJungleSpider:
            ctx.scale(radius / 26.f);
            ctx.set_fill(0xff3A5A48);
            ctx.set_stroke(0xff1E3328);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 18);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff1E3328);
            ctx.set_line_width(3);
            for (int i = 0; i < 8; ++i) {
                float a = -0.9f + (float)i * 0.26f;
                float side = (i < 4) ? 1.f : -1.f;
                ctx.begin_path();
                ctx.move_to(4, side * 10);
                ctx.line_to(18, side * (16 + (float)(i % 4) * 3));
                ctx.line_to(28, side * (10 + (float)(i % 4) * 2));
                ctx.stroke();
            }
            ctx.set_fill(0xffD95460);
            ctx.begin_path();
            ctx.arc(10, -5, 3);
            ctx.arc(10, 5, 3);
            ctx.fill();
            ctx.set_fill(0xff2A4034);
            ctx.begin_path();
            ctx.move_to(-4, -6);
            ctx.line_to(4, 0);
            ctx.line_to(-4, 6);
            ctx.close_path();
            ctx.fill();
            break;
        case MobID::kEmeraldMantis:
            ctx.scale(radius / 37.f);
            ctx.set_fill(0xff6FD65A);
            ctx.set_stroke(0xff2F7A3C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 28, 16);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff4BB84A);
            ctx.begin_path();
            ctx.move_to(20, -18);
            ctx.line_to(42, -8);
            ctx.line_to(38, 0);
            ctx.line_to(18, -6);
            ctx.close_path();
            ctx.move_to(20, 18);
            ctx.line_to(42, 8);
            ctx.line_to(38, 0);
            ctx.line_to(18, 6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(22, -4, 2.5f);
            ctx.arc(22, 4, 2.5f);
            ctx.fill();
            break;
        case MobID::kPoisonToad:
            ctx.scale(radius / 34.f);
            ctx.set_fill(0xff5FAF8B);
            ctx.set_stroke(0xff2D6D59);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 26);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffE6D35A);
            ctx.begin_path();
            ctx.arc(-8, -8, 5);
            ctx.arc(6, -10, 4);
            ctx.arc(0, 8, 4.5f);
            ctx.fill();
            ctx.set_fill(0xff3D8B4A);
            ctx.begin_path();
            ctx.ellipse(-16, 16, 7, 5);
            ctx.ellipse(16, 16, 7, 5);
            ctx.ellipse(-18, -4, 6, 4);
            ctx.ellipse(18, -4, 6, 4);
            ctx.fill();
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(10, -12, 4);
            ctx.arc(10, 8, 4);
            ctx.fill();
            break;
        case MobID::kSporeMushroom:
            ctx.scale(radius / 25.f);
            ctx.set_fill(0xffA94E7A);
            ctx.set_stroke(0xff5C2A48);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 22);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD4A0BC);
            ctx.begin_path();
            ctx.arc(-8, -6, 5);
            ctx.arc(7, -8, 4);
            ctx.arc(2, 6, 4.5f);
            ctx.arc(-4, 8, 3);
            ctx.arc(10, 4, 3);
            ctx.fill();
            ctx.set_fill(0xff3A2040);
            ctx.begin_path();
            ctx.arc(0, 0, 6);
            ctx.fill();
            break;
        case MobID::kCarnivorousPlant:
            ctx.scale(radius / 38.f);
            ctx.set_fill(0xff2F6B3C);
            ctx.set_stroke(0xff1E4428);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 20);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3D8B4A);
            ctx.begin_path();
            ctx.move_to(8, -18);
            ctx.line_to(36, -6);
            ctx.line_to(34, 8);
            ctx.line_to(8, 6);
            ctx.close_path();
            ctx.move_to(8, 18);
            ctx.line_to(36, 6);
            ctx.line_to(34, -8);
            ctx.line_to(8, -6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC4454A);
            ctx.begin_path();
            ctx.arc(14, 0, 8);
            ctx.fill();
            break;
        case MobID::kRiverCrab:
            ctx.scale(radius / 31.f);
            ctx.set_fill(0xff4A9A8A);
            ctx.set_stroke(0xff2A5A52);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 24, 16);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3A7A6A);
            ctx.begin_path();
            ctx.arc(22, -12, 8);
            ctx.arc(22, 12, 8);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff2A5A52);
            ctx.set_line_width(3);
            for (int i = 0; i < 3; ++i) {
                float y = -10.f + (float)i * 10.f;
                ctx.begin_path();
                ctx.move_to(-8, y);
                ctx.line_to(-22, y - 4);
                ctx.stroke();
                ctx.begin_path();
                ctx.move_to(-8, y);
                ctx.line_to(-22, y + 4);
                ctx.stroke();
            }
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(8, -6, 3);
            ctx.arc(8, 6, 3);
            ctx.fill();
            break;
        case MobID::kStoneGuardian:
            ctx.scale(radius / 46.f);
            ctx.set_fill(0xff6A7A68);
            ctx.set_stroke(0xff3A4538);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(0, 0, 30);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff8A9A82);
            ctx.begin_path();
            ctx.round_rect(-28, -28, 20, 20, 3);
            ctx.round_rect(8, -28, 20, 20, 3);
            ctx.round_rect(-28, 8, 20, 20, 3);
            ctx.round_rect(8, 8, 20, 20, 3);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff7CCB61);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.fill();
            break;
        case MobID::kJungleWasp:
            ctx.scale(radius / 24.f);
            ctx.set_fill(0xffE0A83A);
            ctx.set_stroke(0xff7A5A20);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 20, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.rect(-10, -12, 5, 24);
            ctx.rect(0, -12, 5, 24);
            ctx.rect(10, -12, 4, 24);
            ctx.fill();
            ctx.set_global_alpha(0.45f);
            ctx.set_fill(0xffDDEEFF);
            ctx.begin_path();
            ctx.ellipse(-4, -16, 10, 6);
            ctx.ellipse(-4, 16, 10, 6);
            ctx.fill();
            ctx.set_global_alpha(1.f);
            ctx.set_fill(0xffD95460);
            ctx.begin_path();
            ctx.arc(14, -4, 2.5f);
            ctx.arc(14, 4, 2.5f);
            ctx.fill();
            ctx.set_stroke(0xff7A5A20);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-18, 0);
            ctx.line_to(-28, 0);
            ctx.stroke();
            break;
        case MobID::kAncientTreant:
            ctx.scale(radius / 95.f);
            ctx.set_fill(0xff2F6B3C);
            ctx.set_stroke(0xff1A3A24);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.arc(0, 0, 78);
            ctx.fill();
            ctx.stroke();
            for (uint32_t i = 0; i < 6; ++i) {
                float a = (float)i * (float)M_PI / 3.f;
                ctx.set_fill(0xff3D8B4A);
                ctx.begin_path();
                ctx.ellipse(cosf(a) * 70.f, sinf(a) * 70.f, 18, 10);
                ctx.fill();
            }
            ctx.set_fill(0xff8A633F);
            ctx.set_stroke(0xff493823);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.arc(0, 0, 36);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff6B4A2E);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 24);
            ctx.stroke();
            ctx.set_fill(0xffA8E07A);
            ctx.begin_path();
            ctx.arc(-10, -6, 7);
            ctx.arc(10, -6, 7);
            ctx.fill();
            ctx.set_fill(0xff1A1A1A);
            ctx.begin_path();
            ctx.arc(-10, -6, 3);
            ctx.arc(10, -6, 3);
            ctx.fill();
            break;
        case MobID::kReefStone:
            ctx.scale(radius / 22.f);
            ctx.set_fill(0xff8BA5B2);
            ctx.set_stroke(0xff476674);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 21, 17);
            ctx.fill();
            ctx.stroke();
            // Rock face weathering
            ctx.set_stroke(0xff6A8A97);
            ctx.set_line_width(2.5);
            ctx.begin_path();
            ctx.partial_arc(7, -5, 9, 0.6f, 3.4f, 0);
            ctx.stroke();
            ctx.set_fill(0xff253B47);
            ctx.begin_path();
            ctx.arc(-6, 3, 2.4f);
            ctx.arc(6, 3, 2.4f);
            ctx.fill();
            break;
        case MobID::kSeaFry:
            ctx.scale(radius / 14.f);
            ctx.set_fill(0xff83D5E8);
            ctx.set_stroke(0xff267CA4);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 12, 7.5f);
            ctx.fill();
            ctx.stroke();
            // Tail fin
            ctx.set_fill(0xff6FC6DD);
            ctx.begin_path();
            ctx.ellipse(-12, 0, 5, 4);
            ctx.fill();
            ctx.stroke();
            // Side fin
            ctx.set_fill(0xff9FE0EF);
            ctx.begin_path();
            ctx.ellipse(0, 4, 4.5f, 2.4f);
            ctx.fill();
            ctx.set_fill(0xff173C58);
            ctx.begin_path();
            ctx.arc(6, -1.5f, 2);
            ctx.fill();
            break;
        case MobID::kMoonJelly:
            ctx.scale(radius / 22.f);
            // Trailing tentacles first so the bell overlaps them
            ctx.set_stroke(0xff8DD9F0);
            ctx.set_line_width(3.5);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, 4);
            ctx.qcurve_to(-9, 13, -5, 21);
            ctx.move_to(0, 5);
            ctx.qcurve_to(0, 15, 0, 23);
            ctx.move_to(6, 4);
            ctx.qcurve_to(9, 13, 5, 21);
            ctx.stroke();
            // Bell
            ctx.set_fill(0xff8DD9F0);
            ctx.set_stroke(0xff377BA9);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(0, -2, 15, 11);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffB9EAF6);
            ctx.begin_path();
            ctx.ellipse(-4, -6, 6, 4);
            ctx.fill();
            ctx.set_fill(0xff163B62);
            ctx.begin_path();
            ctx.arc(-4, -1, 2);
            ctx.arc(4, -1, 2);
            ctx.fill();
            break;
        case MobID::kCoralStar:
            ctx.scale(radius / 28.f);
            ctx.set_fill(0xfff6a064);
            ctx.set_stroke(0xffb95342);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            // Five rounded arms
            for (uint32_t i = 0; i < 5; ++i) {
                float a = (float)i * (float)(2.0 * M_PI / 5.0) - (float)M_PI / 2.f;
                ctx.ellipse(cosf(a) * 13.f, sinf(a) * 13.f, 9, 6.5f, a);
            }
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffe0874a);
            ctx.begin_path();
            ctx.arc(0, 0, 9);
            ctx.fill();
            ctx.set_fill(0xff422D35);
            ctx.begin_path();
            ctx.arc(-3.5f, -2, 1.9f);
            ctx.arc(3.5f, -2, 1.9f);
            ctx.fill();
            break;
        case MobID::kReefCrab:
            ctx.scale(radius / 32.f);
            // Claws
            ctx.set_fill(0xffDA8060);
            ctx.set_stroke(0xff8B423C);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(-22, -8, 8, 6, 0.5f);
            ctx.ellipse(22, -8, 8, 6, -0.5f);
            ctx.fill();
            ctx.stroke();
            // Shell
            ctx.set_fill(0xffDA8060);
            ctx.begin_path();
            ctx.ellipse(0, 0, 24, 16);
            ctx.fill();
            ctx.stroke();
            // Shell ridge
            ctx.set_stroke(0xffB0604A);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 14, 3.4f, 6.0f, 0);
            ctx.stroke();
            ctx.set_fill(0xff2C292B);
            ctx.begin_path();
            ctx.arc(-8, -6, 2.6f);
            ctx.arc(8, -6, 2.6f);
            ctx.fill();
            break;
        case MobID::kSpinePuffer:
            ctx.scale(radius / 35.f);
            // Spines around the body
            ctx.set_stroke(0xffb98948);
            ctx.set_line_width(3);
            ctx.round_line_cap();
            ctx.begin_path();
            for (uint32_t i = 0; i < 8; ++i) {
                float a = (float)i * (float)(M_PI / 4.f) + 0.2f;
                ctx.move_to(cosf(a) * 20.f, sinf(a) * 20.f);
                ctx.line_to(cosf(a) * 29.f, sinf(a) * 29.f);
            }
            ctx.stroke();
            // Body
            ctx.set_fill(0xfff4d58d);
            ctx.set_stroke(0xffb98948);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.arc(0, 0, 20);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff45392E);
            ctx.begin_path();
            ctx.arc(-6, -3, 2.8f);
            ctx.arc(6, -3, 2.8f);
            ctx.fill();
            break;
        case MobID::kSeaLeech:
            // Segmented: the head draws the face, the body segments draw plain links.
            if (BitMath::at(flags, 1)) {
                ctx.scale(radius / 30.f);
                ctx.set_fill(0xff426A87);
                ctx.set_stroke(0xff193C58);
                ctx.set_line_width(2.4);
                ctx.begin_path();
                ctx.ellipse(0, 0, 15, 11);
                ctx.fill();
                ctx.stroke();
            } else {
                ctx.scale(radius / 30.f);
                ctx.set_fill(0xff426A87);
                ctx.set_stroke(0xff193C58);
                ctx.set_line_width(2.6);
                ctx.begin_path();
                ctx.ellipse(0, 0, 19, 14);
                ctx.fill();
                ctx.stroke();
                ctx.set_fill(0xffE5F4FC);
                ctx.begin_path();
                ctx.arc(-5.5f, -3, 2.6f);
                ctx.arc(5.5f, -3, 2.6f);
                ctx.fill();
            }
            break;
        case MobID::kMorayEel:
            ctx.scale(radius / 38.f);
            // Long body
            ctx.set_fill(0xff4A9E92);
            ctx.set_stroke(0xff245A62);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.ellipse(0, 0, 30, 13);
            ctx.fill();
            ctx.stroke();
            // Tail
            ctx.set_fill(0xff3E8A80);
            ctx.begin_path();
            ctx.ellipse(-27, 0, 10, 6);
            ctx.fill();
            ctx.stroke();
            // Head ridge
            ctx.set_stroke(0xff2E6E68);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.partial_arc(14, -2, 8, 3.2f, 6.1f, 0);
            ctx.stroke();
            // Mouth line
            ctx.set_stroke(0xff122F3A);
            ctx.set_line_width(1.8);
            ctx.begin_path();
            ctx.partial_arc(18, 1, 6, 5.4f, 0.9f, 0);
            ctx.stroke();
            ctx.set_fill(0xff122F3A);
            ctx.begin_path();
            ctx.arc(13, -5, 2.3f);
            ctx.arc(19, -4, 2.3f);
            ctx.fill();
            break;
        case MobID::kInkCuttle:
            ctx.scale(radius / 42.f);
            // Arms
            ctx.set_stroke(0xff49386F);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, 12);
            ctx.qcurve_to(-12, 22, -7, 30);
            ctx.move_to(0, 14);
            ctx.qcurve_to(0, 24, 0, 32);
            ctx.move_to(8, 12);
            ctx.qcurve_to(12, 22, 7, 30);
            ctx.stroke();
            // Mantle
            ctx.set_fill(0xff8E73B5);
            ctx.set_stroke(0xff49386F);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.ellipse(0, -2, 17, 22);
            ctx.fill();
            ctx.stroke();
            // Ink sac
            ctx.set_fill(0xff6A4E96);
            ctx.begin_path();
            ctx.ellipse(0, 6, 9, 11);
            ctx.fill();
            ctx.set_fill(0xfff2e6ff);
            ctx.begin_path();
            ctx.arc(-6, -10, 4.2f);
            ctx.arc(6, -10, 4.2f);
            ctx.fill();
            ctx.set_fill(0xff2B1F4A);
            ctx.begin_path();
            ctx.arc(-6, -10, 2.1f);
            ctx.arc(6, -10, 2.1f);
            ctx.fill();
            break;
        case MobID::kAbyssShark:
            ctx.scale(radius / 50.f);
            // Tail fin
            ctx.set_fill(0xff26496B);
            ctx.set_stroke(0xff122C4B);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.ellipse(-38, 0, 12, 7, 0.35f);
            ctx.fill();
            ctx.stroke();
            // Dorsal fin
            ctx.set_fill(0xff2A5075);
            ctx.begin_path();
            ctx.move_to(-4, -13);
            ctx.line_to(8, -30);
            ctx.line_to(14, -11);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff315C83);
            ctx.begin_path();
            ctx.ellipse(0, 0, 36, 17);
            ctx.fill();
            ctx.stroke();
            // Pectoral fin
            ctx.set_fill(0xff26496B);
            ctx.begin_path();
            ctx.move_to(2, 8);
            ctx.line_to(14, 22);
            ctx.line_to(18, 7);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Belly
            ctx.set_fill(0xff4E7FA5);
            ctx.begin_path();
            ctx.ellipse(2, 8, 28, 8);
            ctx.fill();
            // Gills
            ctx.set_stroke(0xff1E4160);
            ctx.set_line_width(2);
            ctx.begin_path();
            for (uint32_t i = 0; i < 3; ++i) {
                float gx = 6.f + (float)i * 6.f;
                ctx.move_to(gx, -12);
                ctx.line_to(gx - 2, -4);
            }
            ctx.stroke();
            ctx.set_fill(0xffFFE8C7);
            ctx.begin_path();
            ctx.arc(20, -4, 3);
            ctx.arc(29, -3, 2.6f);
            ctx.fill();
            break;
        case MobID::kAbyssMaw:
            ctx.scale(radius / 95.f);
            ctx.set_fill(0xff244F75);
            ctx.set_stroke(0xff102A46);
            ctx.set_line_width(6);
            ctx.begin_path();
            ctx.ellipse(0, 0, 88, 76);
            ctx.fill();
            ctx.stroke();
            // Maw
            ctx.set_fill(0xff0A1A2E);
            ctx.begin_path();
            ctx.ellipse(0, 6, 52, 42);
            ctx.fill();
            ctx.stroke();
            // Teeth
            ctx.set_fill(0xff9FD8E8);
            ctx.begin_path();
            for (uint32_t i = 0; i < 8; ++i) {
                float a = (float)i * (float)(M_PI / 4.f) - 0.2f;
                float tx = cosf(a) * 46.f;
                float ty = 6.f + sinf(a) * 37.f;
                ctx.move_to(tx, ty);
                ctx.line_to(tx - cosf(a) * 11.f, ty - sinf(a) * 11.f);
                ctx.line_to(tx - cosf(a) * 11.f + sinf(a) * 5.f, ty - sinf(a) * 11.f - cosf(a) * 5.f);
                ctx.close_path();
            }
            ctx.fill();
            ctx.set_fill(0xff7FE8F5);
            ctx.begin_path();
            ctx.arc(-30, -30, 7);
            ctx.arc(30, -30, 7);
            ctx.fill();
            break;
        case MobID::kFrostStone:
            ctx.scale(radius / 24.f);
            ctx.set_fill(0xffA7BBCB);
            ctx.set_stroke(0xff60798E);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 23, 17);
            ctx.fill();
            ctx.stroke();
            // Snow cap
            ctx.set_fill(0xfff2f7fb);
            ctx.begin_path();
            ctx.ellipse(0, -11, 17, 7);
            ctx.fill();
            ctx.set_fill(0xff344B61);
            ctx.begin_path();
            ctx.arc(-7, 4, 2.4f);
            ctx.arc(7, 4, 2.4f);
            ctx.fill();
            break;
        case MobID::kSnowMite:
            ctx.scale(radius / 12.f);
            ctx.set_stroke(0xff9DAFBD);
            ctx.set_line_width(2);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-5, -6); ctx.line_to(-11, -10);
            ctx.move_to(-5, 6); ctx.line_to(-11, 10);
            ctx.move_to(5, -6); ctx.line_to(11, -10);
            ctx.move_to(5, 6); ctx.line_to(11, 10);
            ctx.stroke();
            ctx.set_fill(0xfff1f4f8);
            ctx.set_stroke(0xff9DAFBD);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 10, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3B4D61);
            ctx.begin_path();
            ctx.arc(4, -2, 1.8f);
            ctx.arc(4, 2, 1.8f);
            ctx.fill();
            break;
        case MobID::kSnowHare:
            ctx.scale(radius / 18.f);
            // Ears
            ctx.set_fill(0xfff6f8fa);
            ctx.set_stroke(0xffadb9c7);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.ellipse(-5, -13, 3.5f, 9, 0.25f * animation_value);
            ctx.ellipse(5, -13, 3.5f, 9, -0.25f * animation_value);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xfff6f8fa);
            ctx.begin_path();
            ctx.arc(0, 0, 13);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff42536a);
            ctx.begin_path();
            ctx.arc(6, -3, 2);
            ctx.arc(6, 3, 2);
            ctx.fill();
            break;
        case MobID::kIceSpider:
            ctx.scale(radius / 25.f);
            ctx.set_stroke(0xff477F9C);
            ctx.set_line_width(2.4);
            ctx.round_line_cap();
            ctx.begin_path();
            #define ice_leg(angle) \
            { \
                float c = cosf(angle) * 20; \
                float s = sinf(angle) * 20; \
                ctx.move_to(0,0); \
                ctx.qcurve_to(s * 0.8, c * 0.5, s, c); \
            }
            ice_leg(-M_PI + 0.9 + sinf(attr.animation) * 0.2)
            ice_leg(-M_PI + 0.3 + cosf(attr.animation) * 0.2)
            ice_leg(-M_PI - 0.3 + sinf(attr.animation) * 0.2)
            ice_leg(-M_PI - 0.9 - cosf(attr.animation) * 0.2)
            ice_leg(-0.9 - sinf(attr.animation) * 0.2)
            ice_leg(-0.3 + cosf(attr.animation) * 0.2)
            ice_leg(0.3 - sinf(attr.animation) * 0.2)
            ice_leg(0.9 - cosf(attr.animation) * 0.2)
            #undef ice_leg
            ctx.stroke();
            // Abdomen (big) + head (small)
            ctx.set_fill(0xff9FD8E7);
            ctx.set_stroke(0xff477F9C);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.ellipse(-5, 0, 13, 11);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC6EAF4);
            ctx.begin_path();
            ctx.arc(9, 0, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1D4E6C);
            ctx.begin_path();
            ctx.arc(11, -2.5f, 1.8f);
            ctx.arc(11, 2.5f, 1.8f);
            ctx.fill();
            break;
        case MobID::kFrostHornet:
            ctx.scale(radius / 28.f);
            // Wings
            ctx.set_fill(0x80cfe9f7);
            ctx.set_stroke(0xff2F648C);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(-4, -9, 12, 5.5f, -0.2f);
            ctx.ellipse(-4, 9, 12, 5.5f, 0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff72B7D2);
            ctx.set_stroke(0xff2F648C);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9, 17);
            ctx.fill();
            ctx.stroke();
            // Stripes
            ctx.set_stroke(0xff3F82A0);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.move_to(-6, -4); ctx.line_to(6, -4);
            ctx.move_to(-6, 3); ctx.line_to(6, 3);
            ctx.stroke();
            // Stinger
            ctx.set_fill(0xff2F648C);
            ctx.begin_path();
            ctx.move_to(0, 17); ctx.line_to(-2.5f, 25); ctx.line_to(2.5f, 25);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff153A58);
            ctx.begin_path();
            ctx.arc(0, -12, 2.2f);
            ctx.arc(0, -5, 2.2f);
            ctx.fill();
            break;
        case MobID::kRimeCentipede:
            // Segmented: the head draws the face, the links draw plain bodies.
            if (BitMath::at(flags, 1)) {
                ctx.scale(radius / 32.f);
                ctx.set_fill(0xff86C7DA);
                ctx.set_stroke(0xff356C89);
                ctx.set_line_width(2.2);
                ctx.begin_path();
                ctx.ellipse(0, 0, 15, 10);
                ctx.fill();
                ctx.stroke();
                ctx.set_stroke(0xff356C89);
                ctx.set_line_width(1.8);
                ctx.begin_path();
                ctx.move_to(-9, 6); ctx.line_to(-13, 11);
                ctx.move_to(9, 6); ctx.line_to(13, 11);
                ctx.stroke();
            } else {
                ctx.scale(radius / 32.f);
                ctx.set_fill(0xff86C7DA);
                ctx.set_stroke(0xff356C89);
                ctx.set_line_width(2.4);
                ctx.begin_path();
                ctx.ellipse(0, 0, 19, 13);
                ctx.fill();
                ctx.stroke();
                ctx.set_stroke(0xff356C89);
                ctx.set_line_width(1.8);
                ctx.begin_path();
                ctx.move_to(-11, 8); ctx.line_to(-15, 13);
                ctx.move_to(11, 8); ctx.line_to(15, 13);
                ctx.stroke();
                ctx.set_fill(0xff193F5D);
                ctx.begin_path();
                ctx.arc(-5, -3, 2.2f);
                ctx.arc(5, -3, 2.2f);
                ctx.fill();
            }
            break;
        case MobID::kBlizzardWisp:
            ctx.scale(radius / 30.f);
            ctx.rotate(attr.animation / 4);
            // Three overlapping cloud puffs
            ctx.set_fill(0xffe3edf5);
            ctx.set_stroke(0xff90afc5);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.arc(-11, 2, 11);
            ctx.arc(4, -4, 13);
            ctx.arc(12, 6, 10);
            ctx.fill();
            ctx.stroke();
            // Spiral gust lines
            ctx.set_stroke(0xffb8cfe0);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 15, 0.4f, 4.2f, 0);
            ctx.partial_arc(0, 0, 9, 2.2f, 6.0f, 0);
            ctx.stroke();
            ctx.set_fill(0xff405E78);
            ctx.begin_path();
            ctx.arc(8, -2, 2.1f);
            ctx.arc(8, 4, 2.1f);
            ctx.fill();
            break;
        case MobID::kGlacierBeast:
            ctx.scale(radius / 40.f);
            // Horns
            ctx.set_fill(0xff8B9FB3);
            ctx.set_stroke(0xff4B6279);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.ellipse(-8, -24, 4, 9, 0.3f);
            ctx.ellipse(8, -24, 4, 9, -0.3f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff8B9FB3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 32, 21);
            ctx.fill();
            ctx.stroke();
            // Fur ridge
            ctx.set_fill(0xffA6B8C9);
            ctx.begin_path();
            ctx.ellipse(0, -12, 22, 8);
            ctx.fill();
            ctx.set_fill(0xff20374F);
            ctx.begin_path();
            ctx.arc(16, -5, 2.6f);
            ctx.arc(24, -3, 2.3f);
            ctx.fill();
            break;
        case MobID::kFrostYeti:
            ctx.scale(radius / 45.f);
            // Arms
            ctx.set_fill(0xfff0f4f7);
            ctx.set_stroke(0xff869eb2);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.ellipse(-28, 8, 8, 20, 0.2f);
            ctx.ellipse(28, 8, 8, 20, -0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xfff0f4f7);
            ctx.begin_path();
            ctx.ellipse(0, 2, 28, 32);
            ctx.fill();
            ctx.stroke();
            // Head
            ctx.set_fill(0xfffafbfd);
            ctx.begin_path();
            ctx.arc(0, -28, 14);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff294B68);
            ctx.begin_path();
            ctx.arc(-5, -30, 2.4f);
            ctx.arc(5, -30, 2.4f);
            ctx.fill();
            break;
        case MobID::kIceGolem:
            ctx.scale(radius / 50.f);
            // Shoulder ice
            ctx.set_fill(0xff5AA0BC);
            ctx.set_stroke(0xff356C8D);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(-26, -10, 14, 11, 0.2f);
            ctx.ellipse(26, -10, 14, 11, -0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff70B6CE);
            ctx.set_stroke(0xff356C8D);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 32, 30);
            ctx.fill();
            ctx.stroke();
            // Ice facets
            ctx.set_fill(0xff9BD5E6);
            ctx.begin_path();
            ctx.ellipse(-10, -8, 13, 10, 0.3f);
            ctx.ellipse(12, 10, 11, 8, -0.3f);
            ctx.fill();
            // Horn shards
            ctx.set_fill(0xff9BD5E6);
            ctx.set_stroke(0xff356C8D);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.move_to(-8, -28); ctx.line_to(-3, -42); ctx.line_to(4, -27);
            ctx.close_path();
            ctx.move_to(6, -28); ctx.line_to(13, -40); ctx.line_to(17, -26);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffe6f9ff);
            ctx.begin_path();
            ctx.arc(-9, 2, 3.4f);
            ctx.arc(9, 2, 3.4f);
            ctx.fill();
            break;
        case MobID::kWinterSovereign:
            ctx.scale(radius / 95.f);
            // Ice crown
            ctx.set_fill(0xff9BD5E6);
            ctx.set_stroke(0xff2E617D);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.move_to(-28, -58); ctx.line_to(-18, -92); ctx.line_to(-8, -60);
            ctx.close_path();
            ctx.move_to(-4, -60); ctx.line_to(4, -98); ctx.line_to(12, -60);
            ctx.close_path();
            ctx.move_to(14, -60); ctx.line_to(26, -86); ctx.line_to(34, -56);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Shoulders
            ctx.set_fill(0xff6FAAC4);
            ctx.set_stroke(0xff2E617D);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-58, 6, 24, 18, 0.2f);
            ctx.ellipse(58, 6, 24, 18, -0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff82BED4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 68, 54);
            ctx.fill();
            ctx.stroke();
            // Frost plates
            ctx.set_fill(0xffA8DCEA);
            ctx.begin_path();
            ctx.ellipse(-18, -14, 24, 15, 0.25f);
            ctx.ellipse(20, 16, 20, 13, -0.25f);
            ctx.fill();
            ctx.set_fill(0xffe8fcff);
            ctx.begin_path();
            ctx.arc(-22, -6, 6.5f);
            ctx.arc(22, -6, 6.5f);
            ctx.fill();
            break;
        case MobID::kEmberStone:
            ctx.scale(radius / 24.f);
            ctx.set_fill(0xff514552);
            ctx.set_stroke(0xff272330);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, 23, 18);
            ctx.fill();
            ctx.stroke();
            // Glowing crack
            ctx.set_stroke(0xff8E3F3A);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(-12, 4);
            ctx.qcurve_to(-2, -2, 4, 3);
            ctx.qcurve_to(9, 7, 15, 1);
            ctx.stroke();
            ctx.set_fill(0xffffa36a);
            ctx.begin_path();
            ctx.arc(-7, -4, 2.4f);
            ctx.arc(7, -4, 2.4f);
            ctx.fill();
            break;
        case MobID::kAshMite:
            ctx.scale(radius / 13.f);
            ctx.set_stroke(0xff5C505B);
            ctx.set_line_width(2);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-5, -5); ctx.line_to(-10, -9);
            ctx.move_to(-5, 5); ctx.line_to(-10, 9);
            ctx.move_to(4, -5); ctx.line_to(9, -9);
            ctx.move_to(4, 5); ctx.line_to(9, 9);
            ctx.stroke();
            ctx.set_fill(0xffAAA0A4);
            ctx.set_stroke(0xff5C505B);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 10, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff2A2531);
            ctx.begin_path();
            ctx.arc(4, -2, 1.8f);
            ctx.arc(4, 2, 1.8f);
            ctx.fill();
            break;
        case MobID::kCinderMoth:
            ctx.scale(radius / 19.f);
            // Wings
            ctx.set_fill(0xffB59A97);
            ctx.set_stroke(0xff75565b);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(-9, -8, 12, 8, 0.35f);
            ctx.ellipse(-9, 8, 12, 8, -0.35f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xffC9A4A0);
            ctx.set_stroke(0xff75565B);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 7, 15);
            ctx.fill();
            ctx.stroke();
            // Antennae
            ctx.set_stroke(0xff75565B);
            ctx.set_line_width(1.8);
            ctx.begin_path();
            ctx.move_to(-2, -14); ctx.qcurve_to(-8, -22, -12, -20);
            ctx.move_to(2, -14); ctx.qcurve_to(8, -22, 12, -20);
            ctx.stroke();
            ctx.set_fill(0xff352C34);
            ctx.begin_path();
            ctx.arc(-3, -9, 1.9f);
            ctx.arc(3, -9, 1.9f);
            ctx.fill();
            break;
        case MobID::kLavaBeetle:
            ctx.scale(radius / 29.f);
            // Legs
            ctx.set_stroke(0xff492E38);
            ctx.set_line_width(2.4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, 8); ctx.line_to(-14, 14);
            ctx.move_to(-6, 14); ctx.line_to(-13, 20);
            ctx.move_to(6, 8); ctx.line_to(14, 14);
            ctx.move_to(6, 14); ctx.line_to(13, 20);
            ctx.stroke();
            // Shell
            ctx.set_fill(0xff9D5147);
            ctx.set_stroke(0xff492E38);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(0, 0, 16, 24);
            ctx.fill();
            ctx.stroke();
            // Glowing seam
            ctx.set_stroke(0xffE58B53);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.move_to(0, -18); ctx.line_to(0, 16);
            ctx.stroke();
            ctx.set_fill(0xffffc28c);
            ctx.begin_path();
            ctx.arc(0, -18, 2.6f);
            ctx.fill();
            break;
        case MobID::kEmberWasp:
            ctx.scale(radius / 28.f);
            // Wings
            ctx.set_fill(0x80F5C08A);
            ctx.set_stroke(0xff783F36);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(-4, -10, 13, 6, -0.25f);
            ctx.ellipse(-4, 10, 13, 6, 0.25f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xffE58B53);
            ctx.set_stroke(0xff783F36);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9, 18);
            ctx.fill();
            ctx.stroke();
            // Bands
            ctx.set_stroke(0xff3A232C);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.move_to(-7, -6); ctx.line_to(7, -6);
            ctx.move_to(-7, 2); ctx.line_to(7, 2);
            ctx.stroke();
            // Stinger
            ctx.set_fill(0xff322531);
            ctx.begin_path();
            ctx.move_to(0, 18); ctx.line_to(-2.6f, 27); ctx.line_to(2.6f, 27);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff322531);
            ctx.begin_path();
            ctx.arc(0, -13, 2.2f);
            ctx.arc(0, -6, 2.2f);
            ctx.fill();
            break;
        case MobID::kMagmaCrab:
            ctx.scale(radius / 38.f);
            // Claws
            ctx.set_fill(0xffB55C47);
            ctx.set_stroke(0xff60352F);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(-26, -10, 10, 7, 0.5f);
            ctx.ellipse(26, -10, 10, 7, -0.5f);
            ctx.fill();
            ctx.stroke();
            // Shell
            ctx.set_fill(0xffB55C47);
            ctx.begin_path();
            ctx.ellipse(0, 0, 29, 21);
            ctx.fill();
            ctx.stroke();
            // Magma glow across the plate
            ctx.set_stroke(0xffE58B53);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 19, 3.3f, 6.1f, 0);
            ctx.stroke();
            ctx.set_fill(0xffffD08A);
            ctx.begin_path();
            ctx.arc(-9, -7, 2.7f);
            ctx.arc(9, -7, 2.7f);
            ctx.fill();
            break;
        case MobID::kFireSpider:
            ctx.scale(radius / 31.f);
            ctx.set_stroke(0xff542A31);
            ctx.set_line_width(2.6);
            ctx.round_line_cap();
            ctx.begin_path();
            #define fire_leg(angle) \
            { \
                float c = cosf(angle) * 22; \
                float s = sinf(angle) * 22; \
                ctx.move_to(0,0); \
                ctx.qcurve_to(s * 0.8, c * 0.5, s, c); \
            }
            fire_leg(-M_PI + 0.9 + sinf(attr.animation) * 0.2)
            fire_leg(-M_PI + 0.3 + cosf(attr.animation) * 0.2)
            fire_leg(-M_PI - 0.3 + sinf(attr.animation) * 0.2)
            fire_leg(-M_PI - 0.9 - cosf(attr.animation) * 0.2)
            fire_leg(-0.9 - sinf(attr.animation) * 0.2)
            fire_leg(-0.3 + cosf(attr.animation) * 0.2)
            fire_leg(0.3 - sinf(attr.animation) * 0.2)
            fire_leg(0.9 - cosf(attr.animation) * 0.2)
            #undef fire_leg
            ctx.stroke();
            // Abdomen + head
            ctx.set_fill(0xffB94E46);
            ctx.set_stroke(0xff542A31);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.ellipse(-6, 0, 16, 13);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffCE6355);
            ctx.begin_path();
            ctx.arc(11, 0, 8);
            ctx.fill();
            ctx.stroke();
            // Ember marking
            ctx.set_stroke(0xffE58B53);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.partial_arc(-6, 0, 9, 3.6f, 5.8f, 0);
            ctx.stroke();
            ctx.set_fill(0xffffC36C);
            ctx.begin_path();
            ctx.arc(13, -2.5f, 1.9f);
            ctx.arc(13, 2.5f, 1.9f);
            ctx.fill();
            break;
        case MobID::kMagmaCentipede:
            // Segmented: the head draws the face, the links draw plain bodies.
            if (BitMath::at(flags, 1)) {
                ctx.scale(radius / 36.f);
                ctx.set_fill(0xff9D4A46);
                ctx.set_stroke(0xff4B2934);
                ctx.set_line_width(2.2);
                ctx.begin_path();
                ctx.ellipse(0, 0, 17, 12);
                ctx.fill();
                ctx.stroke();
                ctx.set_stroke(0xffE58B53);
                ctx.set_line_width(2);
                ctx.begin_path();
                ctx.partial_arc(0, 0, 9, 3.5f, 5.9f, 0);
                ctx.stroke();
                ctx.set_stroke(0xff4B2934);
                ctx.set_line_width(1.8);
                ctx.begin_path();
                ctx.move_to(-10, 7); ctx.line_to(-14, 12);
                ctx.move_to(10, 7); ctx.line_to(14, 12);
                ctx.stroke();
            } else {
                ctx.scale(radius / 36.f);
                ctx.set_fill(0xff9D4A46);
                ctx.set_stroke(0xff4B2934);
                ctx.set_line_width(2.6);
                ctx.begin_path();
                ctx.ellipse(0, 0, 22, 15);
                ctx.fill();
                ctx.stroke();
                ctx.set_stroke(0xffE58B53);
                ctx.set_line_width(2.4);
                ctx.begin_path();
                ctx.partial_arc(0, 0, 13, 3.4f, 6.0f, 0);
                ctx.stroke();
                ctx.set_stroke(0xff4B2934);
                ctx.set_line_width(1.8);
                ctx.begin_path();
                ctx.move_to(-12, 9); ctx.line_to(-17, 15);
                ctx.move_to(12, 9); ctx.line_to(17, 15);
                ctx.stroke();
                ctx.set_fill(0xffffB36D);
                ctx.begin_path();
                ctx.arc(-6, -4, 2.4f);
                ctx.arc(6, -4, 2.4f);
                ctx.fill();
            }
            break;
        case MobID::kObsidianGolem:
            ctx.scale(radius / 46.f);
            // Shoulders
            ctx.set_fill(0xff332F42);
            ctx.set_stroke(0xff211D2C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(-28, -6, 15, 12, 0.2f);
            ctx.ellipse(28, -6, 15, 12, -0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff444052);
            ctx.set_stroke(0xff211D2C);
            ctx.set_line_width(3.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 30, 36);
            ctx.fill();
            ctx.stroke();
            // Lava fissure
            ctx.set_stroke(0xff8E3F3A);
            ctx.set_line_width(3.4);
            ctx.begin_path();
            ctx.move_to(-10, -24);
            ctx.qcurve_to(2, -8, -4, 4);
            ctx.qcurve_to(-9, 14, 4, 24);
            ctx.stroke();
            ctx.set_fill(0xffff8A57);
            ctx.begin_path();
            ctx.arc(-8, -8, 3.2f);
            ctx.arc(8, -8, 3.2f);
            ctx.fill();
            break;
        case MobID::kLavaColossus:
            ctx.scale(radius / 50.f);
            // Shoulders
            ctx.set_fill(0xff74302F);
            ctx.set_stroke(0xff421F2B);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(-30, 0, 16, 20, 0.15f);
            ctx.ellipse(30, 0, 16, 20, -0.15f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff8C3838);
            ctx.set_stroke(0xff421F2B);
            ctx.set_line_width(3.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 32, 30);
            ctx.fill();
            ctx.stroke();
            // Magma cracks
            ctx.set_stroke(0xffE58B53);
            ctx.set_line_width(3.2);
            ctx.begin_path();
            ctx.move_to(-18, -14); ctx.qcurve_to(-4, -4, -12, 6);
            ctx.move_to(10, -18); ctx.qcurve_to(18, -6, 8, 4);
            ctx.move_to(-2, 10); ctx.qcurve_to(6, 16, -4, 22);
            ctx.stroke();
            ctx.set_fill(0xffffD277);
            ctx.begin_path();
            ctx.arc(-9, -8, 3.4f);
            ctx.arc(9, -8, 3.4f);
            ctx.fill();
            break;
        case MobID::kMoltenOverlord:
            ctx.scale(radius / 95.f);
            // Rock horns
            ctx.set_fill(0xff6B3030);
            ctx.set_stroke(0xff351D2A);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(-34, -56); ctx.qcurve_to(-62, -78, -52, -98);
            ctx.qcurve_to(-42, -74, -18, -58);
            ctx.close_path();
            ctx.move_to(34, -56); ctx.qcurve_to(62, -78, 52, -98);
            ctx.qcurve_to(42, -74, 18, -58);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Shoulders
            ctx.set_fill(0xff7A3535);
            ctx.set_stroke(0xff351D2A);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-60, 8, 26, 20, 0.2f);
            ctx.ellipse(60, 8, 26, 20, -0.2f);
            ctx.fill();
            ctx.stroke();
            // Body
            ctx.set_fill(0xff8E3F3A);
            ctx.begin_path();
            ctx.ellipse(0, 0, 70, 56);
            ctx.fill();
            ctx.stroke();
            // Bright magma fissures
            ctx.set_stroke(0xffE58B53);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(-26, -26); ctx.qcurve_to(-6, -8, -18, 10);
            ctx.move_to(14, -30); ctx.qcurve_to(26, -10, 12, 6);
            ctx.move_to(-8, 16); ctx.qcurve_to(4, 26, -6, 36);
            ctx.stroke();
            ctx.set_fill(0xffffD47C);
            ctx.begin_path();
            ctx.arc(-24, -8, 7);
            ctx.arc(24, -8, 7);
            ctx.fill();
            break;
        case MobID::kDuneStone:
            ctx.scale(radius / 23.f);
            ctx.set_fill(0xffD8B77D);
            ctx.set_stroke(0xff967044);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 2, 22, 16);
            ctx.fill();
            ctx.stroke();
            // Wind-eroded groove
            ctx.set_stroke(0xffB08A52);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.move_to(-13, -2);
            ctx.qcurve_to(-2, -8, 11, -1);
            ctx.stroke();
            ctx.set_fill(0xff57442F);
            ctx.begin_path();
            ctx.arc(-6, 5, 2.4f);
            ctx.arc(6, 5, 2.4f);
            ctx.fill();
            break;
        case MobID::kSandMite:
            ctx.scale(radius / 12.f);
            ctx.set_stroke(0xffA17B4C);
            ctx.set_line_width(1.9);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-5, -5); ctx.line_to(-10, -9);
            ctx.move_to(-5, 5); ctx.line_to(-10, 9);
            ctx.move_to(4, -5); ctx.line_to(9, -9);
            ctx.move_to(4, 5); ctx.line_to(9, 9);
            ctx.stroke();
            ctx.set_fill(0xffE7C98C);
            ctx.set_stroke(0xffA17B4C);
            ctx.set_line_width(1.9);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9, 6.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff503B2B);
            ctx.begin_path();
            ctx.arc(3.5f, -1.8f, 1.7f);
            ctx.arc(3.5f, 1.8f, 1.7f);
            ctx.fill();
            break;
        case MobID::kDuneBeetle:
            ctx.scale(radius / 29.f);
            // Legs
            ctx.set_stroke(0xff77512E);
            ctx.set_line_width(2.4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, 7); ctx.line_to(-14, 12);
            ctx.move_to(-6, 13); ctx.line_to(-13, 19);
            ctx.move_to(6, 7); ctx.line_to(14, 12);
            ctx.move_to(6, 13); ctx.line_to(13, 19);
            ctx.stroke();
            // Carapace (broad, flat)
            ctx.set_fill(0xffC79A55);
            ctx.set_stroke(0xff77512E);
            ctx.set_line_width(2.6);
            ctx.begin_path();
            ctx.ellipse(0, 0, 19, 22);
            ctx.fill();
            ctx.stroke();
            // Shell seam
            ctx.set_stroke(0xff9E7238);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.move_to(0, -18); ctx.line_to(0, 18);
            ctx.stroke();
            ctx.set_fill(0xff372B21);
            ctx.begin_path();
            ctx.arc(-5, -14, 2.5f);
            ctx.arc(5, -14, 2.5f);
            ctx.fill();
            break;
        case MobID::kDuneSpider:
            ctx.scale(radius / 31.f);
            ctx.set_stroke(0xff88663F);
            ctx.set_line_width(2.6);
            ctx.round_line_cap();
            ctx.begin_path();
            #define dune_leg(angle) \
            { \
                float c = cosf(angle) * 22; \
                float s = sinf(angle) * 22; \
                ctx.move_to(0,0); \
                ctx.qcurve_to(s * 0.8, c * 0.5, s, c); \
            }
            dune_leg(-M_PI + 0.9 + sinf(attr.animation) * 0.2)
            dune_leg(-M_PI + 0.3 + cosf(attr.animation) * 0.2)
            dune_leg(-M_PI - 0.3 + sinf(attr.animation) * 0.2)
            dune_leg(-M_PI - 0.9 - cosf(attr.animation) * 0.2)
            dune_leg(-0.9 - sinf(attr.animation) * 0.2)
            dune_leg(-0.3 + cosf(attr.animation) * 0.2)
            dune_leg(0.3 - sinf(attr.animation) * 0.2)
            dune_leg(0.9 - cosf(attr.animation) * 0.2)
            #undef dune_leg
            ctx.stroke();
            // Big abdomen + small head
            ctx.set_fill(0xffD2B178);
            ctx.set_stroke(0xff88663F);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.ellipse(-7, 0, 17, 14);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffe0c493);
            ctx.begin_path();
            ctx.arc(10, 0, 7.5f);
            ctx.fill();
            ctx.stroke();
            // Dark band
            ctx.set_stroke(0xff88663F);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.move_to(-18, -4); ctx.line_to(2, -4);
            ctx.stroke();
            ctx.set_fill(0xff453328);
            ctx.begin_path();
            ctx.arc(12, -2.5f, 1.9f);
            ctx.arc(12, 2.5f, 1.9f);
            ctx.fill();
            break;
        case MobID::kDuneHornet:
            ctx.scale(radius / 30.f);
            // Wings
            ctx.set_fill(0x80F2DCAB);
            ctx.set_stroke(0xff8A6035);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(-4, -10, 13, 5.5f, -0.25f);
            ctx.ellipse(-4, 10, 13, 5.5f, 0.25f);
            ctx.fill();
            ctx.stroke();
            // Slender body
            ctx.set_fill(0xffE0B45F);
            ctx.set_stroke(0xff8A6035);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 8, 19);
            ctx.fill();
            ctx.stroke();
            // Bands
            ctx.set_stroke(0xff8A6035);
            ctx.set_line_width(2.2);
            ctx.begin_path();
            ctx.move_to(-6, -6); ctx.line_to(6, -6);
            ctx.move_to(-6, 2); ctx.line_to(6, 2);
            ctx.stroke();
            // Stinger
            ctx.set_fill(0xff8A6035);
            ctx.begin_path();
            ctx.move_to(0, 19); ctx.line_to(-2.4f, 28); ctx.line_to(2.4f, 28);
            ctx.close_path();
            ctx.fill();
            ctx.set_fill(0xff433027);
            ctx.begin_path();
            ctx.arc(0, -14, 2.1f);
            ctx.arc(0, -7, 2.1f);
            ctx.fill();
            break;
        case MobID::kDuneTitan:
            ctx.scale(radius / 50.f);
            // Side plates
            ctx.set_fill(0xff9E7238);
            ctx.set_stroke(0xff62452E);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(-30, 0, 16, 22, 0.1f);
            ctx.ellipse(30, 0, 16, 22, -0.1f);
            ctx.fill();
            ctx.stroke();
            // Main carapace
            ctx.set_fill(0xffB58A55);
            ctx.set_stroke(0xff62452E);
            ctx.set_line_width(3.4);
            ctx.begin_path();
            ctx.ellipse(0, 0, 32, 30);
            ctx.fill();
            ctx.stroke();
            // Carapace seam
            ctx.set_stroke(0xff967044);
            ctx.set_line_width(2.8);
            ctx.begin_path();
            ctx.partial_arc(0, 0, 20, 3.4f, 6.1f, 0);
            ctx.stroke();
            ctx.set_fill(0xffffD58B);
            ctx.begin_path();
            ctx.arc(-10, -8, 3.4f);
            ctx.arc(10, -8, 3.4f);
            ctx.fill();
            break;
        case MobID::kSandwormMonarch:
            ctx.scale(radius / 95.f);
            // Mandibles
            ctx.set_fill(0xffB08A52);
            ctx.set_stroke(0xff795234);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(-34, -26);
            ctx.qcurve_to(-62, -44, -50, -66);
            ctx.qcurve_to(-40, -44, -16, -30);
            ctx.close_path();
            ctx.move_to(34, -26);
            ctx.qcurve_to(62, -44, 50, -66);
            ctx.qcurve_to(40, -44, 16, -30);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            // Head
            ctx.set_fill(0xffD0A66A);
            ctx.set_stroke(0xff795234);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(0, 0, 62, 48);
            ctx.fill();
            ctx.stroke();
            // Segment ridges
            ctx.set_stroke(0xffB08A52);
            ctx.set_line_width(4);
            ctx.begin_path();
            for (uint32_t i = 0; i < 3; ++i) {
                float ry = -18.f + (float)i * 18.f;
                ctx.move_to(-44, ry);
                ctx.qcurve_to(0, ry + 6, 44, ry);
            }
            ctx.stroke();
            ctx.set_fill(0xffffE0A0);
            ctx.begin_path();
            ctx.arc(-20, -14, 7);
            ctx.arc(20, -14, 7);
            ctx.fill();
            break;
        case MobID::kJungleStone:
            ctx.scale(radius / 23.f);
            ctx.set_fill(0xff5C6B58);
            ctx.set_stroke(0xff2E3A2C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 2, 21, 17);
            ctx.fill();
            ctx.stroke();
            // Moss patch
            ctx.set_fill(0xff6E9B4A);
            ctx.begin_path();
            ctx.ellipse(-6, -9, 11, 6, 0.3f);
            ctx.fill();
            ctx.set_fill(0xff1E2A1C);
            ctx.begin_path();
            ctx.arc(-6, 5, 2.3f);
            ctx.arc(6, 5, 2.3f);
            ctx.fill();
            break;
        case MobID::kJungleCrawler:
            ctx.scale(radius / 13.f);
            ctx.set_stroke(0xff3F6B2E);
            ctx.set_line_width(2);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-5, -5); ctx.line_to(-10, -9);
            ctx.move_to(-5, 5); ctx.line_to(-10, 9);
            ctx.move_to(4, -5); ctx.line_to(9, -9);
            ctx.move_to(4, 5); ctx.line_to(9, 9);
            ctx.stroke();
            ctx.set_fill(0xff7FBF4A);
            ctx.set_stroke(0xff3F6B2E);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 10, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1E2A1C);
            ctx.begin_path();
            ctx.arc(4, -2, 1.8f);
            ctx.arc(4, 2, 1.8f);
            ctx.fill();
            break;
        case MobID::kRoyalBoulder:
            ctx.scale(radius / 23.f);
            ctx.set_fill(0xffA8A194);
            ctx.set_stroke(0xff6E675C);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.round_rect(-20, -16, 40, 32, 5.f);
            ctx.fill();
            ctx.stroke();
            // Masonry crack
            ctx.set_stroke(0xff6E675C);
            ctx.set_line_width(2.4);
            ctx.begin_path();
            ctx.move_to(-10, -16); ctx.line_to(-4, -2); ctx.line_to(-9, 8);
            ctx.stroke();
            ctx.set_fill(0xff3E382F);
            ctx.begin_path();
            ctx.arc(-7, 6, 2.3f);
            ctx.arc(7, 6, 2.3f);
            ctx.fill();
            break;
        case MobID::kRoyalPebble:
            ctx.scale(radius / 13.f);
            ctx.set_fill(0xffC4BDAF);
            ctx.set_stroke(0xff6E675C);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.ellipse(0, 0, 9, 6.5f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3E382F);
            ctx.begin_path();
            ctx.arc(3.5f, -1.8f, 1.7f);
            ctx.arc(3.5f, 1.8f, 1.7f);
            ctx.fill();
            break;
        // ---- Ant Hell (new) — 128px local canvas, single circular collide ----
        case MobID::kAcidAnt: {
            ctx.scale(radius / 21.f);
            float leg = animation_value * 2.5f;
            ctx.set_stroke(0xff3F5540);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, -8); ctx.line_to(-16, -14 - leg);
            ctx.move_to(2, -9); ctx.line_to(10, -16 + leg);
            ctx.move_to(10, -7); ctx.line_to(20, -12 - leg);
            ctx.move_to(-6, 8); ctx.line_to(-16, 14 + leg);
            ctx.move_to(2, 9); ctx.line_to(10, 16 - leg);
            ctx.move_to(10, 7); ctx.line_to(20, 12 + leg);
            ctx.stroke();
            // Acid sac
            ctx.set_fill(0xffA6CB66);
            ctx.set_stroke(0xff3F5540);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-18, 0, 14, 12);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x99D3EA7A);
            ctx.begin_path();
            ctx.ellipse(-20, -2, 7, 6);
            ctx.fill();
            // Thorax + head
            ctx.set_fill(0xff6C8B51);
            ctx.begin_path();
            ctx.ellipse(-2, 0, 10, 9);
            ctx.arc(12, 0, 9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff2A3A2C);
            ctx.begin_path();
            ctx.arc(15, -3.5f, 2.2f);
            ctx.arc(15, 3.5f, 2.2f);
            ctx.fill();
            break;
        }
        case MobID::kScoutAnt: {
            ctx.scale(radius / 16.f);
            float leg = animation_value * 3.f;
            ctx.set_stroke(0xff685241);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-4, -6); ctx.line_to(-14, -14 - leg);
            ctx.move_to(4, -7); ctx.line_to(8, -16 + leg);
            ctx.move_to(12, -5); ctx.line_to(22, -11 - leg);
            ctx.move_to(-4, 6); ctx.line_to(-14, 14 + leg);
            ctx.move_to(4, 7); ctx.line_to(8, 16 - leg);
            ctx.move_to(12, 5); ctx.line_to(22, 11 + leg);
            // Long antennae
            ctx.move_to(16, -4); ctx.qcurve_to(24, -14 + leg, 30, -10);
            ctx.move_to(16, 4); ctx.qcurve_to(24, 14 - leg, 30, 10);
            ctx.stroke();
            ctx.set_fill(0xffB89A73);
            ctx.set_stroke(0xff685241);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-12, 0, 11, 8);
            ctx.ellipse(0, 0, 7, 6);
            ctx.arc(12, 0, 7);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD5B78E);
            ctx.begin_path();
            ctx.ellipse(-10, -2, 5, 3.5f);
            ctx.fill();
            ctx.set_fill(0xff3A2C22);
            ctx.begin_path();
            ctx.arc(14, -2.5f, 1.8f);
            ctx.arc(14, 2.5f, 1.8f);
            ctx.fill();
            break;
        }
        case MobID::kArmoredAnt: {
            ctx.scale(radius / 29.f);
            float leg = animation_value * 1.5f;
            ctx.set_stroke(0xff383D3B);
            ctx.set_line_width(6);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, -12); ctx.line_to(-22, -18 - leg);
            ctx.move_to(0, -14); ctx.line_to(6, -24 + leg);
            ctx.move_to(12, -10); ctx.line_to(26, -16 - leg);
            ctx.move_to(-8, 12); ctx.line_to(-22, 18 + leg);
            ctx.move_to(0, 14); ctx.line_to(6, 24 - leg);
            ctx.move_to(12, 10); ctx.line_to(26, 16 + leg);
            ctx.stroke();
            ctx.set_fill(0xff706454);
            ctx.set_stroke(0xff383D3B);
            ctx.set_line_width(6);
            ctx.begin_path();
            ctx.ellipse(-14, 0, 16, 14);
            ctx.ellipse(2, 0, 12, 11);
            ctx.arc(18, 0, 11);
            ctx.fill();
            ctx.stroke();
            // Hex back plate
            ctx.set_fill(0xffA18A67);
            ctx.begin_path();
            ctx.move_to(-14, -10); ctx.line_to(-4, -14); ctx.line_to(4, -10);
            ctx.line_to(4, 10); ctx.line_to(-4, 14); ctx.line_to(-14, 10);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff1E2220);
            ctx.begin_path();
            ctx.arc(20, -4, 2.4f);
            ctx.arc(20, 4, 2.4f);
            ctx.fill();
            break;
        }
        case MobID::kBroodCarrier: {
            ctx.scale(radius / 24.f);
            float leg = animation_value * 2.f;
            ctx.set_stroke(0xff634839);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, -9); ctx.line_to(-18, -15 - leg);
            ctx.move_to(2, -10); ctx.line_to(8, -18 + leg);
            ctx.move_to(12, -8); ctx.line_to(24, -14 - leg);
            ctx.move_to(-6, 9); ctx.line_to(-18, 15 + leg);
            ctx.move_to(2, 10); ctx.line_to(8, 18 - leg);
            ctx.move_to(12, 8); ctx.line_to(24, 14 + leg);
            ctx.stroke();
            ctx.set_fill(0xffB17D58);
            ctx.set_stroke(0xff634839);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-14, 0, 15, 13);
            ctx.ellipse(2, 0, 10, 9);
            ctx.arc(16, 0, 9);
            ctx.fill();
            ctx.stroke();
            // Brood egg on abdomen
            ctx.set_fill(0xffF0D4A2);
            ctx.set_stroke(0xff99866A);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.ellipse(-16, -4, 10, 8);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffD6A775);
            ctx.begin_path();
            ctx.ellipse(0, -2, 4, 3);
            ctx.fill();
            ctx.set_fill(0xff3A2A22);
            ctx.begin_path();
            ctx.arc(18, -3, 2.1f);
            ctx.arc(18, 3, 2.1f);
            ctx.fill();
            break;
        }
        case MobID::kWingedAnt: {
            ctx.scale(radius / 20.f);
            float wing = animation_value * 0.12f;
            // Wings first (under body)
            ctx.set_fill(0x88B7C9B6);
            ctx.set_stroke(0xff708779);
            ctx.set_line_width(4);
            {
                RenderContext c(&ctx);
                ctx.rotate(-0.35f + wing);
                ctx.begin_path();
                ctx.ellipse(-2, -18, 22, 9);
                ctx.fill();
                ctx.stroke();
            }
            {
                RenderContext c(&ctx);
                ctx.rotate(0.35f - wing);
                ctx.begin_path();
                ctx.ellipse(-2, 18, 22, 9);
                ctx.fill();
                ctx.stroke();
            }
            float leg = animation_value * 2.f;
            ctx.set_stroke(0xff65483A);
            ctx.set_line_width(4);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-4, -7); ctx.line_to(-12, -12 - leg);
            ctx.move_to(6, -6); ctx.line_to(14, -11 + leg);
            ctx.move_to(-4, 7); ctx.line_to(-12, 12 + leg);
            ctx.move_to(6, 6); ctx.line_to(14, 11 - leg);
            ctx.stroke();
            ctx.set_fill(0xffA77B5C);
            ctx.set_stroke(0xff65483A);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-12, 0, 11, 9);
            ctx.ellipse(0, 0, 8, 7);
            ctx.arc(12, 0, 8);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff3A2A22);
            ctx.begin_path();
            ctx.arc(14, -2.8f, 2.f);
            ctx.arc(14, 2.8f, 2.f);
            ctx.fill();
            break;
        }
        case MobID::kTunnelGuard: {
            ctx.scale(radius / 30.f);
            float leg = animation_value * 1.8f;
            ctx.set_stroke(0xff453E34);
            ctx.set_line_width(6);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-10, -14); ctx.line_to(-26, -20 - leg);
            ctx.move_to(2, -16); ctx.line_to(4, -28 + leg);
            ctx.move_to(14, -12); ctx.line_to(30, -18 - leg);
            ctx.move_to(-10, 14); ctx.line_to(-26, 20 + leg);
            ctx.move_to(2, 16); ctx.line_to(4, 28 - leg);
            ctx.move_to(14, 12); ctx.line_to(30, 18 + leg);
            ctx.stroke();
            // Mandibles
            ctx.set_fill(0xffC4A36B);
            ctx.set_stroke(0xff453E34);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(18, -8); ctx.qcurve_to(34, -18, 28, -28); ctx.qcurve_to(22, -14, 14, -6);
            ctx.close_path();
            ctx.move_to(18, 8); ctx.qcurve_to(34, 18, 28, 28); ctx.qcurve_to(22, 14, 14, 6);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff886D4F);
            ctx.begin_path();
            ctx.ellipse(-12, 0, 18, 16);
            ctx.ellipse(4, 0, 14, 13);
            ctx.arc(18, 0, 12);
            ctx.fill();
            ctx.stroke();
            // Back ridges
            ctx.set_stroke(0xffC4A36B);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.move_to(-22, -6); ctx.line_to(-4, -8); ctx.line_to(8, -4);
            ctx.move_to(-22, 6); ctx.line_to(-4, 8); ctx.line_to(8, 4);
            ctx.stroke();
            ctx.set_fill(0xff2A241C);
            ctx.begin_path();
            ctx.arc(20, -4.5f, 2.5f);
            ctx.arc(20, 4.5f, 2.5f);
            ctx.fill();
            break;
        }
        case MobID::kSporeAnt: {
            ctx.scale(radius / 22.f);
            float leg = animation_value * 2.f;
            ctx.set_stroke(0xff493D54);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, -8); ctx.line_to(-16, -14 - leg);
            ctx.move_to(2, -9); ctx.line_to(8, -17 + leg);
            ctx.move_to(12, -7); ctx.line_to(22, -13 - leg);
            ctx.move_to(-6, 8); ctx.line_to(-16, 14 + leg);
            ctx.move_to(2, 9); ctx.line_to(8, 17 - leg);
            ctx.move_to(12, 7); ctx.line_to(22, 13 + leg);
            ctx.stroke();
            ctx.set_fill(0xff806785);
            ctx.set_stroke(0xff493D54);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-14, 0, 14, 12);
            ctx.ellipse(0, 0, 9, 8);
            ctx.arc(14, 0, 9);
            ctx.fill();
            ctx.stroke();
            // Spore sac
            ctx.set_fill(0xffB58FBE);
            ctx.begin_path();
            ctx.ellipse(-16, -2, 9, 8);
            ctx.fill();
            ctx.set_fill(0xffC7E18B);
            ctx.begin_path();
            ctx.arc(16, -3.2f, 2.4f);
            ctx.arc(16, 3.2f, 2.4f);
            ctx.fill();
            break;
        }
        case MobID::kFangAnt: {
            ctx.scale(radius / 23.f);
            float leg = animation_value * 2.2f;
            ctx.set_stroke(0xff594536);
            ctx.set_line_width(5);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-6, -8); ctx.line_to(-16, -14 - leg);
            ctx.move_to(2, -9); ctx.line_to(8, -17 + leg);
            ctx.move_to(12, -7); ctx.line_to(22, -13 - leg);
            ctx.move_to(-6, 8); ctx.line_to(-16, 14 + leg);
            ctx.move_to(2, 9); ctx.line_to(8, 17 - leg);
            ctx.move_to(12, 7); ctx.line_to(22, 13 + leg);
            ctx.stroke();
            // Long fangs
            ctx.set_fill(0xffE1C59C);
            ctx.set_stroke(0xff594536);
            ctx.set_line_width(4);
            ctx.begin_path();
            ctx.move_to(14, -5); ctx.line_to(32, -14 - animation_value); ctx.line_to(16, -1);
            ctx.close_path();
            ctx.move_to(14, 5); ctx.line_to(32, 14 + animation_value); ctx.line_to(16, 1);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA58260);
            ctx.set_stroke(0xff594536);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.ellipse(-12, 0, 12, 10);
            ctx.ellipse(0, 0, 9, 8);
            ctx.arc(14, 0, 9);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC5A17B);
            ctx.begin_path();
            ctx.ellipse(-10, -2, 5, 3.5f);
            ctx.fill();
            ctx.set_fill(0xff2E241C);
            ctx.begin_path();
            ctx.arc(16, -3, 2.1f);
            ctx.arc(16, 3, 2.1f);
            ctx.fill();
            break;
        }
        case MobID::kRoyalSentinel: {
            ctx.scale(radius / 31.f);
            float leg = animation_value * 1.6f;
            ctx.set_stroke(0xff6C4735);
            ctx.set_line_width(6);
            ctx.round_line_cap();
            ctx.begin_path();
            ctx.move_to(-8, -12); ctx.line_to(-22, -18 - leg);
            ctx.move_to(2, -14); ctx.line_to(6, -24 + leg);
            ctx.move_to(14, -10); ctx.line_to(28, -16 - leg);
            ctx.move_to(-8, 12); ctx.line_to(-22, 18 + leg);
            ctx.move_to(2, 14); ctx.line_to(6, 24 - leg);
            ctx.move_to(14, 10); ctx.line_to(28, 16 + leg);
            ctx.stroke();
            // Mandibles
            ctx.set_fill(0xffE0B46E);
            ctx.set_stroke(0xff6C4735);
            ctx.set_line_width(5);
            ctx.begin_path();
            ctx.move_to(16, -7); ctx.qcurve_to(30, -16, 26, -24); ctx.qcurve_to(20, -12, 12, -5);
            ctx.close_path();
            ctx.move_to(16, 7); ctx.qcurve_to(30, 16, 26, 24); ctx.qcurve_to(20, 12, 12, 5);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffC18C55);
            ctx.begin_path();
            ctx.ellipse(-14, 0, 16, 14);
            ctx.ellipse(2, 0, 12, 11);
            ctx.arc(18, 0, 12);
            ctx.fill();
            ctx.stroke();
            // Gold royal hex plate
            ctx.set_fill(0xffE0B46E);
            ctx.begin_path();
            ctx.move_to(-14, -9); ctx.line_to(-5, -13); ctx.line_to(3, -9);
            ctx.line_to(3, 9); ctx.line_to(-5, 13); ctx.line_to(-14, 9);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffA77347);
            ctx.begin_path();
            ctx.ellipse(-6, 0, 4, 3);
            ctx.fill();
            ctx.set_fill(0xff3A281C);
            ctx.begin_path();
            ctx.arc(20, -4, 2.5f);
            ctx.arc(20, 4, 2.5f);
            ctx.fill();
            break;
        }
        case MobID::kPushCrate: {
            // Mario wood crate + Zelda seal stamp
            ctx.scale(radius / 28.f);
            ctx.set_fill(0xffC4A36A);
            ctx.set_stroke(0xff6B4E2E);
            ctx.set_line_width(3.5f);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.round_rect(-22, -22, 44, 44, 4);
            ctx.fill();
            ctx.stroke();
            ctx.set_stroke(0xff8B6840);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-22, 0); ctx.line_to(22, 0);
            ctx.move_to(0, -22); ctx.line_to(0, 22);
            ctx.stroke();
            ctx.set_fill(0xffE8D5A3);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.fill();
            ctx.set_stroke(0xff5A3D22);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.arc(0, 0, 8);
            ctx.stroke();
            break;
        }
        case MobID::kYahahaPad: {
            // Soft glowing Korok/seal mark on the ground
            ctx.scale(radius / 22.f);
            ctx.set_global_alpha(0.55f);
            ctx.set_fill(0xff7CFF8A);
            ctx.begin_path();
            ctx.arc(0, 0, 22);
            ctx.fill();
            ctx.set_global_alpha(0.9f);
            ctx.set_stroke(0xffFFE08A);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.arc(0, 0, 16);
            ctx.stroke();
            ctx.set_fill(0xfffff8ec);
            ctx.begin_path();
            ctx.arc(0, 0, 5);
            ctx.fill();
            ctx.set_global_alpha(1.f);
            break;
        }
        case MobID::kTreasureNpc: {
            // Mashup silhouette: leaf cloak + rust helm + coin pouch
            ctx.scale(radius / 32.f);
            ctx.set_fill(0xff4A7A3A);
            ctx.begin_path();
            ctx.ellipse(0, 8, 18, 14);
            ctx.fill();
            ctx.set_fill(0xffF0D878);
            ctx.begin_path();
            ctx.arc(0, -6, 14);
            ctx.fill();
            ctx.set_fill(0xff6B6B6B);
            ctx.begin_path();
            ctx.ellipse(0, -14, 16, 8);
            ctx.fill();
            ctx.set_fill(0xff3A3A3A);
            ctx.begin_path();
            ctx.arc(-5, -8, 2.2f);
            ctx.arc(5, -8, 2.2f);
            ctx.fill();
            ctx.set_fill(0xffC9A227);
            ctx.begin_path();
            ctx.arc(14, 10, 7);
            ctx.fill();
            ctx.set_stroke(0xff5A3D22);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.move_to(-10, -18);
            ctx.qcurve_to(-22, -28, -8, -32);
            ctx.qcurve_to(0, -24, -4, -16);
            ctx.close_path();
            ctx.set_fill(0xff6FAF4A);
            ctx.fill();
            ctx.stroke();
            break;
        }
        case MobID::kLootChest: {
            // Friendly Zelda/Mario chest (gold trim, no teeth)
            ctx.scale(radius / 29.f);
            ctx.set_fill(0xffA86B3C);
            ctx.set_stroke(0xff4B2C21);
            ctx.set_line_width(3);
            ctx.round_line_join();
            ctx.begin_path();
            ctx.round_rect(-22.f, -18.f, 44, 36, 4);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffF0CB70);
            ctx.begin_path();
            ctx.round_rect(-22.f, -18.f, 44, 10, 3);
            ctx.fill();
            ctx.set_fill(0xffE5B95C);
            ctx.set_stroke(0xff87622F);
            ctx.set_line_width(2);
            ctx.begin_path();
            ctx.round_rect(-5.f, -4.f, 10, 12, 2);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff7CFF8A);
            ctx.begin_path();
            ctx.arc(0, 2, 2.5f);
            ctx.fill();
            break;
        }
        default:
            assert(!"Didn't cover mob render");
            break;
    }
}
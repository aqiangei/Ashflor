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
        default:
            assert(!"Didn't cover mob render");
            break;
    }
}
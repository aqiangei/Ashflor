#include <Client/Ui/TitleScreen/TitleScreen.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>

#include <functional>
#include <string>

using namespace Ui;

static Element *make_divider() {
    return new Element(400, 6, { .fill = 0x20000000, .line_width = 3, .round_radius = 3, .v_justify = Style::Top });
}

static Element *make_date(char const *zh, char const *en) {
    return new Ui::DynamicText(20, [zh, en]() {
        return std::string{I18n::use_chinese ? zh : en};
    }, { .fill = 0xffffffff, .h_justify = Style::Left, .v_justify = Style::Top });
}

static Element *make_paragraph(char const *zh, char const *en) {
    return new Ui::HContainer({
        new Ui::StaticText(14, "-", { .fill = 0xffffffff, .v_justify = Style::Top }),
        new Ui::DynamicText(14, [zh, en]() {
            return std::string{I18n::use_chinese ? zh : en};
        }, { .fill = 0xffffffff, .h_justify = Style::Left, .v_justify = Style::Top })
    }, 0, 10, { .h_justify = Style::Left, .v_justify = Style::Top });
}

static Element *make_entries(std::initializer_list<std::pair<char const *, char const *>> contents) {
    Element *elt = new Ui::VContainer({}, 5, 3, { .h_justify = Style::Left, .v_justify = Style::Top });
    for (auto const &p : contents)
        elt->add_child(make_paragraph(p.first, p.second));
    return elt;
}

static Element *make_changelog_contents() {
    Element *elt = new Ui::VContainer({
        make_date("2026年10月14日", "October 14th 2026"),
        make_entries({
            {"王城 Boss：堕落国王三阶段（冲锋/弹幕/召唤）；王宫常驻刷新",
             "Royal Boss: Fallen King 3-phase (charge/barrage/summons); palace respawn"},
            {"王城 AI：拟态扑击、枪卫突刺、幽影闪现、荆棘弹、盾面减伤、铁卫拦弹",
             "Royal AI: mimic lunge, spear thrust, phantom blink, thorn missiles, shield DR, Iron Guard intercept"},
            {"场内传送：花园/沙漠↔王城，王城北门→火山（站门点自动切图）",
             "Field portals: Garden/Desert↔Royal, Royal north→Volcano"},
            {"王城可进：六区城墙迷宫；标题选图；外城出生；王城怪/瓣 v1",
             "Royal Capital: 6-zone wall mazes; title select; outer spawn; royal mobs/petals v1"},
            {"丛林可玩：密林迷宫/沼河/树根/树心；蜘蛛蜈蚣黄蜂池 → Web·Stinger 线",
             "Jungle live: canopy mazes, swamp rivers, roots, tree-heart; spider/centipede/hornet loot"},
            {"七大地貌全开：花园/沙漠/海洋/雪原/丛林/蚁穴/火山；地形自然围合 Zone",
             "All 7 biomes: Garden/Desert/Ocean/Snow/Jungle/Ant Hell/Volcano; terrain-formed zones"},
            {"地表不再画生态口袋色块；各地貌专属刷怪生态区（花田/珊瑚/蚁后巢等）",
             "No flat eco-pocket paints; per-biome spawn ecosystems (meadow/coral/queen nest…)"},
            {"地图大改：宽廊道螺旋山谷 + 多人刷怪带 / 少人攻坚圈；每图独立布局",
             "Map redesign: wide spiral valleys, farm lanes & push pockets; per-biome layouts"},
            {"设置/暂停：触控操作开关；固定摇杆 + 圆形攻防键",
             "Request Mobile toggle; fixed joystick + circular A/B"},
            {"输入框防 iOS 缩放；标题屏点击槽位交换装备",
             "iOS input zoom fix; title-screen tap-to-swap loadout"},
            {"地形 CCD 防穿墙；装备槽稀有度底色；空服 AI Bot",
             "Terrain CCD; rarity-tinted slots; AI bots fill empty maps"},
            {"移动端放大全部按钮触控区；摇杆避让左下侧栏",
             "Larger mobile touch targets; joystick clears side buttons"},
            {"重生防卡死：拒 NaN 输入、运动自愈、重生防抖清触控",
             "Respawn anti-freeze: reject NaN input, heal motion, debounce+clear touches"},
        }),
        make_divider(),
        make_date("2026年10月12日", "October 12th 2026"),
        make_entries({
            {"进极地等地图不再首帧卡死（地图块分帧烘焙）",
             "Entering Polar/other maps no longer freezes (chunk bake amortized)"},
            {"聊天时可鼠标移动；主键盘与小键盘 Enter 均可聚焦/发送",
             "Chat while mouse-moving; both Enter keys focus/send chat"},
            {"转速：按住 Q/E 线性调速；移动端可拖圆轮；提示圆环联动",
             "Spin: hold Q/E to ramp; drag wheel on mobile; linked hint ring"},
            {"死亡不再掉落花瓣，装备与背包全保留",
             "Death no longer drops petals; loadout and bag kept"},
            {"退回主页软离场：不断 WebSocket，保留花瓣",
             "Soft leave to title: keep WebSocket and inventory"},
            {"地形碰撞贴合锯齿岸线（与绘制共用轮廓）",
             "Terrain collision follows jagged shore outline"},
            {"断线/重部署等待页 + 接花瓣小游戏",
             "Reconnect wait screen with petal-catch mini-game"},
            {"地形锯齿岸线 / 弯曲河流 / 螺旋·方·圆占位（参考 florr_clone-web）",
             "Jagged shores, meander rivers, spiral/box/circle terrain"},
            {"地图改为大正方形，中心固定出生点，外围难度递增",
             "Square arena, fixed center spawn, difficulty rises outward"},
            {"出生区稀疏中立怪（石头 / 瓢虫），便于开局",
             "Sparse neutrals (rocks / ladybugs) around the spawn hub"},
            {"小地图随场地变为正方形",
             "Minimap matches the square arena"},
            {"左下角侧栏改为 IconPark 图标（背包 / 天赋 / 合成 / 特殊）",
             "Side buttons use IconPark icons (bag / talent / craft / special)"},
            {"Nginx 反代：可用 http://IP/ 进入，无需 :9001",
             "Nginx reverse proxy: open http://IP/ without :9001"},
            {"账号数据原子落盘与备份回滚",
             "Atomic account save + backup / restore"},
            {"合成面板：按花瓣五合一、连按加速、全部合成",
             "Craft panel: 5-merge by petal, hold-to-spam, craft-all"},
        }),
        make_divider(),
        make_date("2026年10月", "October 2026"),
        make_entries({
            {"多地图房间与小地图", "Multi-map rooms and minimap"},
            {"地形特征与碰撞", "Terrain features and collision"},
            {"背包 / 天赋 / 合成 / 特殊商店侧栏",
             "Inventory / talents / craft / special shop drawers"},
        }),
        make_divider(),
        make_date("2025年10月20日", "October 20th 2025"),
        make_entries({
            {"新增 1 种花瓣", "Added 1 new petal"},
            {"豌豆改为四向分裂", "Peas now split into 4 orthogonal directions"},
            {"更多 bug 修复", "Even more bugfixes"},
        }),
        make_divider(),
        make_date("2025年8月30日", "August 30th 2025"),
        make_entries({
            {"移动端触控", "Added mobile touch controls"},
            {"中立怪 60 级以上会仇恨队长",
             "Neutral mobs aggro the leader above level 60"},
            {"更多 bug 修复", "More bugfixes"},
        }),
        make_divider(),
        make_date("2025年8月10日", "August 10th 2025"),
        make_entries({
            {"加入 TDM", "Added TDM"},
            {"部分高速怪降速", "Made some faster mobs slower"},
            {"多项 bug 修复", "Various bugfixes"},
        }),
        make_divider(),
        make_date("2025年7月31日", "July 31st 2025"),
        make_entries({
            {"新增 1 种花瓣", "Added 1 new petal"},
            {"首次生成花瓣延迟 2.5s → 1s",
             "Initial petal spawn delay 2.5s → 1s"},
        }),
        make_divider(),
        make_date("2025年6月15日", "June 15th 2025"),
        make_entries({
            {"基础玩法完成", "Finished base game"},
            {"新增 3 种花瓣", "Added 3 new petals"},
            {"掉落率自动平衡", "Drop rates are now autobalanced"},
            {"首次生成花瓣额外 2.5 秒",
             "First petal spawn takes 2.5 extra seconds"},
        }),
        make_divider(),
        make_date("2025年6月11日", "June 11th 2025"),
        make_entries({
            {"新增 1 种怪物", "Added 1 new mob"},
            {"调整部分怪物数值", "Balanced some mob stats"},
            {"重写花瓣描述", "Revamped petal descriptions"},
        }),
        make_divider(),
        make_date("2025年6月7日", "June 7th 2025"),
        make_entries({
            {"完成更新日志", "Finished changelog"},
            {"调整部分花瓣数值", "Tweaked some petal stats"},
            {"加入设置与怪物图鉴", "Added settings and mob gallery"},
        }),
        make_divider(),
        make_date("2025年6月6日", "June 6th 2025"),
        make_entries({
            {"加入更新日志", "Added Changelog"},
            {"新增 2 种花瓣", "Added 2 new petals"},
            {"新增 1 种怪物", "Added 1 new mob"},
        }),
        make_divider(),
        make_date("2025年3月23日", "March 23rd 2025"),
        make_entries({
            {"游戏可玩", "Game is now playable"},
            {"加入刷怪花瓣", "Added spawner petals"},
        }),
        make_divider(),
        new Ui::DynamicText(14, [](){ return I18n::tr(I18n::kOlderChangelog); })
    }, 0, 10, {});
    return new Ui::ScrollContainer(elt, 300);
}

Element *Ui::make_changelog() {
    Element *elt = new Ui::VContainer({
        new Ui::DynamicText(25, [](){ return I18n::tr(I18n::kChangelog); }),
        make_changelog_contents()
    }, 15, 10, {
        .fill = Chrome::kPanelDeep,
        .line_width = 6,
        .round_radius = Chrome::kRadius,
        .animate = [](Element *elt, Renderer &ctx){
            elt->y = Game::alive() ? 170.f : 68.f;
            ctx.translate(0, (elt->animation - 1) * elt->height);
        },
        .should_render = [](){
            if (Ui::panel_open != Panel::kChangelog) return false;
            return Game::should_render_title_ui()
                || (Game::should_render_game_ui() && Game::alive());
        },
        .h_justify = Style::Left,
        .v_justify = Style::Top
    });
    elt->x = 12;
    elt->y = 68;
    Ui::Panel::changelog = elt;
    return elt;
}

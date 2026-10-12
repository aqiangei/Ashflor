#include <Client/Ui/TitleScreen/TitleScreen.hh>

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
        make_date("2026年10月12日", "October 12th 2026"),
        make_entries({
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
        .fill = 0xff5a9fdb,
        .line_width = 7,
        .round_radius = 6,
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

#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Chrome.hh>
#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <Helpers/Math.hh>
#include <Shared/StaticData.hh>

#include <format>
#include <string>

using namespace Ui;

// Leaderboard board: one column per LeaderboardCategory, top rows plus the
// player's own row highlighted at the bottom of every column.
static float const BOARD_WIDTH = 1120;
static float const BOARD_HEIGHT = 320;
static float const BOARD_PAD = 8;
static float const ROW_HEIGHT = 19;
static float const TITLE_SIZE = 22;
static float const SUBTITLE_SIZE = 12;
static float const HEADER_SIZE = 14;
static float const ROW_SIZE = 12;
static float const SELF_ROW_HEIGHT = 22;

// Shared by the title-screen and in-game copies so only one request is sent.
static double last_request_at = -1e9;

static char const *category_title(uint8_t category) {
    switch (category) {
        case kLbPlayTime: return I18n::tr(I18n::kLbPlayTimeTitle);
        case kLbLevel: return I18n::tr(I18n::kLbLevelTitle);
        case kLbMessages: return I18n::tr(I18n::kLbMessagesTitle);
        case kLbKills: return I18n::tr(I18n::kLbKillsTitle);
        case kLbCrafts: return I18n::tr(I18n::kLbCraftsTitle);
        case kLbScore: return I18n::tr(I18n::kLbScoreTitle);
        default: return "";
    }
}

static std::string format_stat_value(uint8_t category, uint64_t value) {
    if (category == kLbPlayTime) {
        uint64_t hours = value / 3600;
        uint64_t minutes = (value % 3600) / 60;
        if (I18n::use_chinese)
            return hours > 0
                ? std::format("{}小时{}分钟", hours, minutes)
                : std::format("{}分钟", minutes);
        return hours > 0
            ? std::format("{}h {}m", hours, minutes)
            : std::format("{}m", minutes);
    }
    if (value >= 10000)
        return std::format("{:.2f}万", value / 10000.0);
    return std::to_string(value);
}

// Trim a name until it fits the given width (canvas text is centre aligned,
// so widths are measured to emulate left/right alignment).
static std::string fit_text(Renderer &ctx, std::string const &text, float max_width, float size) {
    ctx.set_text_size(size);
    if (text.empty()) return text;
    if (ctx.get_text_size(text.c_str()) <= max_width) return text;
    std::string out;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = (unsigned char) text[i];
        size_t len = 1;
        if ((c & 0xf8) == 0xf0) len = 4;
        else if ((c & 0xf0) == 0xe0) len = 3;
        else if ((c & 0xe0) == 0xc0) len = 2;
        if (i + len > text.size()) break;
        out.append(text, i, len);
        i += len;
        if (ctx.get_text_size((out + "…").c_str()) > max_width) {
            out.resize(out.size() - len);
            break;
        }
    }
    return out + "…";
}

// Canvas text is always drawn at the current origin, so shift the transform
// to the wanted spot (and restore it afterwards).
static void draw_text(Renderer &ctx, std::string const &text, float x, float y,
                       float size, uint32_t fill, uint32_t stroke) {
    if (text.empty()) return;
    ctx.set_text_size(size);
    ctx.set_fill(fill);
    ctx.set_stroke(stroke);
    ctx.set_line_width(size * 0.12f);
    ctx.center_text_align();
    ctx.center_text_baseline();
    RenderContext scope(&ctx);
    ctx.translate(x, y);
    ctx.stroke_text(text.c_str());
    ctx.fill_text(text.c_str());
}

// Draw text hugging the left edge of [left, left + max_width]
static void draw_left(Renderer &ctx, std::string const &text, float left, float y,
                       float size, uint32_t fill, uint32_t stroke) {
    if (text.empty()) return;
    ctx.set_text_size(size);
    float w = ctx.get_text_size(text.c_str());
    draw_text(ctx, text, left + w / 2, y, size, fill, stroke);
}

// Draw text hugging the right edge of [right - max_width, right]
static void draw_right(Renderer &ctx, std::string const &text, float right, float y,
                        float size, uint32_t fill, uint32_t stroke) {
    if (text.empty()) return;
    ctx.set_text_size(size);
    float w = ctx.get_text_size(text.c_str());
    draw_text(ctx, text, right - w / 2, y, size, fill, stroke);
}

LeaderboardBoard::LeaderboardBoard() : Element(BOARD_WIDTH, BOARD_HEIGHT) {
    style.fill = Chrome::kPanelDeep;
    style.round_radius = Chrome::kRadius;
    style.line_width = 0;
    style.h_justify = Style::Left;
    style.v_justify = Style::Top;
}

void LeaderboardBoard::on_render(Renderer &ctx) {
    Element::on_render(ctx);

    // Ask the server for a fresh board while the panel is open
    if (Game::timestamp - last_request_at > 3.0) {
        last_request_at = Game::timestamp;
        Game::request_leaderboard_board();
    }

    Game::LeaderboardBoard const &board = Game::leaderboard_board;
    float half_w = BOARD_WIDTH / 2;
    float half_h = BOARD_HEIGHT / 2;
    float col_w = (BOARD_WIDTH - 2 * BOARD_PAD) / (float) kLbCategoryCount;

    // Title + subtitle
    draw_text(ctx, I18n::tr(I18n::kLeaderboardTitle), 0, -half_h + 20,
              TITLE_SIZE, Chrome::kTitle, Chrome::kRimDark);
    std::string subtitle;
    if (board.recorded == 0 && board.tops[0][0].name.empty()) {
        subtitle = I18n::use_chinese ? "加载中…" : "Loading…";
    } else {
        uint32_t online = Game::simulation.arena_info.player_count;
        if (I18n::use_chinese)
            subtitle = std::format("记录 {} · 在线 {}", board.recorded, online);
        else
            subtitle = std::format("{} recorded · {} online", board.recorded, online);
    }
    draw_text(ctx, subtitle, 0, -half_h + 42, SUBTITLE_SIZE, Chrome::kMuted, Chrome::kRimDark);

    float header_y = -half_h + 66;
    float rows_y = -half_h + 84;
    float self_y = -half_h + 284;

    for (uint8_t category = 0; category < kLbCategoryCount; ++category) {
        float col_left = -half_w + BOARD_PAD + col_w * (float) category;
        float col_right = col_left + col_w;
        float cx = (col_left + col_right) / 2;

        // Column header
        draw_text(ctx, category_title(category), cx, header_y, HEADER_SIZE,
                  Chrome::kGold, Chrome::kRimDark);
        // Header underline
        ctx.set_stroke(Chrome::kRimBrass);
        ctx.set_line_width(1.5f);
        ctx.begin_path();
        ctx.move_to(col_left + 4, header_y + 11);
        ctx.line_to(col_right - 4, header_y + 11);
        ctx.stroke();

        // Top rows
        for (uint8_t i = 0; i < LEADERBOARD_STAT_SIZE; ++i) {
            Game::LeaderboardStatRow const &row = board.tops[category][i];
            if (row.name.empty()) continue;
            float y = rows_y + (float) i * ROW_HEIGHT;
            std::string label = "#" + std::to_string(i + 1) + " ";
            label += fit_text(ctx, row.name, col_w * 0.62f, ROW_SIZE);
            std::string value = format_stat_value(category, row.value);
            draw_left(ctx, label, col_left + 4, y, ROW_SIZE, Chrome::kBody, Chrome::kRimDark);
            draw_right(ctx, fit_text(ctx, value, col_w * 0.34f, ROW_SIZE), col_right - 4, y,
                       ROW_SIZE, Chrome::kBody, Chrome::kRimDark);
        }

        // Player's own row (highlighted)
        ctx.set_fill(Chrome::kGold);
        ctx.begin_path();
        ctx.round_rect(col_left + 2, self_y - SELF_ROW_HEIGHT / 2,
                       col_w - 4, SELF_ROW_HEIGHT, 6);
        ctx.fill();
        std::string self_name = board.self_name[category];
        if (self_name.empty()) self_name = Game::nickname;
        std::string label;
        if (board.has_self[category])
            label = "#" + std::to_string(board.self_rank[category]) + " ";
        label += fit_text(ctx, self_name, col_w * 0.62f, ROW_SIZE);
        std::string value = board.has_self[category]
            ? format_stat_value(category, board.self_value[category])
            : "-";
        draw_left(ctx, label, col_left + 6, self_y, ROW_SIZE, Chrome::kRimDark, Chrome::kGold);
        draw_right(ctx, fit_text(ctx, value, col_w * 0.34f, ROW_SIZE), col_right - 6, self_y,
                   ROW_SIZE, Chrome::kRimDark, Chrome::kGold);
    }
}

Element *Ui::make_leaderboard() {
    Element *elt = new Ui::LeaderboardBoard();
    elt->style.animate = [](Element *elt, Renderer &ctx){
        elt->y = Game::alive() ? 170.f : 84.f;
        ctx.translate(0, (elt->animation - 1) * elt->height);
    };
    elt->style.should_render = [](){
        if (Ui::panel_open != Panel::kLeaderboard) return false;
        return Game::should_render_title_ui()
            || (Game::should_render_game_ui() && Game::alive());
    };
    elt->x = 12;
    elt->y = 84;
    // Title copy first; in-game copy tracked as alt for click-outside hit tests
    if (Ui::Panel::leaderboard == nullptr)
        Ui::Panel::leaderboard = elt;
    else
        Ui::Panel::leaderboard_alt = elt;
    return elt;
}

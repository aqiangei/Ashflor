#include <Client/Game.hh>

#include <Client/DOM.hh>
#include <Client/Debug.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Particle.hh>
#include <Client/Setup.hh>
#include <Client/Socket.hh>
#include <Client/Storage.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Ui/TitleScreen/TitleScreen.hh>
#include <Client/Ui/TextInput.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <emscripten.h>

static double g_last_time = 0;
float const MAX_TRANSITION_CIRCLE = 2500;

static int _c = setup_canvas();
static int _i = setup_inputs();

namespace Game {
    Simulation simulation;
    Renderer renderer;
    Renderer game_ui_renderer;
    Socket socket;
    Ui::Window title_ui_window;
    Ui::Window game_ui_window;
    Ui::Window other_ui_window;
    EntityID camera_id;
    EntityID player_id;
    std::string nickname;
    std::string guest_id;
    std::string username;
    std::string password;
    std::string account_status;
    uint32_t daily_bonus_seconds = 0;
    uint32_t offline_bonus_seconds = 0;
    uint8_t bonus_enabled = 0; // off until the player toggles it on
    double bonus_synced_at = 0;
    uint64_t session_token = 0;
    uint8_t pending_auto_login = 0;
    uint8_t spawn_after_auth = 0;
    uint8_t account_logged_in = 0;
    uint8_t auth_panel = 0;
    uint8_t pending_kick_notice = 0;
    std::string disconnect_message;
    std::array<uint8_t, PetalID::kNumPetals> seen_petals;
    std::array<uint8_t, MobID::kNumMobs> seen_mobs;
    std::vector<LifeEntry> life_mob_kills;
    std::vector<LifeEntry> life_petal_picks;
    uint8_t life_summary_ready = 0;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> cached_loadout = {PetalID::kNone};

    double timestamp = 0;

    LeaderboardBoard leaderboard_board;
    double leaderboard_board_at = 0;

    double score = 0;
    float slot_indicator_opacity = 0;
    float transition_circle = 0;
    double spawn_request_at = 0;
    uint8_t spawn_retries = 0;

    uint32_t respawn_level = 1;


    uint8_t loadout_count = 5;
    uint8_t simulation_ready = 0;
    uint8_t on_game_screen = 0;
    uint8_t leaving_to_title = 0;
    uint8_t show_debug = 0;
    uint8_t selected_map = MapID::kGarden;
    uint8_t minimap_expanded = 0;
    std::vector<OnlineRosterEntry> online_roster;
    // >1 = closer / smaller view. Default slightly zoomed-in vs old 1.0.
    float camera_zoom = 1.28f;
    uint8_t pause_menu_open = 0;
    uint8_t leave_confirm_open = 0;
    uint8_t game_panel = 0;
    uint8_t chat_open = 0;
    uint8_t chat_channel = ChatChannel::kChatNearby;
    std::string chat_whisper_to;
    std::string chat_completion_hint;
    std::string chat_draft;
    std::vector<ChatLine> chat_lines;
    uint8_t chat_scroll_to_end = 0;
    std::unordered_map<std::string, SpeechBubble> speech_bubbles;
    ScreenAnnounce screen_announce;
    LootToast loot_toast;
    uint8_t chat_menu_open = 0;
    std::string chat_menu_name;
    float chat_menu_x = 0;
    float chat_menu_y = 0;
    std::string party_status;
    std::vector<PartyMember> party_members;
    uint8_t party_member_count = 0;
    uint8_t revive_progress = 0;
    uint8_t awaiting_party_revive = 0;
    uint8_t invite_open = 0;
    uint8_t invite_type = 0;
    std::string invite_from;
    std::string guild_name;
    std::string guild_status;
    std::vector<GuildMember> guild_members;
    std::string guild_announcement;
    std::string guild_anno_draft;
    uint8_t guild_member_count = 0;
    uint8_t guild_panel_open = 0;
    std::string guild_create_draft;
    std::array<uint32_t, PetalID::kNumPetals> craft_bank = {};
    uint32_t coins = 0;
    uint8_t guest_save_valid = 0;
    uint32_t guest_level = 1;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> guest_petals = {PetalID::kNone};
    std::array<uint32_t, PetalID::kNumPetals> guest_craft_bank = {};
    uint32_t guest_coins = 0;
    std::array<uint8_t, TalentID::kNum> guest_talent_ranks = {};
    std::array<uint8_t, TalentID::kNum> talent_ranks = {};
    uint8_t talent_pending = 0;
    uint8_t talent_pending_id = 0;
    uint8_t talent_pending_rank = 0;
    std::string talent_status;
    std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> loadout_presets = {};
    uint8_t active_loadout_preset = LOADOUT_PRESET_COUNT;
    std::string loadout_preset_status;
    uint8_t mobile_l_preset_pick = 0;
    uint8_t preset_ghost_bank = 0;
    uint8_t rot_speed_setting = 7;
    float rot_speed_f = 7.f;
    float rot_hint_opacity = 0.f;
    double rot_hint_stamp = 0;
    uint8_t craft_selected_rarity = 0;
    PetalID::T craft_tray_id = PetalID::kNone;
    uint32_t craft_tray_count = 0;
    std::string craft_status;
}

using namespace Game;

void Game::nudge_rot_speed(float delta) {
    rot_speed_f = fclamp(rot_speed_f + delta, 0.f, (float) Talent::kRotSettingMax);
    rot_speed_setting = (uint8_t) lroundf(rot_speed_f);
    if (rot_speed_setting > Talent::kRotSettingMax)
        rot_speed_setting = Talent::kRotSettingMax;
    rot_hint_opacity = 1.f;
    rot_hint_stamp = timestamp;
}

void Game::set_rot_speed_from_frac(float frac01) {
    frac01 = fclamp(frac01, 0.f, 1.f);
    rot_speed_f = frac01 * (float) Talent::kRotSettingMax;
    rot_speed_setting = (uint8_t) lroundf(rot_speed_f);
    if (rot_speed_setting > Talent::kRotSettingMax)
        rot_speed_setting = Talent::kRotSettingMax;
    rot_hint_opacity = 1.f;
    rot_hint_stamp = timestamp;
}

static uint8_t guest_id_char_ok(char c) {
    if (c >= 'A' && c <= 'Z') return (c != 'I' && c != 'O') ? 1 : 0;
    if (c >= '2' && c <= '9') return 1;
    return 0;
}

static uint8_t guest_id_valid(std::string const &id) {
    if (id.size() != 8) return 0; // G# + 6
    if (id[0] != 'G' || id[1] != '#') return 0;
    for (size_t i = 2; i < id.size(); ++i)
        if (!guest_id_char_ok(id[i])) return 0;
    return 1;
}

static std::string make_guest_id() {
    static char const alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    constexpr uint32_t n = sizeof(alphabet) - 1;
    std::string id = "G#";
    for (int i = 0; i < 6; ++i) {
        uint32_t r = (uint32_t) EM_ASM_INT({ return (Math.random() * 0x7fffffff) | 0; });
        id.push_back(alphabet[r % n]);
    }
    return id;
}

void Game::persist_camera_zoom() {
    StorageProtocol::Encoder writer(&StorageProtocol::buffer[0]);
    uint16_t z = (uint16_t) (camera_zoom * 100.f + 0.5f);
    if (z < 55) z = 55;
    if (z > 220) z = 220;
    writer.write<uint16_t>(z);
    StorageProtocol::store("cam_zoom", (uint32_t) (writer.at - writer.base));
}

void Game::ensure_guest_id() {
    if (!guest_id_valid(guest_id))
        guest_id = make_guest_id();
    if (!account_logged_in)
        nickname = guest_id;
}

std::string const &Game::guest_play_name() {
    ensure_guest_id();
    return guest_id;
}

void Game::init() {
    Input::is_mobile = check_mobile();
    Input::request_mobile = Input::is_mobile; // overridden by Storage if saved
    DOM::cache_client();
    Storage::retrieve();
    reset();
    // Saved username+password → Socket auto-login after verify (see pending_guest_progress).
    // Brand lives inside make_title_input_box so it cannot overlap the form
    title_ui_window.add_child(
        Ui::make_title_input_box()
    );
    title_ui_window.add_child(
        Ui::make_title_info_box()
    );
    title_ui_window.add_child(
        Ui::make_panel_buttons()
    );
    title_ui_window.add_child(
        Ui::make_settings_panel()
    );
    title_ui_window.add_child(
        Ui::make_petal_gallery()
    );
    title_ui_window.add_child(
        Ui::make_mob_gallery()
    );
    title_ui_window.add_child(
        Ui::make_changelog()
    );
    title_ui_window.add_child(
        Ui::make_leaderboard()
    );
    title_ui_window.add_child(
        Ui::make_github_link_button()
    );
    game_ui_window.add_child(
        Ui::make_death_main_screen()
    );
    game_ui_window.add_child(
        Ui::make_level_bar()
    );
    game_ui_window.add_child(
        Ui::make_health_bar()
    );
    game_ui_window.add_child(
        Ui::make_boss_health_bar()
    );
    game_ui_window.add_child(
        Ui::make_server_announce_banner()
    );
    game_ui_window.add_child(
        Ui::make_loot_toast()
    );
    game_ui_window.add_child(
        Ui::make_minimap()
    );
    game_ui_window.add_child(
        Ui::make_loadout_backgrounds()
    );
    game_ui_window.add_child(
        Ui::make_mobile_joystick()
    );
    game_ui_window.add_child(
        Ui::make_mobile_attack_button()
    );
    game_ui_window.add_child(
        Ui::make_mobile_defend_button()
    );
    game_ui_window.add_child(
        Ui::make_mobile_swap_button()
    );
    for (uint8_t i = 0; i < MAX_SLOT_COUNT * 2; ++i) game_ui_window.add_child(new Ui::UiLoadoutPetal(i));
    game_ui_window.add_child(Ui::make_bag_drag_ghost());
    game_ui_window.add_child(Ui::make_craft_fx_layer());
    game_ui_window.add_child(
        Ui::make_stat_screen()
    );
    game_ui_window.add_child(
        Ui::make_death_life_summary()
    );
    game_ui_window.add_child(
        Ui::make_game_topbar()
    );
    // In-game copies of title panels (title window is skipped while playing)
    game_ui_window.add_child(
        Ui::make_leaderboard()
    );
    game_ui_window.add_child(
        Ui::make_mob_gallery()
    );
    game_ui_window.add_child(
        Ui::make_petal_gallery()
    );
    game_ui_window.add_child(
        Ui::make_pause_menu()
    );
    game_ui_window.add_child(
        Ui::make_leave_confirm()
    );
    game_ui_window.add_child(
        Ui::make_chat_panel()
    );
    game_ui_window.add_child(
        Ui::make_chat_context_menu()
    );
    game_ui_window.add_child(
        Ui::make_invite_toast()
    );
    game_ui_window.add_child(
        Ui::make_guild_panel()
    );
    game_ui_window.add_child(
        Ui::make_game_side_buttons()
    );
    game_ui_window.add_child(
        Ui::make_inventory_panel()
    );
    game_ui_window.add_child(
        Ui::make_presets_panel()
    );
    game_ui_window.add_child(
        Ui::make_talents_panel()
    );
    game_ui_window.add_child(
        Ui::make_craft_panel()
    );
    game_ui_window.add_child(
        Ui::make_special_craft_panel()
    );
    game_ui_window.add_child(
        Ui::make_shop_panel()
    );
    game_ui_window.add_child(
        Ui::make_boost_panel()
    );
    Ui::make_petal_tooltips();
    Ui::make_mob_tooltips();
    other_ui_window.add_child(
        Ui::make_debug_stats()
    );
    other_ui_window.add_child(
        [](){ 
            Ui::Element *elt = new Ui::HContainer({
                new Ui::DynamicText(16, [](){ return Game::disconnect_message; })
            }, 5, 5, { 
                .fill = 0x40000000,
                .round_radius = 5,
                .should_render = [](){
                    return !Game::socket.ready && Game::disconnect_message != "";
                },
                .v_justify = Ui::Style::Top
            });
            elt->y = 50;
            return elt;
        }()
    );
    other_ui_window.style.no_polling = 1;
    // Follow page host so FRP / remote access works (ws/wss + host:port)
    char *dyn_url = (char *) EM_ASM_PTR({
        var proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
        return stringToNewUTF8(proto + '//' + location.host);
    });
    std::string ws_url = dyn_url ? std::string(dyn_url) : std::string(WS_URL);
    free(dyn_url);
    // ?v=VERSION_HASH lets nginx map sticky traffic during zero-downtime deploys
    ws_url += "?v=" + std::to_string(VERSION_HASH);
    socket.connect(ws_url);
}

void Game::reset() {
    simulation_ready = 0;
    on_game_screen = 0;
    leaving_to_title = 0;
    pause_menu_open = 0;
    leave_confirm_open = 0;
    game_panel = 0;
    chat_open = 0;
    chat_scroll_to_end = 0;
    chat_channel = ChatChannel::kChatNearby;
    chat_whisper_to.clear();
    chat_draft.clear();
    chat_lines.clear();
    screen_announce = {};
    loot_toast = {};
    chat_menu_open = 0;
    chat_menu_name.clear();
    party_status.clear();
    party_members.clear();
    party_member_count = 0;
    revive_progress = 0;
    awaiting_party_revive = 0;
    invite_open = 0;
    invite_type = 0;
    invite_from.clear();
    guild_name.clear();
    guild_status.clear();
    guild_members.clear();
    guild_announcement.clear();
    guild_anno_draft.clear();
    guild_member_count = 0;
    guild_panel_open = 0;
    guild_create_draft.clear();
    account_logged_in = 0;
    daily_bonus_seconds = 0;
    offline_bonus_seconds = 0;
    bonus_enabled = 0;
    bonus_synced_at = 0;
    pending_auto_login = 0;
    spawn_after_auth = 0;
    // Keep kick notice across reconnect so it shows where wrong-password does.
    if (pending_kick_notice) {
        account_status = I18n::tr(I18n::kAuthAlreadyIn);
        auth_panel = 1;
    } else {
        account_status.clear();
        auth_panel = 0;
    }
    score = 0;
    slot_indicator_opacity = 0;
    transition_circle = 0;
    respawn_level = guest_save_valid ? guest_level : 1;
    loadout_count = 5;
    camera_id = player_id = NULL_ENTITY;
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        cached_loadout[i] = PetalID::kNone;
    if (guest_save_valid) {
        craft_bank = guest_craft_bank;
        coins = guest_coins;
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            talent_ranks[i] = guest_talent_ranks[i];
    } else {
        craft_bank = {};
        coins = 0;
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            talent_ranks[i] = 0;
    }
    talent_pending = 0;
    talent_status.clear();
    loadout_presets = {};
    active_loadout_preset = LOADOUT_PRESET_COUNT;
    loadout_preset_status.clear();
    mobile_l_preset_pick = 0;
    preset_ghost_bank = 0;
    craft_tray_id = PetalID::kNone;
    craft_tray_count = 0;
    simulation.reset();
}

void Game::send_guest_progress() {
    if (!socket.ready || account_logged_in) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kGuestProgress);
    writer.write<uint8_t>(guest_save_valid ? 1 : 0);
    if (!guest_save_valid) {
        socket.send(writer.packet, writer.at - writer.packet);
        return;
    }
    writer.write<uint32_t>(guest_level < 1 ? 1 : guest_level);
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        writer.write<PetalID::T>(guest_petals[i]);
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
        writer.write<uint32_t>(guest_craft_bank[i]);
    writer.write<uint32_t>(guest_coins);
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        writer.write<uint8_t>(guest_talent_ranks[i]);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::seed_guest_from_session() {
    guest_craft_bank = craft_bank;
    guest_coins = coins;
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        guest_talent_ranks[i] = talent_ranks[i];
    uint32_t lv = respawn_level < 1 ? 1 : respawn_level;
    if (alive()) {
        uint32_t from_score = score_to_level(score);
        if (from_score < 1) from_score = 1;
        if (from_score > lv) lv = from_score;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            guest_petals[i] = cached_loadout[i];
    } else if (simulation.ent_exists(camera_id)) {
        // Dead / title: camera inventory preserves slot layout (primary + backpack).
        Entity const &camera = simulation.get_ent(camera_id);
        uint32_t cam_lv = camera.get_respawn_level();
        if (cam_lv < 1) cam_lv = 1;
        if (cam_lv > lv) lv = cam_lv;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            guest_petals[i] = camera.get_inventory(i);
    }
    // High-water mark — never let a snapshot drop stored guest level.
    if (lv > guest_level || !guest_save_valid) guest_level = lv;
    if (guest_level < 1) guest_level = 1;
    guest_save_valid = 1;
}

void Game::sync_guest_progress() {
    if (account_logged_in) return;
    seed_guest_from_session();
}

void Game::close_game_panel() {
    game_panel = kPanelNone;
}

uint8_t Game::game_panel_blocking() {
    return game_panel != kPanelNone;
}

void Game::toggle_game_panel(uint8_t panel) {
    // Death screen: bag / talents / craft / recipes / presets stay usable
    uint8_t const death_panel = panel == kPanelInventory
        || panel == kPanelPresets || panel == kPanelTalents
        || panel == kPanelCraft || panel == kPanelSpecial
        || panel == kPanelShop;
    if ((!alive() && !(on_game_screen && death_panel))
        || leave_confirm_open) return;
    if (chat_open) close_chat();
    if (pause_menu_open) pause_menu_open = 0;
    if (game_panel == panel) {
        game_panel = kPanelNone;
        return;
    }
    game_panel = panel;
    Input::keys_held.clear();
    Input::game_inputs.x = 0;
    Input::game_inputs.y = 0;
    Input::game_inputs.flags = 0;
}

void Game::open_chat() {
    if (!alive() || pause_menu_open || leave_confirm_open) return;
    close_game_panel();
    chat_open = 1;
    chat_scroll_to_end = 1;
    // Drop held letter keys so they don't leak into binds / chat draft.
    // Mobile: also stop the flower while the soft keyboard is up.
    Input::keys_held.clear();
    Input::keys_held_this_tick.clear();
    if (Input::mobile_controls()) {
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        Input::game_inputs.flags = 0;
    }
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->focus();
}

void Game::close_chat() {
    if (!chat_open) return;
    chat_open = 0;
    chat_completion_hint.clear();
    close_chat_menu();
    // Clear DOM before blur — blur() syncs from the overlay and would otherwise
    // put the just-sent message back into chat_draft.
    if (Ui::TextInput *input = Ui::chat_text_input()) {
        input->clear();
        input->blur();
    } else {
        chat_draft.clear();
    }
}

void Game::push_chat_line(uint8_t channel, std::string const &sender, std::string const &text,
                          std::string const &whisper_to) {
    if (text.empty() && sender.empty()) return;
    chat_lines.push_back({ channel, sender, whisper_to, text, timestamp });
    while (chat_lines.size() > CHAT_HISTORY)
        chat_lines.erase(chat_lines.begin());
    if (chat_open) chat_scroll_to_end = 1;
    // Overhead bubble above speaker (~8s); skip system lines
    if (!sender.empty() && !text.empty() && channel != ChatChannel::kChatSystem)
        speech_bubbles[sender] = { text, timestamp + 8000 };
    // Admin account aqa → soft top banner for every in-game client (like boss HP).
    if (channel == ChatChannel::kChatServer && sender == "aqa" && !text.empty()) {
        screen_announce.sender = sender;
        screen_announce.text = text;
        screen_announce.expires_at = timestamp + SCREEN_ANNOUNCE_MS;
    }
}

void Game::cycle_chat_channel() {
    if (!chat_open) return;
    // Nearby → Party → Guild → Server → Whisper(if set) → Nearby
    if (chat_channel == ChatChannel::kChatNearby)
        chat_channel = ChatChannel::kChatParty;
    else if (chat_channel == ChatChannel::kChatParty)
        chat_channel = ChatChannel::kChatGuild;
    else if (chat_channel == ChatChannel::kChatGuild)
        chat_channel = ChatChannel::kChatServer;
    else if (chat_channel == ChatChannel::kChatServer) {
        if (!chat_whisper_to.empty())
            chat_channel = ChatChannel::kChatWhisper;
        else
            chat_channel = ChatChannel::kChatNearby;
    } else
        chat_channel = ChatChannel::kChatNearby;
}

// Slash commands offered for Tab complete (short forms first).
static char const *const kChatSlashCmds[] = {
    "/w", "/whisper", "/密语", "/私聊",
    "/r", "/reply", "/回复",
    "/local", "/附近",
    "/squad", "/party", "/组队聊",
    "/guild", "/公会",
    "/server", "/全服",
    "/invite", "/组队",
    "/leave", "/离队",
    "/guild-create", "/创建公会",
    "/guild-invite", "/公会邀请",
    "/guild-leave", "/退会",
    "/guild-motd", "/公告",
    "/help", "/帮助",
};

static uint8_t chat_cmd_needs_name(std::string const &cmd) {
    return cmd == "/w" || cmd == "/whisper" || cmd == "/密语" || cmd == "/私聊"
        || cmd == "/invite" || cmd == "/组队"
        || cmd == "/guild-invite" || cmd == "/公会邀请";
}

static void collect_chat_name_candidates(std::vector<std::string> &out) {
    auto add = [&](std::string const &n) {
        if (n.empty()) return;
        for (auto const &x : out) if (x == n) return;
        out.push_back(n);
    };
    if (!chat_whisper_to.empty()) add(chat_whisper_to);
    for (auto const &m : party_members) add(m.name);
    for (auto const &line : chat_lines) {
        add(line.sender);
        add(line.whisper_to);
    }
}

static std::vector<std::string> chat_completion_matches(std::string const &draft) {
    std::vector<std::string> matches;
    if (draft.empty() || draft.front() != '/') return matches;
    size_t sp = draft.find_first_of(" \t");
    std::string head = sp == std::string::npos ? draft : draft.substr(0, sp);
    std::string rest = sp == std::string::npos ? std::string{} : draft.substr(sp + 1);
    while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t'))
        rest.erase(rest.begin());

    if (sp == std::string::npos) {
        // Completing the command itself
        for (char const *cmd : kChatSlashCmds) {
            if (std::strncmp(cmd, head.c_str(), head.size()) == 0)
                matches.push_back(cmd);
        }
        return matches;
    }

    // Completing a player name after /w|/invite|…
    if (!chat_cmd_needs_name(head)) return matches;
    size_t sp2 = rest.find_first_of(" \t");
    if (sp2 != std::string::npos) return matches; // already past the name
    std::string prefix = rest;
    std::vector<std::string> names;
    collect_chat_name_candidates(names);
    for (auto const &n : names) {
        if (prefix.empty() || n.compare(0, prefix.size(), prefix) == 0)
            matches.push_back(head + " " + n);
    }
    return matches;
}

static uint32_t g_chat_tab_cycle = 0;

uint8_t Game::try_chat_tab_complete() {
    if (!chat_open) return 0;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->sync_from_dom();
    std::vector<std::string> matches = chat_completion_matches(chat_draft);
    if (matches.empty()) return 0;
    uint32_t idx = g_chat_tab_cycle % (uint32_t) matches.size();
    ++g_chat_tab_cycle;
    std::string filled = matches[idx];
    uint8_t has_name = filled.find(' ') != std::string::npos;
    uint8_t bare_no_args = filled == "/help" || filled == "/帮助"
        || filled == "/leave" || filled == "/离队"
        || filled == "/guild-leave" || filled == "/退会";
    // Trailing space: ready to type args / message (except bare no-arg cmds)
    if (!bare_no_args && (filled.empty() || filled.back() != ' '))
        filled.push_back(' ');
    (void) has_name;
    chat_draft = filled;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->set_text(chat_draft);
    refresh_chat_completion_hint();
    return 1;
}

void Game::refresh_chat_completion_hint() {
    chat_completion_hint.clear();
    if (!chat_open) return;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->sync_from_dom();
    static std::string last_draft_for_cycle;
    if (chat_draft != last_draft_for_cycle) {
        // User typed — reset cycle so Tab starts from the first match again
        if (chat_draft.size() < last_draft_for_cycle.size()
            || last_draft_for_cycle.empty()
            || chat_draft.compare(0, last_draft_for_cycle.size(), last_draft_for_cycle) != 0)
            g_chat_tab_cycle = 0;
        last_draft_for_cycle = chat_draft;
    }
    std::vector<std::string> matches = chat_completion_matches(chat_draft);
    if (matches.empty()) return;
    std::string best = matches[g_chat_tab_cycle % matches.size()];
    for (auto const &m : matches) {
        if (m.size() > chat_draft.size()
            && m.compare(0, chat_draft.size(), chat_draft) == 0) {
            best = m;
            break;
        }
    }
    if (best.size() <= chat_draft.size()) {
        if (matches.size() > 1) {
            chat_completion_hint = I18n::use_chinese
                ? std::format("Tab 切换（{}/{}）· {}",
                    (g_chat_tab_cycle % matches.size()) + 1, matches.size(), best)
                : std::format("Tab cycle ({}/{}) · {}",
                    (g_chat_tab_cycle % matches.size()) + 1, matches.size(), best);
        }
        return;
    }
    chat_completion_hint = I18n::use_chinese
        ? std::format("Tab → {}", best)
        : std::format("Tab → {}", best);
}

void Game::open_chat_menu(std::string const &name, float x, float y) {
    if (name.empty()) return;
    chat_menu_open = 1;
    chat_menu_name = name;
    chat_menu_x = x;
    chat_menu_y = y;
}

void Game::close_chat_menu() {
    chat_menu_open = 0;
    chat_menu_name.clear();
}

void Game::chat_menu_invite() {
    if (chat_menu_name.empty()) return;
    std::string name = chat_menu_name;
    close_chat_menu();
    party_invite_name(name);
}

void Game::chat_menu_invite_guild() {
    if (chat_menu_name.empty()) return;
    std::string name = chat_menu_name;
    close_chat_menu();
    guild_invite_name(name);
}

void Game::chat_menu_whisper() {
    if (chat_menu_name.empty()) return;
    chat_whisper_to = chat_menu_name;
    chat_channel = ChatChannel::kChatWhisper;
    close_chat_menu();
    if (!chat_open) open_chat();
}

void Game::invite_respond(uint8_t accept) {
    if (!invite_open || invite_from.empty()) return;
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kInviteRespond);
    writer.write<uint8_t>(invite_type);
    writer.write<uint8_t>(accept ? 1 : 0);
    writer.write<std::string>(invite_from);
    socket.send(writer.packet, writer.at - writer.packet);
    invite_open = 0;
    invite_from.clear();
}

void Game::invite_dismiss() {
    invite_open = 0;
    invite_from.clear();
}

void Game::toggle_guild_panel() {
    if (!alive()) return;
    guild_panel_open = !guild_panel_open;
    if (guild_panel_open) {
        pause_menu_open = 0;
        close_game_panel();
        close_chat_menu();
    }
}

void Game::close_guild_panel() {
    guild_panel_open = 0;
}

void Game::guild_create() {
    if (!socket.ready || !alive()) return;
    if (guild_create_draft.empty()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kGuildCreate);
    writer.write<std::string>(guild_create_draft);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::guild_invite_name(std::string const &name) {
    if (!socket.ready || !alive()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kGuildInvite);
    writer.write<std::string>(name);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::guild_leave() {
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kGuildLeave);
    socket.send(writer.packet, writer.at - writer.packet);
}

uint8_t Game::guild_self_is_leader() {
    for (GuildMember const &m : guild_members) {
        if ((m.flags & 4) && (m.flags & 1)) return 1;
    }
    return 0;
}

void Game::guild_set_announcement() {
    if (!socket.ready || !alive()) return;
    if (!guild_self_is_leader()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kGuildSetAnno);
    writer.write<std::string>(guild_anno_draft);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::toggle_pause() {
    if (!alive()) return;
    if (leave_confirm_open) {
        leave_confirm_open = 0;
        return;
    }
    if (chat_open) close_chat();
    if (guild_panel_open) {
        close_guild_panel();
        return;
    }
    if (game_panel != kPanelNone) {
        close_game_panel();
        return;
    }
    pause_menu_open = !pause_menu_open;
    if (pause_menu_open) {
        Input::keys_held.clear();
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        Input::game_inputs.flags = 0;
    }
}

void Game::return_to_title() {
    close_game_panel();
    close_guild_panel();
    close_chat();
    invite_open = 0;
    invite_from.clear();
    pause_menu_open = 0;
    leave_confirm_open = 0;
    leaving_to_title = 1;
    on_game_screen = 0;
    awaiting_party_revive = 0;
    revive_progress = 0;
    // Keep camera_zoom — recalled from cam_zoom localStorage across sessions
    speech_bubbles.clear();
    // Drop bonus is match-scoped — always off on the title screen.
    if (bonus_enabled)
        set_bonus_enabled(0);
    // Keep WebSocket — soft-despawn flower only (was close 1000 "leave")
    if (socket.ready) {
        Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
        writer.write<uint8_t>(Serverbound::kClientLeave);
        socket.send(writer.packet, writer.at - writer.packet);
    }
}

uint8_t Game::alive() {
    return socket.ready && simulation_ready
    && simulation.ent_exists(camera_id)
    && simulation.ent_alive(simulation.get_ent(camera_id).get_player());
}

uint8_t Game::in_game() {
    return simulation_ready && on_game_screen
    && simulation.ent_exists(camera_id);
}

uint8_t Game::should_render_title_ui() {
    return transition_circle < MAX_TRANSITION_CIRCLE;
}

uint8_t Game::should_render_game_ui() {
    return transition_circle > 0 && simulation_ready && simulation.ent_exists(camera_id);
}

void Game::poll_ui_event(Ui::ScreenEvent const &event) {
    Ui::focused = nullptr;
    if (Game::should_render_title_ui())
        title_ui_window.poll_events(event);
    if (Game::should_render_game_ui())
        game_ui_window.poll_events(event);
    other_ui_window.poll_events(event);
    if (Ui::focused != nullptr)
        Ui::focused->focused = 1;
}

void Game::tick(double time) {
    double tick_start = Debug::get_timestamp();
    Game::timestamp = time;
    Ui::dt = time - g_last_time;
    // Slightly snappier entity lerp (was 0.24); still smooth under variable dt
    // Higher base → less visual lag behind server snapshots (was 0.28)
    Ui::lerp_amount = 1 - pow(1 - 0.42, Ui::dt * 60 / 1000);
    g_last_time = time;
    simulation.tick();
    
    renderer.reset();
    game_ui_renderer.set_dimensions(renderer.width, renderer.height);
    game_ui_renderer.reset();
    game_ui_renderer.clear();

    Ui::window_width = renderer.width;
    Ui::window_height = renderer.height;
    Ui::focused = nullptr;
    double a = Ui::window_width / 1920;
    double b = Ui::window_height / 1080;
    Ui::scale = std::max(a, b);
    if (alive()) {
        // Soft-leave must keep on_game_screen=0 until flower despawns; otherwise
        // death UI appears and Enter/"Continue" looks like an instant revive.
        if (!leaving_to_title)
            on_game_screen = 1;
        if (life_summary_ready) {
            life_summary_ready = 0;
            life_mob_kills.clear();
            life_petal_picks.clear();
        }
        player_id = simulation.get_ent(camera_id).get_player();
        Entity const &player = simulation.get_ent(player_id);
        Game::loadout_count = player.get_loadout_count();
        // Keep optimistic swaps until the server snapshot catches up — otherwise
        // every frame stomps bag/slot UI mid-drag (esp. mobile tap-to-equip).
        for (uint32_t i = 0; i < MAX_SLOT_COUNT + Game::loadout_count
            && i < 2 * MAX_SLOT_COUNT; ++i) {
            PetalID::T server_id = player.get_loadout_ids(i);
            Ui::UiLoadoutPetal *slot = Ui::UiLoadout::petal_slots[i];
            if (slot != nullptr && slot->no_change_ticks > 0) {
                if (server_id == cached_loadout[i]) {
                    slot->no_change_ticks = 0;
                } else if (--slot->no_change_ticks == 0) {
                    cached_loadout[i] = server_id;
                }
                // Do NOT treat "cached empty, server still has petal" as a pickup.
                // Optimistic move-to-empty clears the source slot first; restoring
                // here made drag/click into empty primary/secondary no-ops.
            } else {
                cached_loadout[i] = server_id;
            }
            if (cached_loadout[i] < PetalID::kNumPetals)
                Game::seen_petals[cached_loadout[i]] = 1;
        }
        score = player.get_score();
    } else {
        player_id = NULL_ENTITY;
        pause_menu_open = 0;
        leave_confirm_open = 0;
        // Dead / title: clear movement so mobile joystick / attack can't leak
        // NaN-ish or stuck inputs into the next spawn packet.
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        Input::game_inputs.flags = 0;
        // Sync camera inventory so death/title bag + loadout edit stay accurate
        if (simulation.ent_exists(camera_id)) {
            Entity const &camera = simulation.get_ent(camera_id);
            Game::loadout_count = Talent::slot_count_from_ranks(Game::talent_ranks);
            for (uint32_t i = 0; i < MAX_SLOT_COUNT + Game::loadout_count
                && i < 2 * MAX_SLOT_COUNT; ++i) {
                PetalID::T server_id = camera.get_inventory(i);
                Ui::UiLoadoutPetal *slot = Ui::UiLoadout::petal_slots[i];
                // Dead UI does not run LoadoutPetal should_render — tick down here
                if (slot != nullptr && slot->no_change_ticks > 0) {
                    if (server_id == cached_loadout[i]) {
                        slot->no_change_ticks = 0;
                    } else if (--slot->no_change_ticks == 0) {
                        cached_loadout[i] = server_id;
                    }
                } else {
                    cached_loadout[i] = server_id;
                }
                if (cached_loadout[i] < PetalID::kNumPetals)
                    Game::seen_petals[cached_loadout[i]] = 1;
            }
        }
        if (leaving_to_title) {
            on_game_screen = 0;
            leaving_to_title = 0; // flower gone — title UI; death Continue stays hidden via on_game_screen
        }
        if (chat_open) close_chat();
    }

    // chat_lines kept for focused history; collapsed UI fades via created_at
    for (auto it = speech_bubbles.begin(); it != speech_bubbles.end(); ) {
        if (it->second.expires_at <= timestamp)
            it = speech_bubbles.erase(it);
        else
            ++it;
    }

    //event poll
    // Mobile: deliver every active touch every frame so the element that already
    // captured it can keep `focused`. New captures are gated on !saturated inside
    // Element::poll_events — skipping saturated here made joystick/A/B lose focus
    // after one frame and zeroed movement.
    if (Input::is_mobile) {
        // A touch handler may spawn/respawn and clear Input::touches. Iterate
        // IDs separately so that clearing the map cannot invalidate this loop.
        std::vector<uint32_t> touch_ids;
        touch_ids.reserve(Input::touches.size());
        for (auto const &[id, touch] : Input::touches) touch_ids.push_back(id);
        for (uint32_t id : touch_ids) {
            auto it = Input::touches.find(id);
            if (it == Input::touches.end()) continue;
            Input::Touch const touch = it->second;
            // Fresh taps outside the active field dismiss keyboard (canvas
            // preventDefault otherwise keeps the HTML <input> focused).
            if (!touch.saturated)
                Ui::dismiss_active_text_input_at(touch.x, touch.y);
            Game::poll_ui_event({ .id = touch.id, .x = touch.x, .y = touch.y, .press = 1 });
        }
    }
    else {
        if (BitMath::at(Input::mouse_buttons_pressed, Input::LeftMouse))
            Ui::dismiss_active_text_input_at(Input::mouse_x, Input::mouse_y);
        Game::poll_ui_event({ .id = 0, .x = Input::mouse_x, .y = Input::mouse_y, .press = 0 });
    }

    if (in_game())
        transition_circle = fclamp(transition_circle * powf(1.05, Ui::dt * 60 / 1000) + Ui::dt / 5, 0, MAX_TRANSITION_CIRCLE);
    else 
        transition_circle = fclamp(transition_circle / powf(1.05, Ui::dt * 60 / 1000) - Ui::dt / 5, 0, MAX_TRANSITION_CIRCLE);

    if (should_render_title_ui()) {
        render_title_screen();
        Particle::tick_title(renderer, Ui::dt);
        title_ui_window.render(renderer);
    } else
        title_ui_window.on_render_skip(renderer);

    if (should_render_game_ui()) {
        RenderContext c(&renderer);
        if (should_render_title_ui()) {
            renderer.set_stroke(0xff222222);
            renderer.set_line_width(Ui::scale * 10);
            renderer.begin_path();
            renderer.arc(renderer.width / 2, renderer.height / 2, transition_circle);
            renderer.stroke();
            renderer.clip();
        }
        render_game();
        if (!Game::alive()) {
            RenderContext c(&renderer);
            renderer.reset_transform();
            renderer.set_fill(0x20000000);
            renderer.fill_rect(0,0,renderer.width,renderer.height);
        }
        game_ui_window.render(game_ui_renderer);
        renderer.set_global_alpha(0.85);
        renderer.translate(renderer.width/2,renderer.height/2);
        renderer.draw_image(game_ui_renderer);
    // Hold K/L: ghost first petal of presets 1..10 / 11..20 on primary slots (visual only).
    // Plain number keys swap primary[i] ↔ secondary[i]. Apply presets via panel / mobile L-pick.
    // Q/E = hold for continuous linear spin rate (not discrete taps).
    if (!Game::pause_menu_open && !Game::chat_open && !Game::leave_confirm_open
            && !Game::game_panel_blocking()) {
            float const rot_rate = 4.5f; // levels / second while held
            float dt_s = (float) (Ui::dt / 1000.0);
            if (dt_s > 0.05f) dt_s = 0.05f;
            if (Input::keys_held.contains('Q'))
                Game::nudge_rot_speed(-rot_rate * dt_s);
            if (Input::keys_held.contains('E'))
                Game::nudge_rot_speed(+rot_rate * dt_s);
            if (Input::keys_held_this_tick.contains('R'))
                Game::swap_all_petals();
            // Hold K / L: ghost-mark primary slots with that bank's preset first petal.
            // Digits do NOT apply presets (apply only via panel / mobile L-pick).
            bool const k_held = Input::keys_held.contains('K');
            bool const l_held = Input::keys_held.contains('L');
            Game::preset_ghost_bank = k_held ? 1 : (l_held ? 2 : 0);
            if (!k_held && !l_held) {
                for (uint8_t i = 0; i < Game::loadout_count; ++i) {
                    if (!Input::keys_held_this_tick.contains(SLOT_KEYBINDS[i])) continue;
                    uint8_t sec = (uint8_t) (Game::loadout_count + i);
                    if (Game::cached_loadout[i] == PetalID::kNone
                        && Game::cached_loadout[sec] == PetalID::kNone)
                        continue;
                    // Same cross-fly as click / R-swap-all
                    Ui::ui_swap_paired_slots(i, sec);
                    break;
                }
            }
            // T = unequip selected secondary (or primary if keyed) → backpack
            if (Input::keys_held_this_tick.contains('T')
                && Ui::UiLoadout::selected_with_keys < MAX_SLOT_COUNT) {
                uint8_t sec = (uint8_t) (Game::loadout_count + Ui::UiLoadout::selected_with_keys);
                if (Game::cached_loadout[sec] != PetalID::kNone)
                    Ui::ui_unequip_to_backpack(sec);
                else if (Ui::UiLoadout::selected_with_keys < Game::loadout_count
                    && Game::cached_loadout[Ui::UiLoadout::selected_with_keys] != PetalID::kNone)
                    Ui::ui_unequip_to_backpack(Ui::UiLoadout::selected_with_keys);
                Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
            }
        }
    } else {
        Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
        Game::preset_ghost_bank = 0;
        game_ui_window.on_render_skip(game_ui_renderer);
    }
        
    if (Game::timestamp - Ui::UiLoadout::last_key_select > 5000)
        Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
    if (!alive()) {
        mobile_l_preset_pick = 0;
        preset_ghost_bank = 0;
    }
    slot_indicator_opacity = lerp(slot_indicator_opacity,
        Ui::UiLoadout::selected_with_keys != MAX_SLOT_COUNT
            || mobile_l_preset_pick || preset_ghost_bank,
        Ui::lerp_amount);
    // Fade hint ring after last adjust (wheel drag or Q/E)
    if (rot_hint_opacity > 0.f && timestamp - rot_hint_stamp > 120) {
        float decay = (float) pow(0.88, Ui::dt * 60 / 1000);
        rot_hint_opacity *= decay;
        if (rot_hint_opacity < 0.02f) rot_hint_opacity = 0.f;
    }

    other_ui_window.render(renderer);

    //no rendering past this point
    // Touch HUD owns movement/flags when Request Mobile is on.
    // Chat open: desktop keeps mouse-aim move (WASD can't reach the game while
    // the HTML input is focused); mobile stands still.
    if (chat_open && Input::mobile_controls()) {
        Input::game_inputs.x = 0;
        Input::game_inputs.y = 0;
        Input::game_inputs.flags = 0;
    } else if (!Input::mobile_controls()) {
        // While typing, prefer mouse aim so keyboard-mode players can still move
        if (Input::keyboard_movement && !chat_open) {
            Input::game_inputs.x = 300 * (Input::keys_held.contains('D') - Input::keys_held.contains('A') + Input::keys_held.contains(39) - Input::keys_held.contains(37));
            Input::game_inputs.y = 300 * (Input::keys_held.contains('S') - Input::keys_held.contains('W') + Input::keys_held.contains(40) - Input::keys_held.contains(38));
        } else {
            // Soft mouse magnitude: near-cursor fine control, saturates by 200 (server full accel)
            float mx = (Input::mouse_x - renderer.width / 2) / Ui::scale;
            float my = (Input::mouse_y - renderer.height / 2) / Ui::scale;
            float dist = std::hypot(mx, my);
            if (dist <= 1e-3f) {
                Input::game_inputs.x = 0;
                Input::game_inputs.y = 0;
            } else {
                float capped = dist >= 200.f ? 200.f : 200.f * std::pow(dist / 200.f, 0.9f);
                float s = capped / dist;
                Input::game_inputs.x = mx * s;
                Input::game_inputs.y = my * s;
            }
        }
        // primary = Space / LMB; secondary = Shift / RMB (RMB suppressed while chat RMB menu open)
        uint8_t primary = Input::keys_held.contains(' ')
            || BitMath::at(Input::mouse_buttons_state, Input::LeftMouse);
        uint8_t secondary = Input::keys_held.contains('\x10')
            || (BitMath::at(Input::mouse_buttons_state, Input::RightMouse) && !chat_menu_open);
        // Invert attack → fire attack on secondary binds; invert defend → defend on primary.
        // Turning both on is a full swap (LMB↔RMB, Space↔Shift).
        uint8_t attack = Input::invert_attack ? secondary : primary;
        uint8_t defend = Input::invert_defend ? primary : secondary;
        Input::game_inputs.flags = (attack << InputFlags::kAttacking) | (defend << InputFlags::kDefending);
    }

    if (socket.ready && alive()) send_inputs();

    if (Input::keys_held_this_tick.contains(';'))
        show_debug = !show_debug;
    if (Input::keys_held_this_tick.contains('M') && alive() && !chat_open && !leave_confirm_open)
        minimap_expanded = !minimap_expanded;
    if (chat_open) refresh_chat_completion_hint();
    if (Input::keys_held_this_tick.contains(9) && chat_open) {
        // Slash draft → Tab completes; otherwise cycle channel (旧行为)
        if (!try_chat_tab_complete())
            cycle_chat_channel();
    }
    // After UI render so slot screen positions are current. One shot per gesture.
    Ui::finish_bag_gesture();
    if (chat_menu_open && BitMath::at(Input::mouse_buttons_released, Input::LeftMouse))
        close_chat_menu();
    // Click outside chat / popups / drawers → close them (Esc still works)
    Ui::try_close_popups_outside_click();
    if (Input::keys_held_this_tick.contains(27) && Game::alive()) {
        if (chat_menu_open) close_chat_menu();
        else if (leave_confirm_open) leave_confirm_open = 0;
        else if (chat_open) close_chat();
        else if (game_panel != kPanelNone) close_game_panel();
        else toggle_pause();
    }
    // X opens bag on death screen too (camera inventory edit before respawn)
    if (!chat_open && !leave_confirm_open
        && (Game::alive() || Game::on_game_screen)) {
        if (Input::keys_held_this_tick.contains('X'))
            toggle_game_panel(kPanelInventory);
        if (Game::alive()) {
            if (Input::keys_held_this_tick.contains('Z'))
                toggle_game_panel(kPanelTalents);
            else if (Input::keys_held_this_tick.contains('C'))
                toggle_game_panel(kPanelCraft);
            else if (Input::keys_held_this_tick.contains('V'))
                toggle_game_panel(kPanelSpecial);
        }
    }
    // Enter: canvas '\r', or DOM overlay Enter / soft-keyboard Send (text_enter_pressed)
    uint8_t const enter = Input::keys_held_this_tick.contains('\r') || DOM::text_enter_pressed;
    DOM::text_enter_pressed = 0;
    if (enter) {
        if (Game::alive()) {
            if (chat_open) send_chat();
            else if (!pause_menu_open && !leave_confirm_open
                && Ui::active_text_input() == nullptr)
                open_chat();
        } else if (!leaving_to_title) {
            Game::spawn_in();
        }
    }

    // Unequipping Antenna/Observer: restore default zoom (don't keep FOV-adjusted zoom).
    if (alive() && simulation.ent_exists(camera_id)) {
        static float prev_cam_fov = BASE_FOV;
        float cur_fov = simulation.get_ent(camera_id).get_fov();
        if (prev_cam_fov < BASE_FOV * 0.92f && cur_fov >= BASE_FOV * 0.92f)
            camera_zoom = 1.28f;
        prev_cam_fov = cur_fov;
    }

    // Wheel zoom — disabled while a game panel (bag/craft/…) is open so scroll owns the wheel
    if (alive() && !chat_open && game_panel == kPanelNone && Input::wheel_delta != 0) {
        float factor = 1.f - Input::wheel_delta * 0.0012f;
        float next = fclamp(camera_zoom * factor, 0.55f, 2.2f);
        if (next != camera_zoom) {
            camera_zoom = next;
            persist_camera_zoom();
        }
        Input::wheel_delta = 0;
    }

    // Mobile pinch-to-zoom (anchor-based, like Maps / florr_clone feel):
    // zoom = startZoom * (dist / startDist). Skip joystick / A / B / R / loadout
    // fingers. Sequential two-finger placement works (not same-frame only).
    {
        static float pinch_start_dist = 0.f;
        static float pinch_start_zoom = 1.f;
        static uint8_t pinch_on = 0;
        static uint8_t pinch_armed = 0;
        static uint32_t pinch_id_a = (uint32_t) -1;
        static uint32_t pinch_id_b = (uint32_t) -1;
        auto end_pinch = [&](uint8_t save) {
            if (save && pinch_on) persist_camera_zoom();
            pinch_on = 0;
            pinch_armed = 0;
            pinch_start_dist = 0.f;
            pinch_id_a = (uint32_t) -1;
            pinch_id_b = (uint32_t) -1;
        };
        uint8_t can_try = alive() && !chat_open && game_panel == kPanelNone
            && Input::mobile_controls();
        if (!can_try) {
            end_pinch(1);
        } else {
            Input::Touch const *cands[8] = {};
            uint32_t cand_n = 0;
            for (auto const &[id, t] : Input::touches) {
                if (Ui::mobile_touch_is_control(id)) continue;
                if (Ui::loadout_bar_hit(t.x, t.y)) continue;
                if (cand_n < 8) cands[cand_n++] = &t;
            }
            Input::Touch const *a = nullptr;
            Input::Touch const *b = nullptr;
            if (pinch_on) {
                auto ia = Input::touches.find(pinch_id_a);
                auto ib = Input::touches.find(pinch_id_b);
                if (ia != Input::touches.end() && ib != Input::touches.end()
                    && !Ui::mobile_touch_is_control(pinch_id_a)
                    && !Ui::mobile_touch_is_control(pinch_id_b)) {
                    a = &ia->second;
                    b = &ib->second;
                } else {
                    end_pinch(1);
                }
            }
            if (a == nullptr && cand_n >= 2) {
                a = cands[0];
                b = cands[1];
                float dx = a->x - b->x;
                float dy = a->y - b->y;
                float dist = std::sqrt(dx * dx + dy * dy);
                // Need a real span (~40 CSS px at dpr≤2) before arming a pinch.
                if (dist >= 48.f) {
                    pinch_id_a = a->id;
                    pinch_id_b = b->id;
                    pinch_start_dist = dist;
                    pinch_start_zoom = camera_zoom;
                    pinch_on = 1;
                    pinch_armed = 0;
                } else {
                    a = nullptr;
                    b = nullptr;
                }
            }
            if (a != nullptr && b != nullptr && pinch_on) {
                float dx = a->x - b->x;
                float dy = a->y - b->y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (pinch_start_dist > 24.f && dist > 24.f) {
                    float ratio = dist / pinch_start_dist;
                    // Ignore finger-settle jitter until the span changes ~4%.
                    if (!pinch_armed && std::fabs(ratio - 1.f) > 0.04f)
                        pinch_armed = 1;
                    if (pinch_armed)
                        camera_zoom = fclamp(pinch_start_zoom * ratio, 0.55f, 2.2f);
                }
                Input::touches[a->id].saturated = 1;
                Input::touches[b->id].saturated = 1;
            } else if (pinch_on) {
                end_pinch(1);
            }
        }
    }

    // Spawn watchdog — only while intentionally waiting to enter (title /
    // death Continue). Never keep firing kClientSpawn mid-fight: a desync
    // where the client thinks it is dead but the server flower is alive used
    // to force a respawn into all-Basic and wipe the account loadout.
    if (spawn_request_at > 0) {
        if (alive()) {
            spawn_request_at = 0;
            spawn_retries = 0;
            if (account_status.find("重试") != std::string::npos
                || account_status.find("Retrying") != std::string::npos)
                account_status.clear();
        } else if (leaving_to_title) {
            // Soft-leave in progress — do not keep requesting spawn.
            spawn_request_at = 0;
            spawn_retries = 0;
        } else {
            // Back off: 1.2s → 2.5s → 4s (cap) so we don't flood the server
            double wait_ms = 1200.0 + (double) std::min<uint8_t>(spawn_retries, 8) * 350.0;
            if (wait_ms > 4000.0) wait_ms = 4000.0;
            if (timestamp - spawn_request_at > wait_ms) {
                if (spawn_retries < 250) ++spawn_retries;
                if (spawn_retries == 1
                    || (spawn_retries % 5 == 0 && account_status.empty()))
                    account_status = I18n::use_chinese
                        ? "正在重试进入游戏…"
                        : "Retrying to enter the game…";
                if (spawn_retries == 20 || spawn_retries == 60)
                    std::cout << "spawn still waiting after " << (uint32_t) spawn_retries
                              << " retries (camera=" << (simulation.ent_exists(camera_id) ? 1 : 0)
                              << " ready=" << (uint32_t) simulation_ready
                              << " screen=" << (uint32_t) on_game_screen << ")" << std::endl;
                send_spawn_request();
            }
        }
    }

    //clearing operations
    simulation.post_tick();
    sync_guest_progress();
    Storage::set();
    Input::reset();
    Debug::frame_times.push_back(Ui::dt);
    Debug::tick_times.push_back(Debug::get_timestamp() - tick_start);
}

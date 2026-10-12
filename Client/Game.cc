#include <Client/Game.hh>

#include <Client/Debug.hh>
#include <Client/Input.hh>
#include <Client/Particle.hh>
#include <Client/Setup.hh>
#include <Client/Storage.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/TextInput.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <cstdlib>
#include <string>

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
    std::string username;
    std::string password;
    std::string account_status;
    uint64_t session_token = 0;
    uint8_t pending_auto_login = 0;
    uint8_t account_logged_in = 0;
    uint8_t auth_panel = 0;
    std::string disconnect_message;
    std::array<uint8_t, PetalID::kNumPetals> seen_petals;
    std::array<uint8_t, MobID::kNumMobs> seen_mobs;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> cached_loadout = {PetalID::kNone};

    double timestamp = 0;

    double score = 0;
    float overlevel_timer = 0;
    float slot_indicator_opacity = 0;
    float transition_circle = 0;

    uint32_t respawn_level = 1;


    uint8_t loadout_count = 5;
    uint8_t simulation_ready = 0;
    uint8_t on_game_screen = 0;
    uint8_t show_debug = 0;
    uint8_t selected_map = MapID::kGarden;
    uint8_t minimap_expanded = 0;
    float camera_zoom = 1.f;
    uint8_t pause_menu_open = 0;
    uint8_t leave_confirm_open = 0;
    uint8_t game_panel = 0;
    uint8_t chat_open = 0;
    uint8_t chat_channel = ChatChannel::kChatNearby;
    std::string chat_whisper_to;
    std::string chat_draft;
    std::vector<ChatLine> chat_lines;
    std::unordered_map<std::string, SpeechBubble> speech_bubbles;
    uint8_t chat_menu_open = 0;
    std::string chat_menu_name;
    float chat_menu_x = 0;
    float chat_menu_y = 0;
    std::string party_status;
    uint8_t party_member_count = 0;
    uint8_t revive_progress = 0;
    uint8_t awaiting_party_revive = 0;
    std::array<uint32_t, PetalID::kNumPetals> craft_bank = {};
    uint32_t coins = 0;
    uint8_t guest_save_valid = 0;
    uint8_t guest_level = 1;
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> guest_petals = {PetalID::kNone};
    std::array<uint32_t, PetalID::kNumPetals> guest_craft_bank = {};
    uint32_t guest_coins = 0;
    uint8_t talent_hp = 0;
    uint8_t talent_rot = 0;
    uint8_t rot_speed_setting = 7;
    uint8_t craft_selected_rarity = 0;
    PetalID::T craft_tray_id = PetalID::kNone;
    uint32_t craft_tray_count = 0;
    std::string craft_status;
}

using namespace Game;

void Game::init() {
    Input::is_mobile = check_mobile();
    Storage::retrieve();
    reset();
    title_ui_window.add_child(
        [](){ 
            Ui::Element *elt = new Ui::StaticText(60, "ashflor.io");
            elt->x = 0;
            elt->y = -270;
            return elt;
        }()
    );
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
    for (uint8_t i = 0; i < MAX_SLOT_COUNT * 2; ++i) game_ui_window.add_child(new Ui::UiLoadoutPetal(i));
    game_ui_window.add_child(
        Ui::make_overlevel_indicator()
    );
    game_ui_window.add_child(
        Ui::make_stat_screen()
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
        Ui::make_game_side_buttons()
    );
    game_ui_window.add_child(
        Ui::make_inventory_panel()
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
    Ui::make_petal_tooltips();
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
    std::string ws_url = dyn_url ? dyn_url : WS_URL;
    free(dyn_url);
    socket.connect(ws_url);
}

void Game::reset() {
    simulation_ready = 0;
    on_game_screen = 0;
    pause_menu_open = 0;
    leave_confirm_open = 0;
    game_panel = 0;
    chat_open = 0;
    chat_channel = ChatChannel::kChatNearby;
    chat_whisper_to.clear();
    chat_draft.clear();
    chat_lines.clear();
    chat_menu_open = 0;
    chat_menu_name.clear();
    party_status.clear();
    party_member_count = 0;
    revive_progress = 0;
    awaiting_party_revive = 0;
    account_logged_in = 0;
    pending_auto_login = 0;
    account_status.clear();
    auth_panel = 0;
    score = 0;
    overlevel_timer = 0;
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
    } else {
        craft_bank = {};
        coins = 0;
    }
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
    writer.write<uint8_t>(guest_level < 1 ? 1 : guest_level);
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        writer.write<uint8_t>(guest_petals[i]);
    for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
        writer.write<uint32_t>(guest_craft_bank[i]);
    writer.write<uint32_t>(guest_coins);
    writer.write<uint8_t>(talent_hp);
    writer.write<uint8_t>(talent_rot);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::sync_guest_progress() {
    if (account_logged_in) return;
    guest_craft_bank = craft_bank;
    guest_coins = coins;
    if (alive()) {
        guest_level = (uint8_t) std::min<uint32_t>(score_to_level(score), 255);
        if (guest_level < 1) guest_level = 1;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            guest_petals[i] = cached_loadout[i];
        guest_save_valid = 1;
    } else if (simulation.ent_exists(camera_id)) {
        // Dead: camera inventory preserves slot layout (primary + backpack).
        Entity const &camera = simulation.get_ent(camera_id);
        guest_level = camera.get_respawn_level();
        if (guest_level < 1) guest_level = 1;
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            guest_petals[i] = camera.get_inventory(i);
        guest_save_valid = 1;
    }
}

void Game::close_game_panel() {
    game_panel = kPanelNone;
}

uint8_t Game::game_panel_blocking() {
    return game_panel != kPanelNone;
}

void Game::toggle_game_panel(uint8_t panel) {
    if (!alive() || leave_confirm_open) return;
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
    Input::keys_held.clear();
    Input::game_inputs.x = 0;
    Input::game_inputs.y = 0;
    Input::game_inputs.flags = 0;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->focus();
}

void Game::close_chat() {
    if (!chat_open) return;
    chat_open = 0;
    chat_draft.clear();
    close_chat_menu();
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->blur();
}

void Game::push_chat_line(uint8_t channel, std::string const &sender, std::string const &text) {
    if (text.empty() && sender.empty()) return;
    chat_lines.push_back({ channel, sender, text, timestamp + 12000 });
    while (chat_lines.size() > CHAT_HISTORY)
        chat_lines.erase(chat_lines.begin());
    // Overhead bubble above speaker (~5s); skip system lines
    if (!sender.empty() && !text.empty() && channel != ChatChannel::kChatSystem)
        speech_bubbles[sender] = { text, timestamp + 5000 };
}

void Game::cycle_chat_channel() {
    if (!chat_open) return;
    if (chat_channel == ChatChannel::kChatNearby)
        chat_channel = ChatChannel::kChatParty;
    else if (chat_channel == ChatChannel::kChatParty)
        chat_channel = ChatChannel::kChatGuild;
    else if (chat_channel == ChatChannel::kChatGuild) {
        if (!chat_whisper_to.empty())
            chat_channel = ChatChannel::kChatWhisper;
        else
            chat_channel = ChatChannel::kChatNearby;
    } else
        chat_channel = ChatChannel::kChatNearby;
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

void Game::chat_menu_whisper() {
    if (chat_menu_name.empty()) return;
    chat_whisper_to = chat_menu_name;
    chat_channel = ChatChannel::kChatWhisper;
    close_chat_menu();
    if (!chat_open) open_chat();
}

void Game::toggle_pause() {
    if (!alive()) return;
    if (leave_confirm_open) {
        leave_confirm_open = 0;
        return;
    }
    if (chat_open) close_chat();
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
    close_chat();
    pause_menu_open = 0;
    leave_confirm_open = 0;
    on_game_screen = 0;
    camera_zoom = 1.f;
    speech_bubbles.clear();
    socket.disconnect();
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
    Ui::lerp_amount = 1 - pow(1 - 0.2, Ui::dt * 60 / 1000);
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
        on_game_screen = 1;
        player_id = simulation.get_ent(camera_id).get_player();
        Entity const &player = simulation.get_ent(player_id);
        Game::loadout_count = player.get_loadout_count();
        for (uint32_t i = 0; i < MAX_SLOT_COUNT + Game::loadout_count; ++i) {
            cached_loadout[i] = player.get_loadout_ids(i);
            Game::seen_petals[cached_loadout[i]] = 1;
        }
        score = player.get_score();
        overlevel_timer = player.get_overlevel_timer();
    } else {
        player_id = NULL_ENTITY;
        overlevel_timer = 0;
        pause_menu_open = 0;
        leave_confirm_open = 0;
        if (chat_open) close_chat();
    }

    while (!chat_lines.empty() && chat_lines.front().expires_at <= timestamp)
        chat_lines.erase(chat_lines.begin());
    for (auto it = speech_bubbles.begin(); it != speech_bubbles.end(); ) {
        if (it->second.expires_at <= timestamp)
            it = speech_bubbles.erase(it);
        else
            ++it;
    }

    //event poll
    if (Input::is_mobile) {
        for (auto &x : Input::touches) {
            Input::Touch const &touch = x.second;
            if (touch.saturated) continue;
            Game::poll_ui_event({ .id = touch.id, .x = touch.x, .y = touch.y, .press = 1 });
        }
    }
    else {
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
    // Number keys: 1:1 swap primary[i] ↔ secondary[i]. R = swap all. T = trash. Q/E = spin rate.
    if (!Game::pause_menu_open && !Game::chat_open && !Game::leave_confirm_open
            && !Game::game_panel_blocking()) {
            if (Input::keys_held_this_tick.contains('Q') && Game::rot_speed_setting > 0)
                --Game::rot_speed_setting;
            if (Input::keys_held_this_tick.contains('E') && Game::rot_speed_setting < 10)
                ++Game::rot_speed_setting;
            if (Input::keys_held_this_tick.contains('R'))
                Game::swap_all_petals();
            for (uint8_t i = 0; i < Game::loadout_count; ++i) {
                if (!Input::keys_held_this_tick.contains(SLOT_KEYBINDS[i])) continue;
                uint8_t sec = (uint8_t) (Game::loadout_count + i);
                // Skip no-op empty↔empty
                if (Game::cached_loadout[i] == PetalID::kNone
                    && Game::cached_loadout[sec] == PetalID::kNone)
                    continue;
                Ui::ui_swap_petals(i, sec);
                break;
            }
            if (Input::keys_held_this_tick.contains('T')
                && Ui::UiLoadout::selected_with_keys < MAX_SLOT_COUNT) {
                uint8_t sec = (uint8_t) (Game::loadout_count + Ui::UiLoadout::selected_with_keys);
                if (Game::cached_loadout[sec] != PetalID::kNone)
                    Ui::ui_delete_petal(sec);
                Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
            }
        }
    } else {
        Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
        game_ui_window.on_render_skip(game_ui_renderer);
    }
        
    if (Game::timestamp - Ui::UiLoadout::last_key_select > 5000)
        Ui::UiLoadout::selected_with_keys = MAX_SLOT_COUNT;
    slot_indicator_opacity = lerp(slot_indicator_opacity, Ui::UiLoadout::selected_with_keys != MAX_SLOT_COUNT, Ui::lerp_amount);

    other_ui_window.render(renderer);

    //no rendering past this point
    if (!Input::is_mobile) {
        if (Input::keyboard_movement) {
            Input::game_inputs.x = 300 * (Input::keys_held.contains('D') - Input::keys_held.contains('A') + Input::keys_held.contains(39) - Input::keys_held.contains(37));
            Input::game_inputs.y = 300 * (Input::keys_held.contains('S') - Input::keys_held.contains('W') + Input::keys_held.contains(40) - Input::keys_held.contains(38));
        } else {
           Input::game_inputs.x = (Input::mouse_x - renderer.width / 2) / Ui::scale;
           Input::game_inputs.y = (Input::mouse_y - renderer.height / 2) / Ui::scale;
        }
        uint8_t primary = Input::keys_held.contains(' ')
            || BitMath::at(Input::mouse_buttons_state, Input::LeftMouse);
        uint8_t secondary = (Input::keys_held.contains('\x10')
            || BitMath::at(Input::mouse_buttons_state, Input::RightMouse))
            && !chat_menu_open && !chat_open;
        uint8_t attack = Input::invert_attack ? secondary : primary;
        uint8_t defend = Input::invert_defend ? primary : secondary;
        Input::game_inputs.flags = (attack << InputFlags::kAttacking) | (defend << InputFlags::kDefending);
    }

    if (socket.ready && alive()) send_inputs();

    if (Input::keys_held_this_tick.contains(';'))
        show_debug = !show_debug;
    if (Input::keys_held_this_tick.contains('M') && alive() && !chat_open && !leave_confirm_open)
        minimap_expanded = !minimap_expanded;
    if (Input::keys_held_this_tick.contains(9) && chat_open)
        cycle_chat_channel();
    if (chat_menu_open && BitMath::at(Input::mouse_buttons_released, Input::LeftMouse))
        close_chat_menu();
    // Click outside chat input while typing → blur / cancel
    if (chat_open && BitMath::at(Input::mouse_buttons_pressed, Input::LeftMouse) && !chat_menu_open) {
        Ui::TextInput *input = Ui::chat_text_input();
        if (input == nullptr || !input->contains_screen_point(Input::mouse_x, Input::mouse_y))
            close_chat();
    }
    // Click outside any open popup / drawer → close it
    Ui::try_close_popups_outside_click();
    if (Input::keys_held_this_tick.contains(27) && Game::alive()) {
        if (chat_menu_open) close_chat_menu();
        else if (leave_confirm_open) leave_confirm_open = 0;
        else if (chat_open) close_chat();
        else if (game_panel != kPanelNone) close_game_panel();
        else toggle_pause();
    }
    if (Game::alive() && !chat_open && !leave_confirm_open) {
        if (Input::keys_held_this_tick.contains('B'))
            toggle_game_panel(kPanelInventory);
        else if (Input::keys_held_this_tick.contains('Z'))
            toggle_game_panel(kPanelTalents);
        else if (Input::keys_held_this_tick.contains('C'))
            toggle_game_panel(kPanelCraft);
        else if (Input::keys_held_this_tick.contains('V'))
            toggle_game_panel(kPanelSpecial);
    }
    if (Input::keys_held_this_tick.contains('\r')) {
        if (Game::alive()) {
            if (chat_open) send_chat();
            else if (!pause_menu_open && !leave_confirm_open) open_chat();
        } else {
            Game::spawn_in();
        }
    }

    // Wheel zoom — disabled while a game panel (bag/craft/…) is open so scroll owns the wheel
    if (alive() && !chat_open && game_panel == kPanelNone && Input::wheel_delta != 0) {
        float factor = 1.f - Input::wheel_delta * 0.0012f;
        camera_zoom = fclamp(camera_zoom * factor, 0.55f, 2.2f);
        Input::wheel_delta = 0;
    }

    //clearing operations
    simulation.post_tick();
    sync_guest_progress();
    Storage::set();
    Input::reset();
    Debug::frame_times.push_back(Ui::dt);
    Debug::tick_times.push_back(Debug::get_timestamp() - tick_start);
}

#include <Client/Game.hh>

#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Ui/TextInput.hh>
#include <Client/Ui/Ui.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

using namespace Game;

void Game::on_message(uint8_t *ptr, uint32_t len) {
    Reader reader(ptr);
    switch(reader.read<uint8_t>()) {
        case Clientbound::kAccountResult: {
            uint8_t result = reader.read<uint8_t>();
            uint32_t level = reader.read<uint32_t>();
            account_logged_in = (result == AccountResultCode::kAccountOk);
            uint8_t was_auto = pending_auto_login;
            pending_auto_login = 0;
            static I18n::Str const reasons[] = {
                I18n::kAuthOk,
                I18n::kAuthInvalidUser,
                I18n::kAuthInvalidPass,
                I18n::kAuthExists,
                I18n::kAuthNotFound,
                I18n::kAuthWrongPass,
                I18n::kAuthAlreadyIn,
                I18n::kAuthLoggedOut
            };
            if (result < sizeof(reasons) / sizeof(reasons[0]))
                account_status = I18n::tr(reasons[result]);
            else
                account_status = I18n::tr(I18n::kAuthInvalidUser);
            if (account_logged_in) {
                // In-game display name is always the account username (not editable)
                nickname = username;
                // Keep guest_id untouched so logout still has a fixed id
                Game::respawn_level = level < 1 ? 1 : level;
                auth_panel = 0;
                for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                    PetalID::T id = reader.read<PetalID::T>();
                    if (id < PetalID::kNumPetals && id != PetalID::kNone)
                        Game::seen_petals[id] = 1;
                }
            } else {
                for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                    reader.read<PetalID::T>();
            }
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                craft_bank[i] = reader.read<uint32_t>();
            coins = reader.read<uint32_t>();
            uint64_t token = reader.read<uint64_t>();
            if (account_logged_in)
                session_token = token;
            else if (was_auto || result == AccountResultCode::kAccountLoggedOut ||
                     result == AccountResultCode::kAccountWrongPass ||
                     result == AccountResultCode::kAccountNotFound ||
                     result == AccountResultCode::kAccountInvalidUser)
                session_token = 0;
            // Explicit logout → back to guest local progress (account row kept on server)
            if (result == AccountResultCode::kAccountLoggedOut) {
                account_logged_in = 0;
                daily_bonus_seconds = 0;
                offline_bonus_seconds = 0;
                auth_panel = 0;
                password.clear();
                session_token = 0;
                craft_tray_clear();
                ensure_guest_id();
                talent_hp = guest_talent_hp;
                talent_rot = guest_talent_rot;
                talent_pending = 0;
                talent_status.clear();
                if (guest_save_valid) {
                    respawn_level = guest_level < 1 ? 1 : guest_level;
                    craft_bank = guest_craft_bank;
                    coins = guest_coins;
                } else {
                    respawn_level = 1;
                    craft_bank = {};
                    coins = 0;
                }
                on_game_screen = 0;
                send_guest_progress();
            } else if (was_auto && !account_logged_in) {
                // Already online elsewhere or stale token → play as guest this connection
                ensure_guest_id();
                talent_hp = guest_talent_hp;
                talent_rot = guest_talent_rot;
                send_guest_progress();
            }
            if (spawn_after_auth) {
                spawn_after_auth = 0;
                spawn_in();
            }
            break;
        }
        case Clientbound::kSpawnResult: {
            uint8_t result = reader.read<uint8_t>();
            if (result == SpawnResultCode::kSpawnNameTaken)
                account_status = I18n::tr(I18n::kSpawnNameTaken);
            break;
        }
        case Clientbound::kCraftUpdate: {
            uint8_t result = reader.read<uint8_t>();
            PetalID::T petal = reader.read<PetalID::T>();
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                craft_bank[i] = reader.read<uint32_t>();
            coins = reader.read<uint32_t>();
            // Tray stages owned petals (bag + bank) — clamp after sync
            if (result == CraftResultCode::kCraftWin || result == CraftResultCode::kCraftMiss) {
                if (craft_tray_id != PetalID::kNone) {
                    if (craft_tray_count <= Craft::kCraftBatch) craft_tray_clear();
                    else craft_tray_count -= Craft::kCraftBatch;
                }
            }
            if (craft_tray_id != PetalID::kNone) {
                uint32_t owned = craft_owned(craft_tray_id);
                if (owned == 0) craft_tray_clear();
                else if (craft_tray_count > owned)
                    craft_tray_count = owned;
            }
            if (result == CraftResultCode::kCraftNoMats)
                craft_status = I18n::use_chinese ? "材料不足" : "Not enough materials";
            else if (result == CraftResultCode::kCraftNoCoins)
                craft_status = I18n::use_chinese ? "钱币不足" : "Not enough coins";
            else if (result == CraftResultCode::kCraftMiss)
                craft_status = I18n::use_chinese ? "合成失败…" : "Craft failed…";
            else if (result == CraftResultCode::kCraftWin)
                craft_status = I18n::use_chinese
                    ? std::format("成功！获得 {}（已进背包）", I18n::petal_name(petal))
                    : std::format("Success! {} → bag", I18n::petal_name(petal));
            else if (result == CraftResultCode::kCraftExchanged)
                craft_status = I18n::use_chinese
                    ? std::format("直兑获得 {}（已进背包）", I18n::petal_name(petal))
                    : std::format("Exchanged {} → bag", I18n::petal_name(petal));
            else if (result == CraftResultCode::kCraftBought)
                craft_status = I18n::use_chinese
                    ? std::format("购买成功！{} 已放入背包", I18n::petal_name(petal))
                    : std::format("Bought {} → backpack", I18n::petal_name(petal));
            else if (result == CraftResultCode::kCraftInvFull)
                craft_status = I18n::use_chinese
                    ? "背包已满，先腾出空位"
                    : "Backpack full — free a slot first";
            if (petal < PetalID::kNumPetals && petal != PetalID::kNone)
                seen_petals[petal] = 1;
            break;
        }
        case Clientbound::kBoostUpdate: {
            daily_bonus_seconds = reader.read<uint32_t>();
            offline_bonus_seconds = reader.read<uint32_t>();
            bonus_enabled = reader.read<uint8_t>() ? 1 : 0;
            bonus_synced_at = timestamp;
            break;
        }
        case Clientbound::kTalentUpdate: {
            uint8_t hp = reader.read<uint8_t>();
            uint8_t rot = reader.read<uint8_t>();
            if (talent_pending) {
                uint8_t got = talent_pending_id == TalentID::kHealth ? hp : rot;
                talent_status = got > talent_pending_rank
                    ? (I18n::use_chinese ? "加点成功，属性已生效" : "Talent upgraded and active")
                    : (I18n::use_chinese ? "加点未成功，请查看等级或点数" : "Upgrade unavailable; check level and points");
                talent_pending = 0;
            }
            talent_hp = hp;
            talent_rot = rot;
            if (!account_logged_in) {
                guest_talent_hp = hp;
                guest_talent_rot = rot;
            }
            break;
        }
        case Clientbound::kLoadoutPresetSync: {
            uint8_t result = reader.read<uint8_t>();
            uint8_t index = reader.read<uint8_t>();
            for (LoadoutPreset &preset : loadout_presets) {
                preset.slot_count = reader.read<uint8_t>();
                if (preset.slot_count > MAX_SLOT_COUNT) preset.slot_count = 0;
                for (PetalID::T &id : preset.petals) {
                    id = reader.read<PetalID::T>();
                    if (id >= PetalID::kNumPetals) id = PetalID::kNone;
                }
            }
            if (result == kPresetSaved)
                loadout_preset_status = I18n::use_chinese
                    ? std::format("方案 {} 已保存到账号", index + 1)
                    : std::format("Preset {} saved to account", index + 1);
            else if (result == kPresetApplied || result == kPresetAppliedAdjusted
                || result == kPresetAppliedBanked) {
                active_loadout_preset = index;
                if (result == kPresetAppliedBanked) {
                    char const *bank = index >= 10 ? "L" : "K";
                    uint8_t const digit = (uint8_t) ((index % 10) + 1);
                    char const *d = digit == 10 ? "0" : nullptr;
                    std::string dbuf;
                    if (d == nullptr) {
                        dbuf = std::to_string(digit);
                        d = dbuf.c_str();
                    }
                    loadout_preset_status = I18n::use_chinese
                        ? std::format("已切到空的 {}{}（离开前的配装已自动保存）", bank, d)
                        : std::format("Empty {}{} (previous loadout auto-saved)", bank, d);
                }
                else if (result == kPresetAppliedAdjusted)
                    loadout_preset_status = I18n::use_chinese
                        ? std::format("方案 {} 已切换；缺少的花瓣已降级或留空", index + 1)
                        : std::format("Preset {} applied; lower tier/empty used", index + 1);
                else
                    loadout_preset_status = I18n::use_chinese
                        ? std::format("已切换到方案 {}", index + 1)
                        : std::format("Preset {} equipped", index + 1);
                for (Ui::UiLoadoutPetal *slot : Ui::UiLoadout::petal_slots)
                    if (slot != nullptr) slot->no_change_ticks = 0;
            } else if (result == kPresetEmpty)
                loadout_preset_status = I18n::use_chinese ? "方案尚未保存" : "Preset is empty";
            else if (result == kPresetNeedAccount)
                loadout_preset_status = I18n::use_chinese ? "请先登录账号以保存云端方案" : "Sign in to save cloud presets";
            else {
                active_loadout_preset = LOADOUT_PRESET_COUNT;
                loadout_preset_status.clear();
            }
            break;
        }
        case Clientbound::kClientUpdate: {
            simulation_ready = 1;
            camera_id = reader.read<EntityID>();
            EntityID curr_id = reader.read<EntityID>();
            while(!(curr_id == NULL_ENTITY)) {
                // The id may already be gone locally (deleted on a pending
                // delete tick). Deleting it twice would desync the id/hash
                // table and can leave the camera without a player.
                if (simulation.ent_exists(curr_id))
                    simulation._delete_ent(curr_id);
                curr_id = reader.read<EntityID>();
            }
            curr_id = reader.read<EntityID>();
            while(!(curr_id == NULL_ENTITY)) {
                uint8_t create = reader.read<uint8_t>();
                if (BitMath::at(create, 0)) simulation.force_alloc_ent(curr_id);
                assert(simulation.ent_exists(curr_id));
                Entity &ent = simulation.get_ent(curr_id);
                ent.read(&reader, BitMath::at(create, 0));
                if (BitMath::at(create, 1)) ent.pending_delete = 1;
                curr_id = reader.read<EntityID>();
            }
            simulation.arena_info.read(&reader, reader.read<uint8_t>());
            simulation.map_id = simulation.arena_info.map_id;
            if (simulation.map_id >= MapID::kNumMaps)
                simulation.map_id = MapID::kGarden;
            break;
        }
        case Clientbound::kChatMessage: {
            uint8_t channel = reader.read<uint8_t>();
            std::string sender;
            std::string text;
            reader.read<std::string>(sender);
            reader.read<std::string>(text);
            push_chat_line(channel, sender, text);
            break;
        }
        case Clientbound::kPartyUpdate: {
            uint8_t count = reader.read<uint8_t>();
            party_member_count = count;
            party_status.clear();
            party_members.clear();
            awaiting_party_revive = 0;
            for (uint8_t i = 0; i < count; ++i) {
                PartyMember m;
                reader.read<std::string>(m.name);
                m.flags = reader.read<uint8_t>();
                m.hp_pct = reader.read<uint8_t>();
                m.x = reader.read<float>();
                m.y = reader.read<float>();
                party_members.push_back(m);
                if (!party_status.empty()) party_status += ", ";
                if (m.flags & 1) party_status += "*";
                party_status += m.name;
                if (m.flags & 2) party_status += " [DOWN]";
                if ((m.flags & 4) && (m.flags & 2)) awaiting_party_revive = 1;
            }
            revive_progress = reader.read<uint8_t>();
            if (count == 0) {
                party_status = I18n::tr(I18n::kPartyEmpty);
                awaiting_party_revive = 0;
                revive_progress = 0;
            }
            break;
        }
        case Clientbound::kInvitePrompt: {
            invite_type = reader.read<uint8_t>();
            reader.read<std::string>(invite_from);
            invite_open = invite_from.empty() ? 0 : 1;
            break;
        }
        case Clientbound::kLeaderboardStats: {
            LeaderboardBoard &board = leaderboard_board;
            board.recorded = reader.read<uint32_t>();
            for (uint8_t category = 0; category < kLbCategoryCount; ++category) {
                uint8_t count = reader.read<uint8_t>();
                if (count > LEADERBOARD_STAT_SIZE) count = LEADERBOARD_STAT_SIZE;
                for (uint8_t i = 0; i < LEADERBOARD_STAT_SIZE; ++i) {
                    if (i >= count) {
                        board.tops[category][i].name.clear();
                        board.tops[category][i].value = 0;
                        continue;
                    }
                    reader.read<std::string>(board.tops[category][i].name);
                    board.tops[category][i].value = reader.read<uint64_t>();
                }
                board.has_self[category] = reader.read<uint8_t>();
                reader.read<std::string>(board.self_name[category]);
                board.self_rank[category] = reader.read<uint32_t>();
                board.self_value[category] = reader.read<uint64_t>();
            }
            leaderboard_board_at = Game::timestamp;
            break;
        }
        case Clientbound::kLifeSummary: {
            life_mob_kills.clear();
            life_petal_picks.clear();
            uint8_t mob_n = reader.read<uint8_t>();
            for (uint8_t i = 0; i < mob_n; ++i) {
                LifeEntry e;
                e.id = reader.read<uint8_t>();
                e.count = reader.read<uint16_t>();
                if (e.id < MobID::kNumMobs && e.count > 0)
                    life_mob_kills.push_back(e);
            }
            uint16_t petal_n = reader.read<uint16_t>();
            for (uint16_t i = 0; i < petal_n; ++i) {
                LifeEntry e;
                e.id = reader.read<PetalID::T>();
                e.count = reader.read<uint16_t>();
                if (e.id < PetalID::kNumPetals && e.count > 0)
                    life_petal_picks.push_back(e);
            }
            // Rarity high→low, then count high→low
            auto by_mob = [](LifeEntry const &a, LifeEntry const &b) {
                uint8_t ra = MOB_DATA[a.id].rarity, rb = MOB_DATA[b.id].rarity;
                if (ra != rb) return ra > rb;
                if (a.count != b.count) return a.count > b.count;
                return a.id < b.id;
            };
            auto by_petal = [](LifeEntry const &a, LifeEntry const &b) {
                uint8_t ra = PETAL_DATA[a.id].rarity, rb = PETAL_DATA[b.id].rarity;
                if (ra != rb) return ra > rb;
                if (a.count != b.count) return a.count > b.count;
                return a.id < b.id;
            };
            std::sort(life_mob_kills.begin(), life_mob_kills.end(), by_mob);
            std::sort(life_petal_picks.begin(), life_petal_picks.end(), by_petal);
            life_summary_ready = 1;
            break;
        }
        case Clientbound::kGuildUpdate: {
            reader.read<std::string>(guild_name);
            uint8_t count = reader.read<uint8_t>();
            guild_member_count = count;
            guild_status.clear();
            for (uint8_t i = 0; i < count; ++i) {
                std::string name;
                reader.read<std::string>(name);
                uint8_t flags = reader.read<uint8_t>();
                if (!guild_status.empty()) guild_status += ", ";
                if (flags & 1) guild_status += "*";
                guild_status += name;
            }
            if (count == 0)
                guild_status = I18n::tr(I18n::kGuildEmpty);
            break;
        }
        default:
            break;
    }
}

void Game::request_leaderboard_board() {
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kLeaderboardRequest);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::send_inputs() {
    // Chat box stays open while moving. Any clicked popup stops the flower.
    uint8_t blocked = Input::freeze_input || Game::pause_menu_open
        || Game::leave_confirm_open || Game::game_panel_blocking()
        || Game::guild_panel_open || Game::invite_open || Game::chat_menu_open
        || Ui::panel_open != Ui::Panel::kNone;
    float x = blocked ? 0.f : Input::game_inputs.x;
    float y = blocked ? 0.f : Input::game_inputs.y;
    uint8_t flags = blocked ? 0 : Input::game_inputs.flags;
    static double last_sent_at = -1000;
    static float last_x = 0, last_y = 0;
    static uint8_t last_flags = 0, last_rot = 0;
    // Send starts, stops, attacks, and large direction changes immediately.
    // Steady input only needs one packet per 25 TPS server step.
    uint8_t changed = flags != last_flags || rot_speed_setting != last_rot
        || (x == 0.f) != (last_x == 0.f)
        || (y == 0.f) != (last_y == 0.f)
        || std::abs(x - last_x) > 24.f || std::abs(y - last_y) > 24.f;
    if (!changed && Game::timestamp - last_sent_at < 40) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kClientInput);
    writer.write<float>(x);
    writer.write<float>(y);
    writer.write<uint8_t>(flags);
    writer.write<uint8_t>(rot_speed_setting);
    socket.send(writer.packet, writer.at - writer.packet);
    last_sent_at = Game::timestamp;
    last_x = x;
    last_y = y;
    last_flags = flags;
    last_rot = rot_speed_setting;
}

void Game::talent_spend(uint8_t talent_id) {
    if (talent_id >= TalentID::kNum || talent_pending) return;
    if (!socket.ready || !simulation_ready || !simulation.ent_exists(camera_id)) {
        talent_status = I18n::use_chinese ? "未连接，暂不能加点" : "Connect before upgrading";
        return;
    }
    uint32_t level = alive() ? score_to_level(score)
        : simulation.get_ent(camera_id).get_respawn_level();
    uint8_t rank = talent_id == TalentID::kHealth ? talent_hp : talent_rot;
    if (rank >= Talent::max_rank(level)) {
        talent_status = I18n::use_chinese ? "当前等级的天赋阶数已满" : "Rank limit reached for this level";
        return;
    }
    if (Talent::points_free(level, talent_hp, talent_rot) < Talent::rank_cost(rank + 1)) {
        talent_status = I18n::use_chinese ? "天赋点不足" : "Not enough talent points";
        return;
    }
    talent_pending = 1;
    talent_pending_id = talent_id;
    talent_pending_rank = rank;
    talent_status = I18n::use_chinese ? "加点中…" : "Upgrading…";
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kTalentSpend);
    writer.write<uint8_t>(talent_id);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::send_chat() {
    if (!alive() || !chat_open) return;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->sync_from_dom();
    std::string text = chat_draft;
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.erase(text.begin());
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();
    uint8_t channel = chat_channel;
    std::string target = (channel == ChatChannel::kChatWhisper) ? chat_whisper_to : std::string{};
    close_chat();
    if (text.empty() || !socket.ready) return;
    if (channel == ChatChannel::kChatWhisper && target.empty()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kChat);
    writer.write<uint8_t>(channel);
    writer.write<std::string>(target);
    writer.write<std::string>(text);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::send_spawn_request() {
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kClientSpawn);
    std::string name = Game::account_logged_in ? Game::username : Game::guest_play_name();
    writer.write<std::string>(name);
    uint8_t map_id = Game::selected_map;
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    writer.write<uint8_t>(map_id);
    socket.send(writer.packet, writer.at - writer.packet);
    spawn_request_at = Game::timestamp;
}

void Game::spawn_in() {
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    if (Game::alive()) return;
    if (!socket.ready) {
        account_status = I18n::use_chinese ? "正在连接服务器，请稍后重试" : "Connecting to server; please try again";
        return;
    }
    if (pending_auto_login) {
        spawn_after_auth = 1;
        account_status = I18n::use_chinese ? "账号恢复后自动出生" : "Spawning after account restore";
        return;
    }
    // Debounce rapid Continue / Enter / auto-respawn taps (mobile double-fire)
    static double last_spawn_req = -1e9;
    if (timestamp - last_spawn_req < 350.0) return;
    last_spawn_req = timestamp;
    // Intentional spawn from title / death Continue — cancel soft-leave guard
    leaving_to_title = 0;
    // Death "Continue" and title Spawn both go through here: leave death UI then spawn
    if (Game::on_game_screen != 0) {
        Game::on_game_screen = 0;
        awaiting_party_revive = 0;
        revive_progress = 0;
    }
    // Drop stale touches so joystick / attack don't carry into the next life
    Input::touches.clear();
    Input::game_inputs.x = 0;
    Input::game_inputs.y = 0;
    Input::game_inputs.flags = 0;
    account_status.clear();
    // Re-sync guest bag/coins before spawn so reconnect/respawn keeps progress
    if (!account_logged_in && guest_save_valid)
        send_guest_progress();
    send_spawn_request();
    spawn_retries = 0;
}

void Game::party_invite_nearest() {
    party_invite_name(std::string{});
}

void Game::party_invite_name(std::string const &name) {
    if (!socket.ready || !alive()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPartyInvite);
    writer.write<std::string>(name);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::party_leave() {
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPartyLeave);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::account_auth(uint8_t is_register) {
    if (!socket.ready) {
        account_status = I18n::tr(I18n::kAuthNeedConnect);
        return;
    }
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kAccountAuth);
    writer.write<uint8_t>(is_register ? AccountAuthMode::kAccountRegister : AccountAuthMode::kAccountLogin);
    writer.write<std::string>(username);
    writer.write<std::string>(password);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::account_resume() {
    if (!socket.ready || session_token == 0 || username.empty()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kAccountAuth);
    writer.write<uint8_t>(AccountAuthMode::kAccountResume);
    writer.write<std::string>(username);
    writer.write<std::string>(std::to_string(session_token));
    pending_auto_login = 1;
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::account_logout() {
    if (!socket.ready) {
        account_status = I18n::tr(I18n::kAuthNeedConnect);
        return;
    }
    if (!account_logged_in) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kAccountLogout);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::delete_petal(uint8_t pos) {
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    if (!Game::alive()) return;
    writer.write<uint8_t>(Serverbound::kPetalDelete);
    writer.write<uint8_t>(pos);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::swap_petals(uint8_t pos1, uint8_t pos2) {
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    // Alive: flower loadout. Dead/title: camera inventory (server handles both).
    if (!socket.ready || !simulation_ready) return;
    if (!simulation.ent_exists(camera_id)) return;
    writer.write<uint8_t>(Serverbound::kPetalSwap);
    writer.write<uint8_t>(pos1);
    writer.write<uint8_t>(pos2);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::save_loadout_preset(uint8_t index) {
    if (index >= LOADOUT_PRESET_COUNT) return;
    if (!account_logged_in) {
        loadout_preset_status = I18n::use_chinese ? "请先登录账号以保存云端方案" : "Sign in to save cloud presets";
        return;
    }
    if (!socket.ready || !simulation_ready || !simulation.ent_exists(camera_id)) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kLoadoutPresetSave);
    writer.write<uint8_t>(index);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::apply_loadout_preset(uint8_t index) {
    if (index >= LOADOUT_PRESET_COUNT) return;
    if (!account_logged_in) {
        loadout_preset_status = I18n::use_chinese ? "请先登录账号以使用云端方案" : "Sign in to use cloud presets";
        return;
    }
    if (!socket.ready || !simulation_ready || !simulation.ent_exists(camera_id)) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kLoadoutPresetApply);
    writer.write<uint8_t>(index);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::withdraw_bank_petal(PetalID::T id) {
    if (!socket.ready || id == PetalID::kNone || id >= PetalID::kNumPetals
        || craft_bank[id] == 0) return;
    if (Recipe::is_bag_only(id)) return;
    uint8_t n = loadout_count;
    if (n == 0 || n > MAX_SLOT_COUNT) return;
    // Optimistic auto-equip — same priority as server.
    uint8_t dest = (uint8_t) -1;
    for (uint8_t i = 0; i < n; ++i)
        if (cached_loadout[i] == PetalID::kNone) { dest = i; break; }
    if (dest == (uint8_t) -1)
        for (uint8_t i = 0; i < n; ++i)
            if (cached_loadout[i] == PetalID::kBasic) { dest = i; break; }
    if (dest == (uint8_t) -1)
        for (uint8_t i = 0; i < n; ++i) {
            uint8_t s = (uint8_t) (n + i);
            if (cached_loadout[s] == PetalID::kNone) { dest = s; break; }
        }
    if (dest == (uint8_t) -1)
        for (uint8_t i = 0; i < n; ++i)
            if (cached_loadout[i] != id) { dest = i; break; }
    if (dest == (uint8_t) -1) return;
    PetalID::T old = cached_loadout[dest];
    if (old != PetalID::kNone && old != PetalID::kBasic) {
        if (craft_bank[old] >= 1000000) return;
        ++craft_bank[old];
    }
    --craft_bank[id];
    cached_loadout[dest] = id;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kBankWithdraw);
    writer.write<PetalID::T>(id);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::deposit_bank_petal(uint8_t static_pos) {
    if (!socket.ready || !simulation_ready) return;
    if (!simulation.ent_exists(camera_id)) return;
    if (static_pos >= 2 * MAX_SLOT_COUNT) return;
    PetalID::T id = cached_loadout[static_pos];
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    // Optimistic — backpack panel reads stacks + slot copies together.
    cached_loadout[static_pos] = PetalID::kNone;
    if (craft_bank[id] < 1000000) ++craft_bank[id];
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kBankDeposit);
    writer.write<uint8_t>(static_pos);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::set_bonus_enabled(uint8_t on) {
    bonus_enabled = on ? 1 : 0;
    if (!socket.ready || !account_logged_in) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kBoostToggle);
    writer.write<uint8_t>(bonus_enabled);
    socket.send(writer.packet, writer.at - writer.packet);
}

uint8_t Game::cycle_next_loadout_preset() {
    if (!account_logged_in) {
        loadout_preset_status = I18n::use_chinese ? "请先登录账号以使用云端方案" : "Sign in to use cloud presets";
        return 0;
    }
    uint8_t start = active_loadout_preset < LOADOUT_PRESET_COUNT
        ? active_loadout_preset : (uint8_t) (LOADOUT_PRESET_COUNT - 1);
    for (uint8_t step = 1; step <= LOADOUT_PRESET_COUNT; ++step) {
        uint8_t idx = (uint8_t) ((start + step) % LOADOUT_PRESET_COUNT);
        if (loadout_presets[idx].slot_count == 0) continue;
        apply_loadout_preset(idx);
        return 1;
    }
    loadout_preset_status = I18n::use_chinese ? "还没有已保存的方案" : "No saved presets yet";
    return 0;
}

uint8_t Game::try_mobile_l_preset_slot(uint8_t primary_index) {
    if (!mobile_l_preset_pick) return 0;
    // L bank = presets 10..19 (L1..L0). One primary slot per L key.
    if (primary_index >= MAX_SLOT_COUNT) return 0;
    apply_loadout_preset((uint8_t) (10 + primary_index));
    mobile_l_preset_pick = 0;
    return 1;
}

void Game::swap_all_petals() {
    // One-key swap every primary ↔ paired secondary (1:1). Update local cache
    // first so UI stays consistent; do not remap petal_slots pointers mid-loop.
    if (!Game::alive()) return;
    for (uint32_t i = 0; i < Game::loadout_count; ++i) {
        uint8_t p = (uint8_t) i;
        uint8_t s = (uint8_t) (i + Game::loadout_count);
        if (s >= 2 * MAX_SLOT_COUNT) break;
        PetalID::T a = Game::cached_loadout[p];
        PetalID::T b = Game::cached_loadout[s];
        if (a == b) continue;
        Game::cached_loadout[p] = b;
        Game::cached_loadout[s] = a;
        Ui::UiLoadoutPetal *slot_p = Ui::UiLoadout::petal_slots[p];
        Ui::UiLoadoutPetal *slot_s = Ui::UiLoadout::petal_slots[s];
        if (slot_p != nullptr) {
            slot_p->petal_id = b;
            if (b != PetalID::kNone) slot_p->last_id = b;
            slot_p->no_change_ticks = 100;
        }
        if (slot_s != nullptr) {
            slot_s->petal_id = a;
            if (a != PetalID::kNone) slot_s->last_id = a;
            slot_s->no_change_ticks = 100;
        }
        Game::swap_petals(p, s);
    }
}

void Game::craft_petals(uint8_t mode, PetalID::T petal_id) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(mode);
    writer.write<PetalID::T>(petal_id);
    socket.send(writer.packet, writer.at - writer.packet);
}

uint32_t Game::craft_bag_count(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (loadout_count == 0 || loadout_count > MAX_SLOT_COUNT) return 0;
    uint32_t n = 0;
    // Secondary [n,2n) + pure bag [2n, n+MAX) — craftable pool (not primary)
    uint32_t begin = loadout_count;
    uint32_t end = loadout_count + MAX_SLOT_COUNT;
    if (end > 2 * MAX_SLOT_COUNT) end = 2 * MAX_SLOT_COUNT;
    for (uint32_t i = begin; i < end; ++i)
        if (cached_loadout[i] == id) ++n;
    return n;
}

uint32_t Game::craft_owned(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    return craft_bag_count(id) + craft_bank[id];
}

uint32_t Game::craft_available(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    uint32_t n = craft_owned(id);
    if (craft_tray_id == id && craft_tray_count > 0) {
        if (craft_tray_count >= n) return 0;
        return n - craft_tray_count;
    }
    return n;
}

void Game::craft_tray_clear() {
    craft_tray_id = PetalID::kNone;
    craft_tray_count = 0;
}

void Game::craft_tray_add(PetalID::T id, uint32_t amount) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals || amount == 0) return;
    if (!Craft::can_upgrade_petal(id)) return;
    if (craft_tray_id != id) {
        craft_tray_id = id;
        craft_tray_count = 0;
    }
    uint32_t owned = craft_owned(id);
    uint32_t room = owned > craft_tray_count ? owned - craft_tray_count : 0;
    if (amount > room) amount = room;
    craft_tray_count += amount;
}

void Game::craft_buy_rarity(uint8_t rarity) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    if (rarity >= RarityID::kNumRarities) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftBuyRarity);
    writer.write<uint16_t>(rarity);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::craft_buy_special(PetalID::T id) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftBuySpecial);
    writer.write<PetalID::T>(id);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::craft_recipe(uint16_t recipe_index) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftRecipe);
    writer.write<uint16_t>(recipe_index);
    socket.send(writer.packet, writer.at - writer.packet);
}

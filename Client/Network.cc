#include <Client/Game.hh>
#include <Client/DOM.hh>

#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Storage.hh>
#include <Client/Ui/InGame/GameInfo.hh>
#include <Client/Ui/InGame/Loadout.hh>
#include <Client/Ui/TextInput.hh>
#include <Client/Ui/Ui.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/NetworkView.hh>
#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <string>

using namespace Game;

bool Game::near_training_dummies() {
    if (!alive() || simulation.arena_info.map_id != MapID::kGarden) return false;
    auto const &player = simulation.get_ent(player_id);
    float const px = player.get_x();
    float const py = player.get_y();
    // Dummies sit on a 960×720 ring around spawn; only show reset when near one.
    constexpr float kNear = 380.f;
    constexpr float kNearSq = kNear * kNear;
    for (uint8_t rarity = 0; rarity < RarityID::kNumRarities; ++rarity) {
        float const angle = -(float)M_PI / 2
            + rarity * (2.f * (float)M_PI / RarityID::kNumRarities);
        float const dx = px - (MAP_SPAWN_X + std::cos(angle) * 960.f);
        float const dy = py - (MAP_SPAWN_Y + std::sin(angle) * 720.f);
        if (dx * dx + dy * dy <= kNearSq) return true;
    }
    return false;
}

void Game::reset_training() {
    if (!socket.ready) return;
    training_readouts = {};
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kTrainingReset);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::on_message(uint8_t *ptr, uint32_t len) {
    Reader reader(ptr);
    switch(reader.read<uint8_t>()) {
        case Clientbound::kTrainingStats:
            for (auto &stats : training_readouts) {
                stats.last = reader.read<std::string>();
                stats.dps = reader.read<std::string>();
                stats.total = reader.read<std::string>();
            }
            break;
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
                I18n::kAuthLoggedOut,
                I18n::kAuthRateLimited
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
                pending_kick_notice = 0;
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
            // Sparse Unique levels (same layout as kCraftUpdate trailer).
            {
                uint16_t n_levels = reader.read<uint16_t>();
                // Logged-in: high-water merge only. Clearing then applying used
                // to drop Grapes Unique Lv when the sparse trailer omitted it.
                if (!account_logged_in)
                    petal_levels = {};
                for (uint16_t i = 0; i < n_levels; ++i) {
                    PetalID::T lid = reader.read<PetalID::T>();
                    uint32_t lv = reader.read<uint32_t>();
                    if (lid >= PetalID::kNumPetals || lv == 0) continue;
                    PetalID::T dst = Craft::canonical_unique_id(lid);
                    if (dst == PetalID::kNone || dst >= PetalID::kNumPetals)
                        dst = lid;
                    if (lv > petal_levels[dst]) petal_levels[dst] = lv;
                }
                Craft::petal_levels_canonicalize(petal_levels);
                // Warm reconnect cache on successful login. Do NOT write on
                // logout packets (account_logged_in already 0) — that would
                // wipe guest_petal_levels before the logout restore below.
                if (account_logged_in) {
                    for (uint32_t i = 1; i < PetalID::kNumPetals; ++i)
                        if (petal_levels[i] > guest_petal_levels[i])
                            guest_petal_levels[i] = petal_levels[i];
                    Craft::petal_levels_canonicalize(guest_petal_levels);
                } else if (result != AccountResultCode::kAccountLoggedOut) {
                    guest_petal_levels = petal_levels;
                }
            }
            uint64_t token = reader.read<uint64_t>();
            if (account_logged_in)
                session_token = token;
            else if (was_auto || result == AccountResultCode::kAccountLoggedOut ||
                     result == AccountResultCode::kAccountWrongPass ||
                     result == AccountResultCode::kAccountNotFound ||
                     result == AccountResultCode::kAccountInvalidUser)
                session_token = 0;
            // Persist Unique Lv reconnect cache (and session fields) immediately.
            if (account_logged_in)
                Storage::set();
            // Explicit logout → back to guest local progress (account row kept on server)
            if (result == AccountResultCode::kAccountLoggedOut) {
                account_logged_in = 0;
                daily_bonus_seconds = 0;
                offline_bonus_seconds = 0;
                bonus_enabled = 0;
                auth_panel = 0;
                password.clear();
                session_token = 0;
                craft_tray_clear();
                craft_result_clear();
                ensure_guest_id();
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    talent_ranks[i] = guest_talent_ranks[i];
                talent_pending = 0;
                talent_status.clear();
                if (guest_save_valid) {
                    respawn_level = guest_level < 1 ? 1 : guest_level;
                    craft_bank = guest_craft_bank;
                    petal_levels = guest_petal_levels;
                    coins = guest_coins;
                } else {
                    respawn_level = 1;
                    craft_bank = {};
                    petal_levels = {};
                    coins = 0;
                }
                on_game_screen = 0;
                send_guest_progress();
            } else if (was_auto && !account_logged_in) {
                // Resume/login failed → guest this connection; open form if credentials rejected
                // or stuck "already online" so the player can password-login / retry.
                ensure_guest_id();
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    talent_ranks[i] = guest_talent_ranks[i];
                send_guest_progress();
                if (result == AccountResultCode::kAccountWrongPass ||
                    result == AccountResultCode::kAccountNotFound ||
                    result == AccountResultCode::kAccountInvalidUser ||
                    result == AccountResultCode::kAccountInvalidPass ||
                    result == AccountResultCode::kAccountAlreadyIn)
                    auth_panel = 1;
            }
            if (spawn_after_auth) {
                spawn_after_auth = 0;
                spawn_in();
            }
            break;
        }
        case Clientbound::kSpawnResult: {
            uint8_t result = reader.read<uint8_t>();
            if (result == SpawnResultCode::kSpawnNameTaken) {
                // Unlock death Continue — otherwise spawn_request_at hides UI forever.
                spawn_request_at = 0;
                spawn_retries = 0;
                account_status = I18n::tr(I18n::kSpawnNameTaken);
            } else if (result == SpawnResultCode::kSpawnGuestIdAssigned) {
                // Server reminted G#… after collision — keep localStorage in sync.
                std::string id;
                reader.read(id);
                if (id.size() == 8 && id[0] == 'G' && id[1] == '#') {
                    guest_id = id;
                    if (!account_logged_in)
                        nickname = id;
                    account_status.clear();
                    Storage::set();
                }
            }
            break;
        }
        case Clientbound::kCraftUpdate: {
            uint8_t result = reader.read<uint8_t>();
            PetalID::T petal = reader.read<PetalID::T>();
            uint16_t amount = reader.read<uint16_t>();
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                craft_bank[i] = reader.read<uint32_t>();
            coins = reader.read<uint32_t>();
            {
                uint16_t n_levels = reader.read<uint16_t>();
                // Logged-in: never wipe — only raise. Omitting Grapes from a
                // partial/desynced trailer used to zero only that family's Lv.
                if (!account_logged_in)
                    petal_levels = {};
                for (uint16_t i = 0; i < n_levels; ++i) {
                    PetalID::T lid = reader.read<PetalID::T>();
                    uint32_t lv = reader.read<uint32_t>();
                    if (lid >= PetalID::kNumPetals || lv == 0) continue;
                    PetalID::T dst = Craft::canonical_unique_id(lid);
                    if (dst == PetalID::kNone || dst >= PetalID::kNumPetals)
                        dst = lid;
                    if (lv > petal_levels[dst]) petal_levels[dst] = lv;
                }
                Craft::petal_levels_canonicalize(petal_levels);
                if (account_logged_in) {
                    for (uint32_t i = 1; i < PetalID::kNumPetals; ++i)
                        if (petal_levels[i] > guest_petal_levels[i])
                            guest_petal_levels[i] = petal_levels[i];
                    Craft::petal_levels_canonicalize(guest_petal_levels);
                } else {
                    guest_petal_levels = petal_levels;
                }
            }
            // Optional tail: material + roll count for All batches.
            // ONLY Miss/Win/LevelWin may advance the pity counter. BankSync /
            // shop / errors also carry a zero tail; if client↔server petal
            // counts drift, those bytes mis-decode as huge attempts and the
            // counter climbs on every loot pickup with an empty tray.
            uint8_t const craft_roll = result == CraftResultCode::kCraftMiss
                || result == CraftResultCode::kCraftWin
                || result == CraftResultCode::kCraftLevelWin;
            if (reader.at < ptr + len) {
                PetalID::T material = reader.read<PetalID::T>();
                uint32_t attempts = reader.read<uint32_t>();
                if (craft_roll
                    && material != PetalID::kNone && material < PetalID::kNumPetals
                    && attempts > 0) {
                    if (craft_attempt_id != material) {
                        craft_attempt_id = material;
                        craft_attempt_n = 0;
                    }
                    // One request caps at uint16 batches; refuse desync spikes.
                    uint32_t add = attempts > 65535u ? 65535u : attempts;
                    craft_attempt_n += std::min(add, 4000000000u - craft_attempt_n);
                    // Success ends this pity streak (next place starts at #1).
                    if (result == CraftResultCode::kCraftWin
                        || result == CraftResultCode::kCraftLevelWin)
                        craft_attempt_n = 0;
                }
            } else if (craft_roll) {
                if (craft_attempt_n < 4000000000u) ++craft_attempt_n;
                if (result == CraftResultCode::kCraftWin
                    || result == CraftResultCode::kCraftLevelWin)
                    craft_attempt_n = 0;
            }
            // Drop inflated pity left by older desync (loot BankSync mis-reads).
            if (!craft_roll && craft_attempt_n > 1000000u)
                craft_attempt_n = 0;
            // Tray is updated optimistically on craft click; only clamp to owned
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
            else if (result == CraftResultCode::kCraftMiss) {
                uint32_t lost = amount > 0 ? amount : (uint32_t) petal;
                if (lost < Craft::kCraftFailLossMin || lost > Craft::kCraftFailLossMax)
                    lost = Craft::kCraftFailLossMin;
                craft_status = I18n::use_chinese
                    ? std::format("合成失败，损失 {} 片", lost)
                    : std::format("Craft failed — lost {}", lost);
            }
            else if (result == CraftResultCode::kCraftWin) {
                // Synthesis: park product on tray until player taps center.
                uint32_t n = amount > 0 ? amount : 1u;
                craft_result_set(petal, n);
                craft_status = I18n::use_chinese
                    ? std::format("成功！{} ×{} — 点击托盘中央取下", I18n::petal_name(petal), n)
                    : std::format("Success! {} ×{} — tap center to take", I18n::petal_name(petal), n);
            }
            else if (result == CraftResultCode::kCraftLevelWin) {
                craft_status = I18n::use_chinese
                    ? std::format("升级成功！{} 达到 Lv.{}", I18n::petal_name(petal), amount)
                    : std::format("Level up! {} is now Lv.{}", I18n::petal_name(petal), amount);
            }
            else if (result == CraftResultCode::kCraftExchanged) {
                // Exchange / shop / assemble: already in bank — no tray claim.
                uint32_t n = amount > 0 ? amount : 1u;
                craft_status = I18n::use_chinese
                    ? std::format("直兑获得 {} ×{}", I18n::petal_name(petal), n)
                    : std::format("Exchanged {} ×{}", I18n::petal_name(petal), n);
            }
            else if (result == CraftResultCode::kCraftBought) {
                if (petal == PetalID::kNone)
                    craft_status = I18n::use_chinese
                        ? "全服广播已发送" : "Server announce sent";
                else {
                    uint32_t n = amount > 0 ? amount : 1u;
                    craft_status = I18n::use_chinese
                        ? std::format("购买成功！{} ×{}", I18n::petal_name(petal), n)
                        : std::format("Bought {} ×{}", I18n::petal_name(petal), n);
                }
            } else if (result == CraftResultCode::kCraftInvFull)
                craft_status = I18n::use_chinese
                    ? "背包已满，先腾出空位"
                    : "Backpack full — free a slot first";
            if (petal < PetalID::kNumPetals && petal != PetalID::kNone)
                seen_petals[petal] = 1;
            if ((result == CraftResultCode::kCraftWin
                || result == CraftResultCode::kCraftMiss
                || result == CraftResultCode::kCraftExchanged
                || result == CraftResultCode::kCraftBought
                || result == CraftResultCode::kCraftLevelWin)
                && petal != PetalID::kNone)
                Ui::craft_play_result(result, petal, amount > 0 ? amount : 1u);
            break;
        }
        case Clientbound::kBoostUpdate: {
            daily_bonus_seconds = reader.read<uint32_t>();
            offline_bonus_seconds = reader.read<uint32_t>();
            bonus_enabled = reader.read<uint8_t>() ? 1 : 0;
            bonus_synced_at = timestamp;
            break;
        }
        case Clientbound::kOnlineRoster: {
            uint8_t n = reader.read<uint8_t>();
            online_roster.clear();
            online_roster.reserve(n);
            for (uint8_t i = 0; i < n; ++i) {
                OnlineRosterEntry e;
                e.name = reader.read<std::string>();
                e.map_id = reader.read<uint8_t>();
                e.guild = reader.read<std::string>();
                e.level = reader.read<uint32_t>();
                if (e.map_id >= MapID::kNumMaps) e.map_id = 0;
                if (e.level < 1) e.level = 1;
                online_roster.push_back(std::move(e));
            }
            break;
        }
        case Clientbound::kTalentUpdate: {
            uint8_t got_ranks[TalentID::kNum] = {};
            for (uint8_t i = 0; i < TalentID::kNum; ++i)
                got_ranks[i] = reader.read<uint8_t>();
            if (talent_pending) {
                uint8_t ok = 0;
                if (talent_pending_id == 0xFF) {
                    ok = 1;
                    for (uint8_t i = 0; i < TalentID::kNum; ++i)
                        if (got_ranks[i] != 0) { ok = 0; break; }
                } else if (talent_pending_id < TalentID::kNum) {
                    // talent_pending_rank holds the expected absolute target
                    ok = got_ranks[talent_pending_id] == talent_pending_rank;
                }
                talent_status = ok
                    ? (I18n::use_chinese ? "天赋已更新" : "Talents updated")
                    : (I18n::use_chinese ? "操作未生效，请查看等级或点数" : "No change; check level and points");
                talent_pending = 0;
            }
            for (uint8_t i = 0; i < TalentID::kNum; ++i)
                talent_ranks[i] = got_ranks[i];
            if (!account_logged_in) {
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    guest_talent_ranks[i] = got_ranks[i];
            }
            break;
        }
        case Clientbound::kFeedbackResult: {
            uint8_t result = reader.read<uint8_t>();
            uint32_t id = reader.read<uint32_t>();
            DOM::feedback_result(result, id);
            break;
        }
        case Clientbound::kFeedbackResolveResult: {
            uint8_t result = reader.read<uint8_t>();
            uint32_t id = reader.read<uint32_t>();
            uint8_t resolved = reader.read<uint8_t>() ? 1 : 0;
            DOM::feedback_resolve_result(result, id, resolved);
            break;
        }
        case Clientbound::kLoadoutPresetSync: {
            uint8_t result = reader.read<uint8_t>();
            uint8_t index = reader.read<uint8_t>();
            for (LoadoutPreset &preset : loadout_presets) {
                preset.secondary.fill(PetalID::kNone);
                preset.slot_count = reader.read<uint8_t>();
                if (preset.slot_count > MAX_SLOT_COUNT) preset.slot_count = 0;
                for (PetalID::T &id : preset.petals) {
                    id = reader.read<PetalID::T>();
                    if (id >= PetalID::kNumPetals) id = PetalID::kNone;
                }
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    preset.talent_ranks[i] = reader.read<uint8_t>();
            }
            // Older servers omit this extension; both rows use the same preset.
            if (reader.at < ptr + len && reader.read<uint8_t>() == 1) {
                Validator secondary_data(reader.at, ptr + len);
                bool valid = true;
                for (uint32_t i = 0; i < LOADOUT_PRESET_COUNT * MAX_SLOT_COUNT; ++i)
                    if (!secondary_data.validate_uint16()) { valid = false; break; }
                if (valid)
                    for (LoadoutPreset &preset : loadout_presets)
                        for (PetalID::T &id : preset.secondary) {
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
            EntityID new_camera = reader.read<EntityID>();
            // Soft-leave keeps the old camera + world. Map transfer / new
            // instance gives a different camera and all-creates; wipe stale
            // ents or force_alloc_ent collides with occupied ID slots.
            if (!(new_camera == camera_id) || !simulation.ent_exists(new_camera))
                simulation.reset();
            camera_id = new_camera;
            EntityID curr_id = reader.read<EntityID>();
            while(!(curr_id == NULL_ENTITY)) {
                // The id may already be gone locally (deleted on a pending
                // delete tick). Deleting it twice would desync the id/hash
                // table and can leave the camera without a player.
                if (simulation.ent_exists(curr_id))
                    simulation._delete_ent(curr_id);
                curr_id = reader.read<EntityID>();
            }
            // Invariant after a successful snapshot: local ents == this
            // packet's in_view. Generation stamp avoids zeroing 8KB every packet.
            static uint32_t snap_gen = 1;
            static uint32_t seen_gen[ENTITY_CAP] = {};
            if (++snap_gen == 0) {
                std::memset(seen_gen, 0, sizeof(seen_gen));
                snap_gen = 1;
            }
            curr_id = reader.read<EntityID>();
            while(!(curr_id == NULL_ENTITY)) {
                uint8_t create = reader.read<uint8_t>();
                if (BitMath::at(create, 0)) simulation.force_alloc_ent(curr_id);
                assert(simulation.ent_exists(curr_id));
                Entity &ent = simulation.get_ent(curr_id);
                ent.read(&reader, BitMath::at(create, 0));
                if (BitMath::at(create, 1)) ent.pending_delete = 1;
                if (curr_id.id < ENTITY_CAP)
                    seen_gen[curr_id.id] = snap_gen;
                curr_id = reader.read<EntityID>();
            }
            uint32_t const gen = snap_gen;
            simulation.for_each_entity([&](Simulation *sim, Entity &ent) {
                if (ent.id.id >= ENTITY_CAP) return;
                if (seen_gen[ent.id.id] == gen) return;
                sim->_delete_ent(ent.id);
            });
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
            std::string whisper_to;
            reader.read<std::string>(sender);
            reader.read<std::string>(text);
            reader.read<std::string>(whisper_to);
            push_chat_line(channel, sender, text, whisper_to);
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
        case Clientbound::kLootToast: {
            PetalID::T id = reader.read<PetalID::T>();
            uint16_t amount = reader.read<uint16_t>();
            std::string title;
            reader.read<std::string>(title);
            loot_toast.petal_id = id;
            loot_toast.amount = amount;
            loot_toast.title = title;
            loot_toast.expires_at = timestamp + LOOT_TOAST_MS;
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
            reader.read<std::string>(guild_announcement);
            // Keep draft in sync unless the leader is mid-edit in the panel input
            if (!guild_panel_open || !guild_self_is_leader()
                || Ui::active_text_input() == nullptr)
                guild_anno_draft = guild_announcement;
            uint8_t count = reader.read<uint8_t>();
            guild_member_count = count;
            guild_members.clear();
            guild_status.clear();
            for (uint8_t i = 0; i < count; ++i) {
                GuildMember m;
                reader.read<std::string>(m.name);
                m.flags = reader.read<uint8_t>();
                m.level = reader.read<uint32_t>();
                m.x = reader.read<float>();
                m.y = reader.read<float>();
                guild_members.push_back(m);
                if (!guild_status.empty()) guild_status += ", ";
                if (m.flags & 1) guild_status += "*";
                if (!(m.flags & 2)) guild_status += "?";
                guild_status += m.name;
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

void Game::sync_auto_party_opt() {
    static uint8_t last_sent = 255;
    if (!socket.ready) {
        last_sent = 255;
        return;
    }
    if (Input::disable_auto_party == last_sent) return;
    last_sent = Input::disable_auto_party;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kAutoPartyOpt);
    writer.write<uint8_t>(Input::disable_auto_party ? 1 : 0);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::send_inputs() {
    // Chat freezes only keyboard-move + mobile (WASD/joystick type names).
    // Mouse-aim keeps steering while the chat field is focused.
    // Pause stops movement but still sends attack/defend so ESC invert toggles
    // take effect immediately (hold=cancel always-on).
    uint8_t move_blocked = Input::freeze_input || Game::pause_menu_open
        || Game::leave_confirm_open || Game::game_panel_blocking()
        || Game::guild_panel_open || Game::invite_open || Game::chat_menu_open
        || Game::update_poster_open
        || Ui::panel_open != Ui::Panel::kNone
        || (Game::chat_open && (Input::mobile_controls() || Input::keyboard_movement));
    uint8_t flags_blocked = Input::freeze_input || Game::leave_confirm_open
        || Game::game_panel_blocking()
        || Game::guild_panel_open || Game::invite_open || Game::chat_menu_open
        || Game::update_poster_open
        || Ui::panel_open != Ui::Panel::kNone
        || (Game::chat_open && (Input::mobile_controls() || Input::keyboard_movement));
    float x = move_blocked ? 0.f : Input::game_inputs.x;
    float y = move_blocked ? 0.f : Input::game_inputs.y;
    uint8_t flags = flags_blocked ? 0 : Input::game_inputs.flags;
    static double last_sent_at = -1000;
    static float last_x = 0, last_y = 0;
    static uint8_t last_flags = 0, last_rot = 0;
    uint16_t view_w = NetworkView::screen_half(renderer.width, Ui::scale, camera_zoom);
    uint16_t view_h = NetworkView::screen_half(renderer.height, Ui::scale, camera_zoom);
    static uint16_t last_view_w = 0, last_view_h = 0;
    // Send starts, stops, attacks, and large direction changes immediately.
    // Steady input only needs one packet per 25 TPS server step.
    uint8_t changed = flags != last_flags || rot_speed_setting != last_rot
        || view_w != last_view_w || view_h != last_view_h
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
    writer.write<uint16_t>(view_w);
    writer.write<uint16_t>(view_h);
    socket.send(writer.packet, writer.at - writer.packet);
    last_sent_at = Game::timestamp;
    last_x = x;
    last_y = y;
    last_flags = flags;
    last_rot = rot_speed_setting;
    last_view_w = view_w;
    last_view_h = view_h;
}

void Game::talent_spend(uint8_t talent_id) {
    if (talent_id >= TalentID::kNum) return;
    talent_set_rank(talent_id, (uint8_t) (talent_ranks[talent_id] + 1));
}

void Game::talent_refund(uint8_t talent_id) {
    if (talent_pending) return;
    if (talent_id == 0xFF) {
        if (!socket.ready || !simulation_ready || !simulation.ent_exists(camera_id)) {
            talent_status = I18n::use_chinese ? "未连接，暂不能回退" : "Connect before refunding";
            return;
        }
        talent_pending = 1;
        talent_pending_id = 0xFF;
        talent_pending_rank = 0;
        talent_status = I18n::use_chinese ? "回退中…" : "Refunding…";
        Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
        writer.write<uint8_t>(Serverbound::kTalentRefund);
        writer.write<uint8_t>(talent_id);
        socket.send(writer.packet, writer.at - writer.packet);
        return;
    }
    if (talent_id >= TalentID::kNum) return;
    if (talent_ranks[talent_id] == 0) {
        talent_status = I18n::use_chinese ? "该天赋尚无点数" : "Nothing to refund";
        return;
    }
    talent_set_rank(talent_id, (uint8_t) (talent_ranks[talent_id] - 1));
}

void Game::talent_set_rank(uint8_t talent_id, uint8_t target_rank) {
    if (talent_id >= TalentID::kNum || talent_pending) return;
    if (!socket.ready || !simulation_ready || !simulation.ent_exists(camera_id)) {
        talent_status = I18n::use_chinese ? "未连接，暂不能加点" : "Connect before upgrading";
        return;
    }
    uint32_t level = alive() ? score_to_level(score)
        : simulation.get_ent(camera_id).get_respawn_level();
    uint8_t cur = talent_ranks[talent_id];
    if (target_rank == cur) return;
    if (target_rank > Talent::max_rank_for(talent_id, level)) {
        talent_status = I18n::use_chinese ? "当前等级的天赋阶数已满" : "Rank limit reached for this level";
        return;
    }
    if (target_rank > cur
        && Talent::points_free(level, talent_ranks)
            < Talent::cost_to_reach(cur, target_rank)) {
        talent_status = I18n::use_chinese ? "天赋点不足" : "Not enough talent points";
        return;
    }
    talent_pending = 1;
    talent_pending_id = talent_id;
    talent_pending_rank = target_rank;
    talent_status = target_rank > cur
        ? (I18n::use_chinese ? "加点中…" : "Upgrading…")
        : (I18n::use_chinese ? "回退中…" : "Refunding…");
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kTalentSetRank);
    writer.write<uint8_t>(talent_id);
    writer.write<uint8_t>(target_rank);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::send_chat() {
    if (!can_chat() || !chat_open) return;
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->sync_from_dom();
    std::string text = chat_draft;
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.erase(text.begin());
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.pop_back();
    uint8_t channel = chat_channel;
    std::string target = (channel == ChatChannel::kChatWhisper) ? chat_whisper_to : std::string{};
    close_chat(); // preserves draft if we bail below
    if (text.empty() || !socket.ready) return;
    if (channel == ChatChannel::kChatWhisper && target.empty()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kChat);
    writer.write<uint8_t>(channel);
    writer.write<std::string>(target);
    writer.write<std::string>(text);
    socket.send(writer.packet, writer.at - writer.packet);
    // Only wipe draft after a real send
    if (Ui::TextInput *input = Ui::chat_text_input())
        input->clear();
    else
        chat_draft.clear();
}

void Game::send_spawn_request() {
    if (!socket.ready) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kClientSpawn);
    std::string name = Game::account_logged_in ? Game::username : Game::guest_play_name();
    writer.write<std::string>(name);
    uint8_t map_id = clamp_to_open_map(Game::selected_map);
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
    // Keep on_game_screen during death→respawn so transition_circle / title UI
    // do not collapse (that flash was the home screen). Death panel hides while
    // spawn_request_at > 0 instead.
    awaiting_party_revive = 0;
    revive_progress = 0;
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
    // Account progress stays on the server; guest localStorage is frozen while
    // logged in, so seed it now or logout falls back to an empty guest (lv 1).
    seed_guest_from_session();
    Storage::set();
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

void Game::post_feedback(std::string const &text) {
    if (!account_logged_in) { DOM::feedback_result(kFeedbackNeedAccount, 0); return; }
    if (!socket.ready) { DOM::feedback_result(kFeedbackStorageError, 0); return; }
    if (text.empty() || text.size() > 960) { DOM::feedback_result(kFeedbackInvalid, 0); return; }
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kFeedbackPost);
    writer.write<std::string>(text);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::set_feedback_resolved(uint32_t id, uint8_t resolved) {
    if (!account_logged_in) {
        DOM::feedback_resolve_result(kFeedbackNeedAccount, id, 0);
        return;
    }
    if (!socket.ready) {
        DOM::feedback_resolve_result(kFeedbackStorageError, id, 0);
        return;
    }
    if (id == 0) {
        DOM::feedback_resolve_result(kFeedbackInvalid, id, 0);
        return;
    }
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kFeedbackSetResolved);
    writer.write<uint32_t>(id);
    writer.write<uint8_t>(resolved ? 1 : 0);
    socket.send(writer.packet, writer.at - writer.packet);
}

uint8_t Game::withdraw_bank_petal(PetalID::T id, uint8_t dest) {
    if (!socket.ready || id == PetalID::kNone || id >= PetalID::kNumPetals
        || craft_bank[id] == 0) return 0;
    if (Recipe::is_bag_only(id)) return 0;
    uint8_t n = loadout_count;
    if (n == 0 || n > MAX_SLOT_COUNT) return 0;
    uint8_t lim = (uint8_t) (n + MAX_SLOT_COUNT);
    if (lim > 2 * MAX_SLOT_COUNT) lim = 2 * MAX_SLOT_COUNT;
    // Explicit slot from a bag drag (may replace); else empty/Basic only.
    if (dest >= lim) {
        dest = (uint8_t) -1;
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
    }
    if (dest == (uint8_t) -1 || dest >= lim) return 0;
    // Coins cannot sit on primary.
    if (dest < n && Recipe::is_bag_only(id)) return 0;
    PetalID::T old = cached_loadout[dest];
    // Same as empty: equip. Real petal already there → bank it first.
    // At stack cap: still replace the slot (do not block equip); bank count stays.
    if (old == PetalID::kBasic) {
        // Basic is filler — drop without banking.
    } else if (old != PetalID::kNone && old < PetalID::kNumPetals) {
        uint32_t cap = Recipe::bank_stack_cap(old);
        if (craft_bank[old] < cap) ++craft_bank[old];
    }
    --craft_bank[id];
    cached_loadout[dest] = id;
    // Hold optimistic equip until the server snapshot matches (same as swaps).
    if (dest < 2 * MAX_SLOT_COUNT) {
        Ui::UiLoadoutPetal *slot = Ui::UiLoadout::petal_slots[dest];
        if (slot != nullptr) {
            slot->petal_id = id;
            slot->last_id = id;
            slot->no_change_ticks = 100;
        }
    }
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kBankWithdraw);
    writer.write<PetalID::T>(id);
    writer.write<uint8_t>(dest);
    socket.send(writer.packet, writer.at - writer.packet);
    return 1;
}

void Game::deposit_bank_petal(uint8_t static_pos) {
    if (!socket.ready || !simulation_ready) return;
    if (!simulation.ent_exists(camera_id)) return;
    if (static_pos >= 2 * MAX_SLOT_COUNT) return;
    PetalID::T id = cached_loadout[static_pos];
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return;
    // Optimistic — always clear the slot. At stack cap, skip the ++ so the
    // bar cannot stick when coins/petals are already maxed.
    cached_loadout[static_pos] = PetalID::kNone;
    uint32_t cap = Recipe::bank_stack_cap(id);
    if (craft_bank[id] < cap) ++craft_bank[id];
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

uint8_t Game::loadout_preview_bank() {
    return preset_ghost_bank != 0 ? preset_ghost_bank : (mobile_l_preset_pick ? 2 : 0);
}

PetalID::T Game::preset_ghost_petal(uint8_t primary_index) {
    uint8_t const bank = loadout_preview_bank();
    if (bank == 0 || primary_index >= MAX_SLOT_COUNT) return PetalID::kNone;
    uint8_t idx = (uint8_t) ((bank == 1 ? 0 : 10) + primary_index);
    if (idx >= LOADOUT_PRESET_COUNT) return PetalID::kNone;
    LoadoutPreset const &p = loadout_presets[idx];
    if (p.slot_count == 0) return PetalID::kNone;
    // Identifier = first saved primary petal in that preset (slot 0, else first non-empty).
    if (p.petals[0] != PetalID::kNone && p.petals[0] < PetalID::kNumPetals)
        return p.petals[0];
    for (uint8_t i = 1; i < p.slot_count && i < MAX_SLOT_COUNT; ++i) {
        if (p.petals[i] != PetalID::kNone && p.petals[i] < PetalID::kNumPetals)
            return p.petals[i];
    }
    return PetalID::kNone;
}

void Game::swap_all_petals() {
    // One-key / mobile R: every primary ↔ paired secondary with cross-fly.
    // Allowed on the death screen so loadout can still be rearranged.
    if (!Game::alive() && !Game::on_game_screen) return;
    for (uint32_t i = 0; i < Game::loadout_count; ++i) {
        uint8_t p = (uint8_t) i;
        uint8_t s = (uint8_t) (i + Game::loadout_count);
        if (s >= 2 * MAX_SLOT_COUNT) break;
        Ui::ui_swap_paired_slots(p, s);
    }
}

void Game::craft_petals(uint8_t mode, PetalID::T petal_id, uint16_t max_batches) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
    if (max_batches == 0) max_batches = 1;
    // uint16 wire; All-drain uses a high cap so miss refunds can keep feeding rolls.
    if (max_batches > 65535) max_batches = 65535;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(mode);
    writer.write<PetalID::T>(petal_id);
    if (mode == CraftMode::kCraftGamble || mode == CraftMode::kCraftGambleAll)
        writer.write<uint16_t>(max_batches);
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

void Game::craft_tray_remove(uint32_t amount) {
    if (amount == 0 || craft_tray_id == PetalID::kNone || craft_tray_count == 0)
        return;
    if (amount >= craft_tray_count) {
        craft_tray_clear();
        return;
    }
    craft_tray_count -= amount;
}

void Game::craft_tray_add(PetalID::T id, uint32_t amount) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals || amount == 0) return;
    if (!Craft::can_craft_petal(id)) return;
    if (craft_tray_id != id) {
        craft_tray_id = id;
        craft_tray_count = 0;
    }
    if (craft_attempt_id != id) {
        craft_attempt_id = id;
        craft_attempt_n = 0;
    } else if (craft_attempt_n > 1000000u) {
        // BankSync desync used to inflate this into the tens of millions.
        craft_attempt_n = 0;
    }
    uint32_t owned = craft_owned(id);
    uint32_t room = owned > craft_tray_count ? owned - craft_tray_count : 0;
    if (amount > room) amount = room;
    craft_tray_count += amount;
}

void Game::craft_result_clear() {
    craft_result_id = PetalID::kNone;
    craft_result_count = 0;
}

void Game::craft_result_take() {
    if (craft_result_id == PetalID::kNone || craft_result_count == 0) return;
    craft_result_clear();
    craft_status = I18n::use_chinese ? "已放入高一阶格子" : "Sent to next rarity";
}

void Game::craft_result_set(PetalID::T id, uint32_t amount) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals || amount == 0) {
        craft_result_clear();
        return;
    }
    // Same product → stack; different → replace (顶掉).
    if (craft_result_id == id)
        craft_result_count += amount;
    else {
        craft_result_id = id;
        craft_result_count = amount;
    }
    if (craft_result_count > 1000000u) craft_result_count = 1000000u;
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

void Game::craft_buy_announce(std::string const &text) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    if (text.empty()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftBuyAnnounce);
    writer.write<uint16_t>(0);
    writer.write<std::string>(text);
    socket.send(writer.packet, writer.at - writer.packet);
    craft_status = I18n::use_chinese ? "正在发送全服广播…" : "Sending server announce…";
}

void Game::craft_recipe(uint16_t recipe_index, uint8_t rarity) {
    if (!socket.ready || !(alive() || on_game_screen)) return;
    if (rarity >= RarityID::kNumRarities) rarity = RarityID::kCommon;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftRecipe);
    writer.write<uint16_t>(recipe_index);
    writer.write<uint8_t>(rarity);
    socket.send(writer.packet, writer.at - writer.packet);
}

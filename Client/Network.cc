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
#include <Shared/StaticDefinitions.hh>

#include <format>

using namespace Game;

void Game::on_message(uint8_t *ptr, uint32_t len) {
    Reader reader(ptr);
    switch(reader.read<uint8_t>()) {
        case Clientbound::kAccountResult: {
            uint8_t result = reader.read<uint8_t>();
            uint8_t level = reader.read<uint8_t>();
            account_logged_in = (result == AccountResultCode::kAccountOk);
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
                if (nickname.empty()) nickname = username;
                Game::respawn_level = level < 1 ? 1 : level;
                auth_panel = 0;
                for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                    PetalID::T id = reader.read<uint8_t>();
                    if (id < PetalID::kNumPetals && id != PetalID::kNone)
                        Game::seen_petals[id] = 1;
                }
            } else {
                for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
                    reader.read<uint8_t>();
            }
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                craft_bank[i] = reader.read<uint32_t>();
            coins = reader.read<uint32_t>();
            // Explicit logout → back to guest local progress (account row kept on server)
            if (result == AccountResultCode::kAccountLoggedOut) {
                account_logged_in = 0;
                auth_panel = 0;
                password.clear();
                craft_tray_clear();
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
            PetalID::T petal = reader.read<uint8_t>();
            for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
                craft_bank[i] = reader.read<uint32_t>();
            coins = reader.read<uint32_t>();
            // Tray is a staging view of banked petals — clamp after server bank sync
            if (result == CraftResultCode::kCraftWin || result == CraftResultCode::kCraftMiss) {
                if (craft_tray_id != PetalID::kNone) {
                    if (craft_tray_count <= Craft::kCraftBatch) craft_tray_clear();
                    else craft_tray_count -= Craft::kCraftBatch;
                }
            }
            if (craft_tray_id != PetalID::kNone) {
                if (craft_bank[craft_tray_id] == 0) craft_tray_clear();
                else if (craft_tray_count > craft_bank[craft_tray_id])
                    craft_tray_count = craft_bank[craft_tray_id];
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
                    ? std::format("背包已满，{} 已转为材料", I18n::petal_name(petal))
                    : std::format("Bag full — {} banked as mats", I18n::petal_name(petal));
            if (petal < PetalID::kNumPetals && petal != PetalID::kNone)
                seen_petals[petal] = 1;
            break;
        }
        case Clientbound::kClientUpdate: {
            simulation_ready = 1;
            camera_id = reader.read<EntityID>();
            EntityID curr_id = reader.read<EntityID>();
            while(!(curr_id == NULL_ENTITY)) {
                assert(simulation.ent_exists(curr_id));
                Entity &ent = simulation.get_ent(curr_id);
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
            awaiting_party_revive = 0;
            for (uint8_t i = 0; i < count; ++i) {
                std::string name;
                reader.read<std::string>(name);
                uint8_t flags = reader.read<uint8_t>();
                if (!party_status.empty()) party_status += ", ";
                if (flags & 1) party_status += "*";
                party_status += name;
                if (flags & 2) party_status += " [DOWN]";
                if ((flags & 4) && (flags & 2)) awaiting_party_revive = 1;
            }
            revive_progress = reader.read<uint8_t>();
            if (count == 0) {
                party_status = I18n::tr(I18n::kPartyEmpty);
                awaiting_party_revive = 0;
                revive_progress = 0;
            }
            break;
        }
        default:
            break;
    }
}

void Game::send_inputs() {
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kClientInput);
    if (Input::freeze_input || Game::pause_menu_open || Game::chat_open
        || Game::leave_confirm_open || Game::game_panel_blocking()) {
        writer.write<float>(0);
        writer.write<float>(0);
        writer.write<uint8_t>(0);
    } else {
        writer.write<float>(Input::game_inputs.x);
        writer.write<float>(Input::game_inputs.y);
        writer.write<uint8_t>(Input::game_inputs.flags);
    }
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

void Game::spawn_in() {
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    if (Game::alive()) return;
    // Death "Continue" and title Spawn both go through here: leave death UI then spawn
    if (Game::on_game_screen != 0) {
        Game::on_game_screen = 0;
        awaiting_party_revive = 0;
        revive_progress = 0;
    }
    if (!socket.ready) return;
    account_status.clear();
    // Re-sync guest bag/coins before spawn so reconnect/respawn keeps progress
    if (!account_logged_in && guest_save_valid)
        send_guest_progress();
    writer.write<uint8_t>(Serverbound::kClientSpawn);
    std::string name = Game::nickname;
    if (name.empty() && Game::account_logged_in) name = Game::username;
    writer.write<std::string>(name);
    uint8_t map_id = Game::selected_map;
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    writer.write<uint8_t>(map_id);
    socket.send(writer.packet, writer.at - writer.packet);
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
    if (!Game::alive()) return;
    writer.write<uint8_t>(Serverbound::kPetalSwap);
    writer.write<uint8_t>(pos1);
    writer.write<uint8_t>(pos2);
    socket.send(writer.packet, writer.at - writer.packet);
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
    if (!socket.ready || !alive()) return;
    if (petal_id == PetalID::kNone || petal_id >= PetalID::kNumPetals) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(mode);
    writer.write<uint8_t>(petal_id);
    socket.send(writer.packet, writer.at - writer.packet);
}

uint32_t Game::craft_available(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    uint32_t n = craft_bank[id];
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
    if (craft_tray_id != id) {
        craft_tray_id = id;
        craft_tray_count = 0;
    }
    uint32_t bank = craft_bank[id];
    uint32_t room = bank > craft_tray_count ? bank - craft_tray_count : 0;
    if (amount > room) amount = room;
    craft_tray_count += amount;
}

void Game::craft_buy_rarity(uint8_t rarity) {
    if (!socket.ready || !alive()) return;
    if (rarity >= RarityID::kNumRarities) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftBuyRarity);
    writer.write<uint8_t>(rarity);
    socket.send(writer.packet, writer.at - writer.packet);
}

void Game::craft_buy_special(PetalID::T id) {
    if (!socket.ready || !alive()) return;
    Writer writer(static_cast<uint8_t *>(OUTGOING_PACKET));
    writer.write<uint8_t>(Serverbound::kPetalCraft);
    writer.write<uint8_t>(CraftMode::kCraftBuySpecial);
    writer.write<uint8_t>(id);
    socket.send(writer.packet, writer.at - writer.packet);
}
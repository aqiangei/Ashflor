#include <Client/Storage.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <cstring>
#include <string>
#include <emscripten.h>

uint32_t const XOR_CYCLE_LENGTH = 7;
static uint8_t const XOR_CYCLE[XOR_CYCLE_LENGTH] = { 0x43, 0x37, 0x92, 0xa4, 0x38, 0xf1, 0x2b };

static void _custom_b16_encode(uint32_t len) {
    for (uint32_t i = len; i > 0; --i) {
        uint8_t c = StorageProtocol::buffer[i - 1];
        StorageProtocol::buffer[2*i-1] = ((c >> 4) & 15) + 'a';
        StorageProtocol::buffer[2*i-2] = (c & 15) + 'a';
    }
}

static void _custom_b16_decode(uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        uint8_t lower = StorageProtocol::buffer[2*i] - 'a';
        uint8_t upper = StorageProtocol::buffer[2*i+1] - 'a';
        if (lower >= 16 || upper >= 16) break;
        StorageProtocol::buffer[i] = lower | (upper << 4);
    }
}

namespace StorageProtocol {
    uint8_t buffer[2 * StorageProtocol::MAX_LENGTH] = {0};

    void store(char const *name, uint32_t len) {
        _custom_b16_encode(len);
        EM_ASM({
            window.localStorage[Module.TextDecoder.decode(HEAPU8.subarray($0,$0+$1))] = Module.TextDecoder.decode(HEAPU8.subarray($2,$2+$3));
        }, name, std::strlen(name), buffer, 2 * len);
    }

    uint32_t retrieve(char const *name, uint32_t max_length) {
        uint32_t len = EM_ASM_INT({
            let a = window.localStorage[Module.TextDecoder.decode(HEAPU8.subarray($0,$0+$1))];
            if (!a) return 0;
            a = a.slice(0, $3);
            HEAPU8.set(new TextEncoder().encode(a), $2);
            return a.length;
        }, name, std::strlen(name), buffer, 2 * max_length) / 2;
        _custom_b16_decode(len);
        return len;
    }

    Encoder::Encoder(uint8_t *ptr) : base(ptr), at(ptr) {}

    template<>
    void Encoder::write<uint8_t>(uint8_t const &o) {
        *at = o ^ (XOR_CYCLE[(at - base) % XOR_CYCLE_LENGTH]);
        ++at;
    }

    template<>
    void Encoder::write<uint16_t>(uint16_t const &val) {
        uint16_t v = val;
        while (v > 127) {
            write<uint8_t>((v & 127) | 128);
            v >>= 7;
        }
        write<uint8_t>(v);
    }

    template<>
    void Encoder::write<uint32_t>(uint32_t const &val) {
        uint32_t v = val;
        while (v > 127)
        {
            write<uint8_t>((v & 127) | 128);
            v >>= 7;
        }
        write<uint8_t>(v);
    }

    template<>
    void Encoder::write<std::string>(std::string const &str) {
        uint32_t len = str.size();
        write<uint32_t>(len);
        for (uint32_t i = 0; i < len; ++i) write<uint8_t>(str[i]);
    }

    Decoder::Decoder(uint8_t const *ptr) : base(ptr), at(ptr) {}

    template<>
    uint8_t Decoder::read<uint8_t>() {
        uint8_t ret = *at ^ (XOR_CYCLE[(at - base) % XOR_CYCLE_LENGTH]);
        ++at;
        return ret;
    }

    template<>
    uint16_t Decoder::read<uint16_t>() {
        uint16_t ret = 0;
        for (uint32_t i = 0; i < 3; ++i) {
            uint8_t o = read<uint8_t>();
            ret |= ((o & 127) << (i * 7));
            if (o <= 127) break;
        }
        return ret;
    }

    template<>
    uint32_t Decoder::read<uint32_t>() {
        uint32_t ret = 0;
        for (uint32_t i = 0; i < 5; ++i) {
            uint8_t o = read<uint8_t>();
            ret |= ((o & 127) << (i * 7));
            if (o <= 127) break;
        }
        return ret;
    }

    template<>
    std::string Decoder::read<std::string>() {
        std::string ref;
        uint32_t len = read<uint32_t>();
        for (uint32_t i = 0; i < len; ++i) ref.push_back(read<uint8_t>());
        return ref;
    }
}

static bool should_update = 0;

template<typename T>
class MutationObserver {
    T prev = {};
public:
    MutationObserver(T const &o) : prev(o) {};
    void cmp(T &o) {
        if (o == prev) return;
        should_update = 1;
        prev = o;
        return;
    }
};

#define STORED \
    X(0, Game::nickname) \
    X(18, Game::guest_id) \
    X(1, Game::seen_mobs) \
    X(2, Game::seen_petals) \
    X(3, Input::keyboard_movement) \
    X(4, Input::movement_helper) \
    X(5, I18n::use_chinese) \
    X(6, Game::username) \
    X(7, Game::guest_save_valid) \
    X(8, Game::guest_level) \
    X(9, Game::guest_petals) \
    X(10, Game::guest_craft_bank) \
    X(11, Game::guest_coins) \
    X(12, Input::invert_attack) \
    X(13, Input::invert_defend) \
    X(22, Input::disable_auto_party) \
    X(19, Input::request_mobile) \
    X(20, Game::password) \
    X(15, Game::guest_talent_ranks) \
    X(17, Game::rot_speed_setting) \
    X(21, Game::show_debug) \
    X(23, Game::show_hitboxes) \
    X(24, Game::show_low_rarity_drops)

#define X(ct, name) static auto checker_##ct = MutationObserver(name);
STORED
#undef X

using namespace StorageProtocol;

void Storage::retrieve() {
    Game::seen_petals[PetalID::kBasic] = 1;
    {
        // v2: 0xfe + (u8 id, u16 rarity_bits)* . Legacy: bare u8 ids.
        uint32_t len = StorageProtocol::retrieve("mobs", 2048);
        Decoder reader(&StorageProtocol::buffer[0]);
        if (len >= 1 && StorageProtocol::buffer[0] == 0xfe) {
            reader.read<uint8_t>();
            while (reader.at + 3 <= StorageProtocol::buffer + len) {
                MobID::T mob_id = reader.read<uint8_t>();
                uint16_t bits = reader.read<uint16_t>();
                if (mob_id >= MobID::kNumMobs) break;
                Game::seen_mobs[mob_id] = bits & ((uint16_t)((1u << RarityID::kNumRarities) - 1));
            }
        } else {
            while (reader.at < StorageProtocol::buffer + len) {
                MobID::T mob_id = reader.read<uint8_t>();
                if (mob_id >= MobID::kNumMobs) break;
                // Legacy: unlock only the baked rarity so higher cells stay to find.
                uint8_t r = MOB_DATA[mob_id].rarity;
                if (r < RarityID::kNumRarities)
                    Game::seen_mobs[mob_id] |= (uint16_t)(1u << r);
            }
        }
    }
    {
        // v2 petal gallery: leading 0xff then u16 ids. Legacy: bare u8 ids (<0xff).
        uint32_t len = StorageProtocol::retrieve("petals", 2048);
        Decoder reader(&StorageProtocol::buffer[0]);
        if (len >= 1 && StorageProtocol::buffer[0] == 0xff) {
            reader.read<uint8_t>();
            while (reader.at < StorageProtocol::buffer + len) {
                PetalID::T petal_id = reader.read<uint16_t>();
                if (petal_id >= PetalID::kNumPetals || petal_id == PetalID::kNone) break;
                Game::seen_petals[petal_id] = 1;
            }
        } else {
            while (reader.at < StorageProtocol::buffer + len) {
                PetalID::T petal_id = reader.read<uint8_t>();
                if (petal_id >= PetalID::kNumPetals || petal_id == PetalID::kNone) break;
                Game::seen_petals[petal_id] = 1;
            }
        }
    }
    {
        uint32_t len = StorageProtocol::retrieve("settings", 2);
        if (len > 0) {
            Decoder reader(&StorageProtocol::buffer[0]);
            uint8_t opts = reader.read<uint8_t>();
            Input::movement_helper = BitMath::at(opts, 0);
            Input::keyboard_movement = BitMath::at(opts, 1);
            // bit3 marks language as explicitly saved; otherwise keep Chinese default
            if (BitMath::at(opts, 3))
                I18n::use_chinese = BitMath::at(opts, 2);
            else
                I18n::use_chinese = 1;
            Input::invert_attack = BitMath::at(opts, 4);
            Input::invert_defend = BitMath::at(opts, 5);
            // bit7 = request_mobile explicitly saved; else default to device probe
            if (BitMath::at(opts, 7))
                Input::request_mobile = BitMath::at(opts, 6);
            else
                Input::request_mobile = Input::is_mobile;
            if (len >= 2) {
                uint8_t b2 = reader.read<uint8_t>();
                Game::show_debug = BitMath::at(b2, 0);
                Input::disable_auto_party = BitMath::at(b2, 1);
                Game::show_hitboxes = BitMath::at(b2, 2);
                Game::show_low_rarity_drops = BitMath::at(b2, 3);
            }
        } else {
            Input::request_mobile = Input::is_mobile;
        }
    }
    {
        // florr_clone-style remembered FOV multiplier (centi-units, 55..220)
        uint32_t len = StorageProtocol::retrieve("cam_zoom", 4);
        if (len >= 2) {
            Decoder reader(&StorageProtocol::buffer[0]);
            uint16_t z = reader.read<uint16_t>();
            float zoom = z / 100.f;
            if (zoom >= 0.55f && zoom <= 2.2f)
                Game::camera_zoom = zoom;
        }
    }
    {
        uint32_t len = StorageProtocol::retrieve("guest_id", sizeof(uint32_t) * MAX_NAME_LENGTH + 4);
        if (len > 0 && len <= sizeof(uint32_t) * MAX_NAME_LENGTH + 4) {
            Decoder reader(&StorageProtocol::buffer[0]);
            Game::guest_id = reader.read<std::string>();
        }
    }
    {
        uint32_t len = StorageProtocol::retrieve("nickname", sizeof(uint32_t) * MAX_NAME_LENGTH + 4);
        if (len > 0 && len <= sizeof(uint32_t) * MAX_NAME_LENGTH + 4) {
            Decoder reader(&StorageProtocol::buffer[0]);
            Game::nickname = reader.read<std::string>();
        }
    }
    // Migrate old free-form nicknames: only keep if already a valid guest id
    if (Game::guest_id.empty() && Game::nickname.size() == 8
        && Game::nickname[0] == 'G' && Game::nickname[1] == '#')
        Game::guest_id = Game::nickname;
    Game::ensure_guest_id();
    {
        uint32_t len = StorageProtocol::retrieve("username", sizeof(uint32_t) * MAX_USERNAME_LENGTH + 4);
        if (len > 0 && len <= sizeof(uint32_t) * MAX_USERNAME_LENGTH + 4) {
            Decoder reader(&StorageProtocol::buffer[0]);
            Game::username = reader.read<std::string>();
        }
    }
    {
        // Remembered password: used for auto-login after WS verify (see Socket.cc).
        uint32_t len = StorageProtocol::retrieve("password", sizeof(uint32_t) * MAX_PASSWORD_LENGTH + 4);
        if (len > 0 && len <= sizeof(uint32_t) * MAX_PASSWORD_LENGTH + 4) {
            Decoder reader(&StorageProtocol::buffer[0]);
            Game::password = reader.read<std::string>();
        }
    }
    bool guest_talents_loaded = false;
    {
        uint32_t len = StorageProtocol::retrieve("guest", StorageProtocol::MAX_LENGTH);
        // v3: ver + valid + level(u32) + petals(u16). v2: petals(u8). Old: no ver.
        if (len >= 2 + 2 * MAX_SLOT_COUNT) {
            Decoder reader(&StorageProtocol::buffer[0]);
            uint8_t first = reader.read<uint8_t>();
            bool guest_ok = true;
            uint8_t petal_u16 = 0;
            if (first == 5 || first == 4 || first == 3 || first == 2) {
                uint32_t need = (first >= 4)
                    ? (2 + 4 + 4 + 2 * MAX_SLOT_COUNT) // ver+valid+level+score+petals u16
                    : (2 + 4 + 2 * MAX_SLOT_COUNT);
                if (len < need) {
                    guest_ok = false;
                } else {
                    Game::guest_save_valid = reader.read<uint8_t>() ? 1 : 0;
                    Game::guest_level = reader.read<uint32_t>();
                    if (first >= 4)
                        Game::guest_score = reader.read<uint32_t>();
                    else
                        Game::guest_score = 0;
                    petal_u16 = (first >= 3) ? 1 : 0;
                }
            } else {
                Game::guest_save_valid = first ? 1 : 0;
                Game::guest_level = reader.read<uint8_t>();
                Game::guest_score = 0;
            }
            if (guest_ok) {
                if (Game::guest_level < 1) Game::guest_level = 1;
                {
                    uint32_t floor = level_to_score(Game::guest_level);
                    if (Game::guest_score < floor) Game::guest_score = floor;
                }
                for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
                    PetalID::T id = petal_u16
                        ? reader.read<uint16_t>()
                        : (PetalID::T) reader.read<uint8_t>();
                    if (id >= PetalID::kNumPetals) id = PetalID::kNone;
                    Game::guest_petals[i] = id;
                    if (id != PetalID::kNone) Game::seen_petals[id] = 1;
                }
                Game::guest_craft_bank = {};
                uint32_t remain = 0;
                if (reader.at < StorageProtocol::buffer + len)
                    remain = (uint32_t) ((StorageProtocol::buffer + len - reader.at) / 4);
                uint32_t bank_n = PetalID::kNumPetals;
                // Newest→oldest append waves (must match AccountStore::load).
                constexpr uint32_t kWikiImportAdded = 612; // wiki petal import wave
                constexpr uint32_t kTriangleAdded = 9; // Triangle Common..Unique
                constexpr uint32_t kHoneyMeteorLadderAdded = 16;
                constexpr uint32_t kHoneyMeteorAdded = 2;
                constexpr uint32_t kMonsteraLadderAdded = 6;
                constexpr uint32_t kAntHellAdded = 12;
                constexpr uint32_t kSuperUltraAdded = 42;
                constexpr uint32_t kCrossLegendAdded = 40;
                constexpr uint32_t kBaseCraftAdded = 22;
                constexpr uint32_t kMatsAdded = 48;
                constexpr uint32_t kMonsteraAdded = 1;
                constexpr uint32_t kCoinAdded = 1;
                constexpr uint32_t kCommonLadderAdded = 14;
                constexpr uint32_t kStingerLegAdded = 1;
                constexpr uint32_t kJungleAdded = 12;
                constexpr uint32_t kLadderAdded = 43;
                constexpr uint32_t kRoyalAdded = 12;
                uint32_t const n = (uint32_t) PetalID::kNumPetals;
                auto sub = [](uint32_t cur, uint32_t amt) -> uint32_t {
                    return cur > amt ? cur - amt : cur;
                };
                uint32_t const prev_wiki = sub(n, kWikiImportAdded);
                uint32_t const prev_triangle = sub(prev_wiki, kTriangleAdded);
                uint32_t const prev_honey_meteor_ladder = sub(prev_triangle, kHoneyMeteorLadderAdded);
                uint32_t const prev_honey_meteor = sub(prev_honey_meteor_ladder, kHoneyMeteorAdded);
                uint32_t const prev_monstera_ladder = sub(prev_honey_meteor, kMonsteraLadderAdded);
                uint32_t const prev_anthell = sub(prev_monstera_ladder, kAntHellAdded);
                uint32_t const prev_super = sub(prev_anthell, kSuperUltraAdded);
                uint32_t const prev_cross = sub(prev_super, kCrossLegendAdded);
                uint32_t const prev_basecraft = sub(prev_cross, kBaseCraftAdded);
                uint32_t const prev_mats = sub(prev_basecraft, kMatsAdded);
                uint32_t const prev_monstera = sub(prev_mats, kMonsteraAdded);
                uint32_t const prev_coin = sub(prev_monstera, kCoinAdded);
                uint32_t const prev_common = sub(prev_coin, kCommonLadderAdded);
                uint32_t const prev_stinger = sub(prev_common, kStingerLegAdded);
                uint32_t const prev_jungle = sub(prev_stinger, kJungleAdded);
                uint32_t const prev_ladder = sub(prev_jungle, kLadderAdded);
                uint32_t const prev_royal = sub(prev_ladder, kRoyalAdded);
                uint32_t const bank_candidates[] = {
                    n, prev_wiki, prev_triangle, prev_honey_meteor_ladder, prev_honey_meteor, prev_monstera_ladder,
                    prev_anthell, prev_super,
                    prev_cross, prev_basecraft, prev_mats, prev_monstera, prev_coin,
                    prev_common, prev_stinger, prev_jungle, prev_ladder, prev_royal
                };
                // Prefer exact trailing length match (bank [+ coins]) over "largest fits".
                auto guest_bank_ok = [&](uint32_t cand) -> bool {
                    return remain == cand || remain == cand + 1;
                };
                bank_n = 0;
                for (uint32_t cand : bank_candidates) {
                    if (guest_bank_ok(cand)) {
                        bank_n = cand;
                        break;
                    }
                }
                if (bank_n == 0) {
                    for (uint32_t cand : bank_candidates) {
                        if (remain >= cand) {
                            bank_n = cand;
                            break;
                        }
                    }
                    if (bank_n == 0) bank_n = PetalID::kNumPetals;
                }
                if (remain >= bank_n && !(remain == 7 || remain == 8)) {
                    for (uint32_t i = 0; i < bank_n; ++i) {
                        uint32_t v = reader.read<uint32_t>();
                        if (i != (uint32_t) PetalID::kCoin && v > 1000000u) v = 0;
                        else if (i == (uint32_t) PetalID::kCoin && v > 1000000000u) v = 1000000000u;
                        Game::guest_craft_bank[i] = v;
                    }
                    if (reader.at + 4 <= StorageProtocol::buffer + len)
                        Game::guest_coins = reader.read<uint32_t>();
                } else if (remain == 7 || remain == 8) {
                    // Exact legacy rarity buckets only (not truncated modern banks).
                    for (uint8_t r = 0; r < 7; ++r) {
                        uint32_t count = reader.read<uint32_t>();
                        PetalID::T id = Craft::first_petal_of_rarity(r);
                        if (id != PetalID::kNone) Game::guest_craft_bank[id] = count;
                    }
                    if (reader.at + 4 <= StorageProtocol::buffer + len)
                        Game::guest_coins = reader.read<uint32_t>();
                }
                if (reader.at + TalentID::kNum <= StorageProtocol::buffer + len) {
                    for (uint8_t i = 0; i < TalentID::kNum; ++i)
                        Game::guest_talent_ranks[i] = reader.read<uint8_t>();
                    guest_talents_loaded = true;
                } else if (reader.at + 2 <= StorageProtocol::buffer + len) {
                    Game::guest_talent_ranks[0] = reader.read<uint8_t>();
                    Game::guest_talent_ranks[1] = reader.read<uint8_t>();
                    guest_talents_loaded = true;
                }
                // Optional trailing Unique levels (added after talents).
                Game::guest_petal_levels = {};
                if (reader.at + 2 <= StorageProtocol::buffer + len) {
                    uint16_t n_levels = reader.read<uint16_t>();
                    for (uint16_t i = 0; i < n_levels; ++i) {
                        if (reader.at + 2 + 4 > StorageProtocol::buffer + len) break;
                        uint16_t id = reader.read<uint16_t>();
                        uint32_t lv = reader.read<uint32_t>();
                        if (id > 0 && id < PetalID::kNumPetals
                            && lv > 0 && lv <= Craft::kMaxPetalLevel)
                            Game::guest_petal_levels[id] = lv;
                    }
                }
                if (Game::guest_coins > 0) {
                    uint64_t sum = (uint64_t) Game::guest_craft_bank[PetalID::kCoin] + Game::guest_coins;
                    if (sum > 1000000000ull) sum = 1000000000ull;
                    Game::guest_craft_bank[PetalID::kCoin] = (uint32_t) sum;
                    Game::guest_coins = 0;
                }
                for (uint32_t i = 0; i < PetalID::kNumPetals; ++i) {
                    if (i == (uint32_t) PetalID::kCoin) continue;
                    if (Game::guest_craft_bank[i] > 1000000u)
                        Game::guest_craft_bank[i] = 0;
                }
                if (Game::guest_save_valid) {
                    Game::craft_bank = Game::guest_craft_bank;
                    Game::petal_levels = Game::guest_petal_levels;
                    Game::coins = Game::guest_coins;
                }
            }
        }
    }
    {
        uint32_t len = StorageProtocol::retrieve("talents", 16);
        if (len >= 3) {
            Decoder reader(&StorageProtocol::buffer[0]);
            if (len >= TalentID::kNum + 1) {
                uint8_t ranks[TalentID::kNum];
                for (uint8_t i = 0; i < TalentID::kNum; ++i)
                    ranks[i] = reader.read<uint8_t>();
                if (!guest_talents_loaded && Game::session_token == 0) {
                    for (uint8_t i = 0; i < TalentID::kNum; ++i)
                        Game::guest_talent_ranks[i] = ranks[i];
                }
                uint8_t rs = reader.read<uint8_t>();
                if (rs > 10) rs = 7;
                Game::rot_speed_setting = rs;
                Game::rot_speed_f = (float) rs;
            } else {
                uint8_t legacy_hp = reader.read<uint8_t>();
                uint8_t legacy_rot = reader.read<uint8_t>();
                if (!guest_talents_loaded && Game::session_token == 0) {
                    Game::guest_talent_ranks[0] = legacy_hp;
                    Game::guest_talent_ranks[1] = legacy_rot;
                }
                uint8_t rs = reader.read<uint8_t>();
                if (rs > 10) rs = 7;
                Game::rot_speed_setting = rs;
                Game::rot_speed_f = (float) rs;
            }
        }
    }
    Talent::clamp_ranks(Game::guest_talent_ranks, Game::guest_level);
    for (uint8_t i = 0; i < TalentID::kNum; ++i)
        Game::talent_ranks[i] = Game::guest_talent_ranks[i];
    #ifdef DEBUG
    uint16_t const all_r = (uint16_t)((1u << RarityID::kNumRarities) - 1);
    for (MobID::T i = 0; i < MobID::kNumMobs; ++i) Game::seen_mobs[i] = all_r;
    for (PetalID::T i = PetalID::kBasic; i < PetalID::kNumPetals; ++i) Game::seen_petals[i] = 1;
    #endif
}

void Storage::set() {
    #define X(ct, name) checker_##ct.cmp(name);
    STORED
    #undef X
    if (should_update == 0) return;
    should_update = 0;
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<uint8_t>(0xff); // marker: following ids are u16
        for (uint32_t id = PetalID::kBasic; id < (uint32_t) PetalID::kNumPetals; ++id)
            if (Game::seen_petals[id]) writer.write<uint16_t>((uint16_t) id);
        StorageProtocol::store("petals", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<uint8_t>(0xfe); // v2 rarity bitfields
        for (MobID::T id = 0; id < MobID::kNumMobs; ++id) {
            if (Game::seen_mobs[id] == 0) continue;
            writer.write<uint8_t>(id);
            writer.write<uint16_t>(Game::seen_mobs[id]);
        }
        StorageProtocol::store("mobs", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<std::string>(Game::guest_id);
        StorageProtocol::store("guest_id", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<std::string>(Game::nickname);
        StorageProtocol::store("nickname", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<std::string>(Game::username);
        StorageProtocol::store("username", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<std::string>(Game::password);
        StorageProtocol::store("password", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<uint8_t>(
            Input::movement_helper
            | (Input::keyboard_movement << 1)
            | (I18n::use_chinese << 2)
            | (1 << 3)
            | (Input::invert_attack << 4)
            | (Input::invert_defend << 5)
            | ((Input::request_mobile ? 1 : 0) << 6)
            | (1 << 7)
        );
        writer.write<uint8_t>(
            (Game::show_debug ? 1 : 0)
            | ((Input::disable_auto_party ? 1 : 0) << 1)
            | ((Game::show_hitboxes ? 1 : 0) << 2)
            | ((Game::show_low_rarity_drops ? 1 : 0) << 3)
        );
        StorageProtocol::store("settings", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        writer.write<uint8_t>(5); // guest blob + sparse Unique levels trailer
        writer.write<uint8_t>(Game::guest_save_valid);
        writer.write<uint32_t>(Game::guest_level);
        {
            uint32_t sc = Game::guest_score;
            uint32_t floor = level_to_score(Game::guest_level < 1 ? 1 : Game::guest_level);
            if (sc < floor) sc = floor;
            writer.write<uint32_t>(sc);
        }
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            writer.write<uint16_t>(Game::guest_petals[i]);
        for (uint32_t i = 0; i < PetalID::kNumPetals; ++i)
            writer.write<uint32_t>(Game::guest_craft_bank[i]);
        writer.write<uint32_t>(Game::guest_coins);
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            writer.write<uint8_t>(Game::guest_talent_ranks[i]);
        {
            uint16_t n_levels = 0;
            for (uint32_t i = 1; i < PetalID::kNumPetals; ++i)
                if (Game::guest_petal_levels[i] > 0) ++n_levels;
            writer.write<uint16_t>(n_levels);
            for (uint32_t i = 1; i < PetalID::kNumPetals; ++i) {
                if (Game::guest_petal_levels[i] == 0) continue;
                writer.write<uint16_t>((uint16_t) i);
                writer.write<uint32_t>(Game::guest_petal_levels[i]);
            }
        }
        StorageProtocol::store("guest", writer.at - writer.base);
    }
    {
        Encoder writer(&StorageProtocol::buffer[0]);
        for (uint8_t i = 0; i < TalentID::kNum; ++i)
            writer.write<uint8_t>(Game::guest_talent_ranks[i]);
        writer.write<uint8_t>(Game::rot_speed_setting);
        StorageProtocol::store("talents", writer.at - writer.base);
    }
}

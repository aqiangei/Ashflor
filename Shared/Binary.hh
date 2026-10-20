#pragma once

#include <Shared/EntityDef.hh>

#include <cstdint>
#include <string>
#include <vector>

enum Clientbound {
    kClientUpdate,
    kAccountResult,
    kChatMessage,
    kPartyUpdate, // count, [name, flags, hp%, x, y]*, revive%
    kCraftUpdate,
    kSpawnResult,
    kTalentUpdate,
    kInvitePrompt, // type, from_name — pending party/guild invite toast
    kGuildUpdate, // name, announcement, count, [name, flags, level]*  flags:1 leader 2 online 4 self
    kLoadoutPresetSync,
    kBoostUpdate,
    kLeaderboardStats, // multi-category stats board (name/value rows)
    kLifeSummary // this-life mob kills + petal pickups (shown on death)
};

enum InviteType : uint8_t {
    kInviteParty = 0,
    kInviteGuild = 1
};

enum SpawnResultCode : uint8_t {
    kSpawnOk = 0,
    kSpawnNameTaken = 1
};

enum Serverbound {
    kVerify,
    kClientInput,
    kClientSpawn,
    kPetalSwap,
    kPetalDelete,
    kAccountAuth,
    kChat,
    kPartyInvite,
    kPartyLeave,
    kPetalCraft,
    kGuestProgress,
    kAccountLogout,
    kTalentSpend,
    kHeartbeat, // app-level keepalive (background tabs freeze rAF)
    kInviteRespond, // type, accept(0/1), from_name
    kGuildCreate,   // guild name
    kGuildInvite,   // target player name
    kGuildLeave,
    kClientLeave, // voluntary return to title — keep WS, despawn flower only
    kLoadoutPresetSave,
    kLoadoutPresetApply,
    kBankWithdraw, // equip one copy from backpack stacks
    kLeaderboardRequest, // ask the server for the stats board
    kBankDeposit, // unequip primary/secondary → backpack stacks
    kBoostToggle, // arg = 0/1 — pause or resume triple-drop bonus timers
    kGuildSetAnno  // announcement text (leader only)
};

// Categories shown as separate columns on the leaderboard board.
// Order is part of the wire format: keep in sync with the client UI.
enum LeaderboardCategory : uint8_t {
    kLbPlayTime = 0, // seconds online
    kLbLevel,        // account level
    kLbMessages,     // chat messages sent
    kLbKills,        // players killed
    kLbCrafts,       // craft attempts
    kLbScore,        // best score ever reached
    kLbCategoryCount
};

enum CraftMode : uint8_t {
    kCraftGamble = 0,     // arg = PetalID → consume 5, chance at higher
    kCraftExchange = 1,   // arg = PetalID → EV-cost consume, guaranteed
    kCraftBuyRarity = 2,  // arg = rarity → random petal of that rarity
    kCraftBuySpecial = 3, // arg = PetalID special (legacy coin shop)
    kCraftGambleAll = 4,  // arg = PetalID → craft every possible batch
    kCraftRecipe = 5      // arg = recipe index → consume ingredients, give result
};

enum CraftResultCode : uint8_t {
    kCraftBankSync = 0,
    kCraftNoMats = 1,
    kCraftMiss = 2,
    kCraftWin = 3,
    kCraftExchanged = 4,
    kCraftNoCoins = 5,
    kCraftBought = 6,
    kCraftInvFull = 7
};

enum LoadoutPresetResult : uint8_t {
    kPresetSynced = 0,
    kPresetSaved,
    kPresetApplied,
    kPresetEmpty,
    kPresetAppliedAdjusted,
    kPresetNeedAccount,
    kPresetAppliedBanked
};

enum AccountAuthMode {
    kAccountLogin = 0,
    kAccountRegister = 1,
    kAccountResume = 2
};

enum AccountResultCode {
    kAccountOk = 0,
    kAccountInvalidUser = 1,
    kAccountInvalidPass = 2,
    kAccountExists = 3,
    kAccountNotFound = 4,
    kAccountWrongPass = 5,
    kAccountAlreadyIn = 6,
    kAccountLoggedOut = 7,
    kAccountRateLimited = 8
};

enum CloseReason {
    kServer = 4001,
    kProtocol = 4002,
    kOutdated = 4003
};

enum ChatChannel : uint8_t {
    kChatNearby = 0,
    kChatParty = 1,
    kChatGuild = 2,
    kChatWhisper = 3,
    kChatSystem = 4
};

class Writer {
public:
    uint8_t *at;
    uint8_t *packet;
    template<typename T>
    class Encoder {
        friend class Writer;
        static void write(Writer &, T const &);
    };

    template<typename T>
    class Encoder<std::vector<T>> {
        friend class Writer;
        static void write(Writer &w, std::vector<T> const &v) {
            w.write<uint32_t>(v.size());
            for (T const &x : v) w.write<T>(x);
        };
    };

    Writer(uint8_t *);
    template<typename T>
    void write(T const &v) {
        Encoder<T>::write(*this, v);
    };
    void push(uint8_t);
};

class Reader {
public:
    template<typename T>
    class Decoder {
        friend class Reader;
        static T read(Reader &);
        static void read(Reader &, T &);
    };

    template<typename T>
    class Decoder<std::vector<T>> {
        friend class Reader;
        static std::vector<T> read(Reader &r) {
            uint32_t len = r.read<uint32_t>();
            std::vector<T> ret(len);
            for (uint32_t i = 0; i < len; ++i)
                ret.push_back(r.read<T>());
            return ret;
        }

        static void read(Reader &r, std::vector<T> &v) {
            uint32_t len = r.read<uint32_t>();
            v.clear();
            v.reserve(len);
            for (uint32_t i = 0; i < len; ++i)
                v.push_back(r.read<T>());
        }
    };

    uint8_t const *packet;
    uint8_t const *at;
    Reader(uint8_t const *);

    template<typename T>
    T read() {
        return Decoder<T>::read(*this);
    }

    template<typename T>
    void read(T &v) {
        Decoder<T>::read(*this, v);
    }

    uint8_t next();
};

class Validator {
public:
    uint8_t const *at;
    uint8_t const *end;
    Validator(uint8_t const *, uint8_t const *);

    uint8_t validate_uint8();
    uint8_t validate_uint16();
    uint8_t validate_uint32();
    uint8_t validate_uint64();
    uint8_t validate_float();
    uint8_t validate_string(uint32_t);
};

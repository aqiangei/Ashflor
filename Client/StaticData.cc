#include <Client/StaticData.hh>

// RARITY_COLORS lives in Shared/RarityScale.hh (MapGen + minimap share it).

std::array<uint32_t, ColorID::kNumColors> const FLOWER_COLORS = {
    0xffffe763, 0xff999999, 0xff689ce2, 0xffec6869
};

std::array<char const *, RarityID::kNumRarities> const RARITY_NAMES = {
    "Common",
    "Unusual",
    "Rare",
    "Epic",
    "Legendary",
    "Mythic",
    "Unique",
    "Super",
    "Ultra"
};

std::array<char const, MAX_SLOT_COUNT> const SLOT_KEYBINDS =
    {'1','2','3','4','5','6','7','8','9','0'};

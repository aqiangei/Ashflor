#include <Client/StaticData.hh>

std::array<uint32_t, RarityID::kNumRarities> const RARITY_COLORS = {
    0xff7eef6d, // Common
    0xffffe65d, // Unusual
    0xff4d52e3, // Rare
    0xff861fde, // Epic
    0xffde1f1f, // Legendary
    0xff1fdbde, // Mythic
    0xffde1f65, // Unique
    0xffff2b75, // Super
    0xfff70fb6  // Ultra
};

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
    {'1','2','3','4','5','6','7','8'};

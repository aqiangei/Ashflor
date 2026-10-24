#include <Client/I18n.hh>

#include <Shared/StaticData.hh>

// Minimal I18n for gardn-art: Petal.cc references petal_name when draw_name≠0.
// Codex always draws with draw_name=0; stubs keep the linker happy without Game.cc.
namespace I18n {
    uint8_t use_chinese = 1;

    char const *petal_name(PetalID::T id) {
        if (id >= PetalID::kNumPetals) return "";
        return PETAL_DATA[id].name ? PETAL_DATA[id].name : "";
    }

    char const *petal_desc(PetalID::T id) {
        if (id >= PetalID::kNumPetals) return "";
        return PETAL_DATA[id].description ? PETAL_DATA[id].description : "";
    }

    char const *mob_name(MobID::T id) {
        if (id >= MobID::kNumMobs) return "";
        return MOB_DATA[id].name ? MOB_DATA[id].name : "";
    }

    char const *mob_desc(MobID::T id) {
        if (id >= MobID::kNumMobs) return "";
        return MOB_DATA[id].description ? MOB_DATA[id].description : "";
    }
}

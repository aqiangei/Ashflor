#pragma once

#include <Client/Ui/Element.hh>

#include <Shared/StaticData.hh>

namespace Ui {
    class DeadFlowerIcon final : public Element {
    public:
        DeadFlowerIcon(float);

        virtual void on_render(Renderer &) override;
    };

    class GalleryPetal final : public Element {
    public:
        PetalID::T id;
        // 1 = always paint icon (mob-drop lists); 0 = locked plate until seen
        uint8_t always_show;
        GalleryPetal(PetalID::T, float, uint8_t always_show = 0);

        virtual void on_render(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };

    class PetalsCollectedIndicator final : public Element {
    public:
        PetalsCollectedIndicator(float);

        virtual void on_render(Renderer &) override;
    };

    class GalleryMob final : public Element {
    public:
        MobID::T id;
        uint8_t display_rarity = 0; // live rarity (may exceed baked MOB_DATA rarity)
        uint32_t tip_touch = (uint32_t) -1;
        GalleryMob(MobID::T, float);
        GalleryMob(MobID::T, uint8_t rarity, float);

        virtual void on_render(Renderer &) override;
        virtual void on_event(uint8_t) override;
    };

    // [mob][rarity] — virtual ladder cells share one MobID across rarities
    extern Element *mob_tooltips[MobID::kNumMobs][RarityID::kNumRarities];
    void make_mob_tooltips();

    Element *make_title_input_box();
    Element *make_title_info_box();
    Element *make_panel_buttons();
    Element *make_death_main_screen();
    Element *make_death_life_summary();
    Element *make_stat_screen();
    Element *make_settings_panel();
    Element *make_petal_gallery();
    Element *make_mob_gallery();
    Element *make_changelog();
    Element *make_debug_stats();
    Element *make_github_link_button();
}
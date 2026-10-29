#!/usr/bin/env python3
"""Fix mojibake descriptions + missing drops after wiki import."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = ROOT / "Shared" / "StaticData.cc"

# English descriptions for wiki petals (PETAL_DATA uses EN; ZH lives in i18n)
EN_DESC = {
    "Lentil": "Increases petal attraction to enemies.",
    "Sawblade": "This looks like it'd hurt.",
    "Dice": "Has a small chance of dealing 35x damage. Luck increases the chance.",
    "Domino": "Damage depends on how many dots it has.",
    "Card": "A playing card. Likely marked.",
    "Poker Chip": "ALL IN.",
    "Bean": "Deals more damage if there are no allied flowers around you.",
    "Clay": "It sticks to dust in the air to get bigger and stronger.",
    "Tomato": "Gets stronger over time.",
    "Dust": "Sometimes it sticks together, sometimes it doesn't.",
    "Broccoli": "Explodes when it hits something.",
    "Orange": "A sweet citrus fruit.",
    "Plank": "Does 20x damage vs projectiles and petals.",
    "Bur": "Decreases armor of affected unit. Does not stack.",
    "Soil": "Increases health, but also size and decreases move speed.",
    "Air": "It's literally nothing.",
    "Basil": "Increases all healing received.",
    "Cotton": "Absorbs damage taken by the flower.",
    "Rubber": "Increases knockback received. Absorbs lightning damage.",
    "Carrot": "Sturdy and reliable. Can be shot and bounces off walls.",
    "Grapes": "It goes poof. Now with poison.",
    "Dead Leaf": "Increases petal health but makes them reload slower.",
    "Clover": "Increases your luck.",
    "Totem": "Grants health regen for a duration after killing a mob.",
    "Root": "Provides layers of additional armor over time.",
    "Lightning": "Strikes several nearby enemies.",
    "Battery": "Discharges electricity whenever the flower gets hit.",
    "Laser": "Pew pew.",
    "Electric Web": "A little less sticky, but a bit more shocking.",
    "Uranium": "Periodically releases radiation.",
    "Sponge": "Spreads damage taken over time.",
    "Fang": "Heals based on damage dealt by this petal.",
    "Shell": "Adds a temporary shield to yourself or allies.",
    "Claw": "Deals extra damage if the victim is above 80% health.",
    "Glass": "Phases through enemies. Limited hit interval.",
    "Light Bulb": "Draws aggro from mobs.",
    "Poo": "Makes mobs less likely to attack you.",
    "Ankh": "When destroyed, teleports the flower back.",
    "Shovel": "Become untargetable but unable to use petals briefly.",
    "Bandage": "Delays death for a limited time.",
    "Wax": "Don't ask where it came from.",
    "Amulet": "Converts a percentage of overheal into shields.",
    "Privet Berry": "Deals extra poison based on existing poison DPS.",
    "Blueberries": "It goes poof. Extra ingredient: lightning.",
    "Fragment": "Perhaps this could be used to forge something great?",
    "Cogwheel": "Doesn't seem to do much...",
    "Mana Orb": "Creates mana.",
    "Magic Leaf": "Gathers mana from the environment passively.",
    "Magic Cactus": "Increases maximum mana capacity.",
    "Magic Cotton": "Absorbs damage using mana.",
    "Magic Eye": "Consumes half current mana to deal lightning damage.",
    "Magic Stinger": "It really hurts, but it's very fragile.",
    "Magic Missile": "You can shoot this one... with magic.",
    "Magic Stick": "Summons the forces of the wind... with magic.",
    "Magic Bubble": "Physics are for the magicless.",
    "Talisman of Evasion": "Allows the wearer to anticipate enemy attacks.",
    "Nazar Amulet": "Reduces damage by 98% and consumes a charge.",
    "Compass": "This might find something interesting...",
    "Mimic": "Copies the petal to the left at the same rarity.",
    "Mysterious Powder": "Increases movement speed.",
    "Mysterious Relic": "Significantly increases maximum health.",
    "Dark Mark": "A dark mark that binds itself to a fallen soul.",
    "Corruption": "Corrupts one's soul.",
    "Blood Sacrifice": "On death, summons a mob to fight again.",
    "Pharaoh's Crown": "Spawns Undead Beetles to fight at your side.",
    "Champion's Crown": "Gives flowers around you a Super Basic.",
    "Soul Splitter": "Fractures your soul into two.",
    "Paranormal Goggles": "Allows finding some paranormal activity.",
}

DROPS = {
    "Beetle": ["PetalID::kLentil"],
    "Massive Beetle": ["PetalID::kLentil"],
    "Wandering Guard": ["PetalID::kSawblade", "PetalID::kFragment", "PetalID::kCogwheel"],
    "Rust Mite": ["PetalID::kSawblade", "PetalID::kFragment"],
    "Mimic Chest": ["PetalID::kSawblade", "PetalID::kMimic", "PetalID::kRelic"],
    "Square": ["PetalID::kDice", "PetalID::kDomino", "PetalID::kCard", "PetalID::kPokerChip"],
    "Ladybug": ["PetalID::kBean", "PetalID::kTomato", "PetalID::kBasil", "PetalID::kClover", "PetalID::kAmulet"],
    "Rock": ["PetalID::kClay", "PetalID::kSoil", "PetalID::kGlass"],
    "Boulder": ["PetalID::kClay"],
    "Sandstorm": ["PetalID::kDust", "PetalID::kCompass", "PetalID::kPowder", "PetalID::kMagicStick"],
    "Cactus": ["PetalID::kBroccoli", "PetalID::kRoot", "PetalID::kMagicCactus"],
    "Bee": ["PetalID::kOrange", "PetalID::kBandage", "PetalID::kWax", "PetalID::kBlueberries", "PetalID::kMagicStinger"],
    "Tunnel Guard": ["PetalID::kPlank"],
    "Spider": ["PetalID::kBur", "PetalID::kElectricWeb", "PetalID::kFang", "PetalID::kTalisman"],
    "Moon Jelly": ["PetalID::kAir", "PetalID::kRubber", "PetalID::kSponge", "PetalID::kMagicBubble"],
    "Baby Ant": ["PetalID::kCotton", "PetalID::kPoo", "PetalID::kMagicCotton"],
    "Hornet": ["PetalID::kCarrot", "PetalID::kLightning", "PetalID::kWax", "PetalID::kMagicMissile"],
    "Centipede": ["PetalID::kGrapes"],
    "Evil Centipede": ["PetalID::kGrapes", "PetalID::kCorruption"],
    "Leaf Beetle": ["PetalID::kDeadLeaf", "PetalID::kMagicLeaf"],
    "Massive Ladybug": ["PetalID::kTotem"],
    "Cinder Moth": ["PetalID::kLightning", "PetalID::kLaser", "PetalID::kLightBulb", "PetalID::kManaOrb", "PetalID::kGoggles"],
    "Scorpion": ["PetalID::kBattery", "PetalID::kUranium", "PetalID::kFang", "PetalID::kPrivetBerry", "PetalID::kNazar"],
    "Coral Star": ["PetalID::kShell"],
    "Reef Crab": ["PetalID::kClaw"],
    "River Crab": ["PetalID::kClaw"],
    "Dune Beetle": ["PetalID::kAnkh", "PetalID::kShovel"],
    "Phantom Orb": ["PetalID::kMagicEye", "PetalID::kSoulSplitter"],
    "Queen Ant": ["PetalID::kBloodSacrifice"],
}


def fix_em_dash_corruption(text: str) -> str:
    # common mojibake for em-dash / multiply sign from bad round-trip
    return (
        text.replace("â\x80\x94", "—")
        .replace("â", "—")
        .replace("Ã\x97", "×")
        .replace("Ã", "×")
    )


def replace_descriptions(text: str) -> str:
    def repl(m: re.Match) -> str:
        name = m.group(1)
        desc = EN_DESC.get(name)
        if not desc:
            return m.group(0)
        return f'.name = "{name}",\n        .description = "{desc}",'

    # Only within wiki import wave through MOB_DATA
    start = text.find("// ---- Wiki import wave ----")
    end = text.find("std::array<struct MobData", start)
    if start < 0 or end < 0:
        print("wiki wave not found")
        return text
    chunk = text[start:end]
    chunk2 = re.sub(
        r'\.name = "([^"]+)",\n\s*\.description = "[^"]*",',
        repl,
        chunk,
    )
    return text[:start] + chunk2 + text[end:]


def patch_drops(text: str) -> str:
    for mob, pids in DROPS.items():
        pat = re.compile(
            rf'(\.name = "{re.escape(mob)}",.*?\.drops = \{{)([^}}]*)(\}})',
            re.S,
        )

        def repl(m: re.Match, pids=pids) -> str:
            inner = m.group(2)
            add = []
            for pid in pids:
                if pid not in inner:
                    add.append(pid)
            if not add:
                return m.group(0)
            prefix = ", ".join(add)
            stripped = inner.strip()
            if stripped:
                new_inner = "\n            " + prefix + ", " + stripped
            else:
                new_inner = " " + prefix + " "
            return m.group(1) + new_inner + m.group(3)

        text2, n = pat.subn(repl, text, count=1)
        if n == 0:
            print(f"drop miss: {mob}")
        else:
            text = text2
            print(f"drop ok: {mob} +{len(pids)}")
    return text


def fix_attr_order(text: str) -> str:
    """Fix Pharaoh/Magic Stick designated-init field order."""
    # .spawns before .secondary_reload is illegal; rewrite known bad patterns
    text = re.sub(
        r"\.attributes = \{ \.spawns = (MobID::\w+), \.spawn_count = (\d+), \.secondary_reload = ([0-9.]+) \}",
        r".attributes = { .secondary_reload = \3, .spawns = \1, .spawn_count = \2 }",
        text,
    )
    text = re.sub(
        r"\.attributes = \{ \.spawns = (MobID::\w+), \.spawn_count = (\d+), \.mana_spawn_cost = ([0-9.]+) \}",
        r".attributes = { .mana_spawn_cost = \3, .spawns = \1, .spawn_count = \2 }",
        text,
    )
    # shell: burst_heal before secondary_reload is OK (burst_heal comes first)
    # ankh: secondary_reload then ankh_mark — ankh_mark is after stack_damage area, secondary early — BAD
    text = re.sub(
        r"\.attributes = \{ \.secondary_reload = ([0-9.]+), \.ankh_mark = 1 \}",
        r".attributes = { .ankh_mark = 1 }",  # keep mark only; reload stays on petal
        text,
    )
    # electric web / shell may have secondary_reload with later fields — check
    # magic stick with mana_spawn_cost after spawns already handled
    # shovel: shovel_burrow and defend_only — defend_only is near end, shovel_burrow mid — BAD if shovel first
    text = re.sub(
        r"\.attributes = \{ \.shovel_burrow = 1, \.defend_only = 1 \}",
        r".attributes = { .shovel_burrow = 1, .defend_only = 1 }",  # shovel before defend_only in struct? 
        # shovel_burrow is around 2869, defend_only at 2908 — shovel first is OK
        text,
    )
    # blueberries/grapes: split_projectile is near end; lightning/privet before it — if split first BAD
    text = re.sub(
        r"\.attributes = \{ \.split_projectile = 1, \.privet_extra_poison = ([0-9.]+) \}",
        r".attributes = { .privet_extra_poison = \1, .split_projectile = 1 }",
        text,
    )
    text = re.sub(
        r"\.attributes = \{ \.split_projectile = 1, \.lightning_damage = ([0-9.]+), \.lightning_chains = (\d+) \}",
        r".attributes = { .lightning_damage = \1, .lightning_chains = \2, .split_projectile = 1 }",
        text,
    )
    text = re.sub(
        r"\.attributes = \{ \.split_projectile = 1 \}",
        r".attributes = { .split_projectile = 1 }",
        text,
    )
    # electric web: secondary_reload early, lightning mid, aura mid — order secondary, aura_*, lightning? 
    # aura_radius is before stack_damage; lightning_damage is after — so aura before lightning OK
    # secondary_reload, lightning_damage, aura_* — BAD because aura before lightning in struct but secondary first OK, then aura, then lightning
    text = re.sub(
        r"\.attributes = \{ \.secondary_reload = ([0-9.]+), \.lightning_damage = ([0-9.]+), \.aura_radius = ([0-9.]+), \.aura_duration = ([0-9.]+) \}",
        r".attributes = { .secondary_reload = \1, .aura_radius = \3, .aura_duration = \4, .lightning_damage = \2 }",
        text,
    )
    # uranium: poison before aura? poison_damage before aura in struct... actually aura before poison in struct!
    # aura_heal/radius/duration then poison_damage
    text = re.sub(
        r"\.attributes = \{ \.poison_damage = \{[^}]+\}, \.aura_radius = ([0-9.]+), \.aura_duration = ([0-9.]+) \}",
        r".attributes = { .aura_radius = \1, .aura_duration = \2 }",
        text,
    )
    # shell: burst_heal, secondary_reload — burst before secondary OK
    # magic eye: mana_lightning then lightning_chains — both after stack, mana_lightning before lightning_chains? 
    # mana_lightning_per at 2880, lightning_chains at 2861 — lightning_chains BEFORE mana_lightning — BAD
    text = re.sub(
        r"\.attributes = \{ \.mana_lightning_per = ([0-9.]+), \.lightning_chains = (\d+) \}",
        r".attributes = { .lightning_chains = \2, .mana_lightning_per = \1 }",
        text,
    )
    # carrot split only — OK
    # dead leaf: petal_hp_factor, extra_reload_factor — extra_reload before petal_hp in struct — BAD
    text = re.sub(
        r"\.attributes = \{ \.petal_hp_factor = ([0-9.]+), \.extra_reload_factor = ([0-9.]+) \}",
        r".attributes = { .extra_reload_factor = \2, .petal_hp_factor = \1 }",
        text,
    )
    # soil: extra_health, body_radius, move_speed — extra_health before body; move_speed before body — 
    # move_speed at 2828, extra_health 2824, body_radius 2846
    # order should be: extra_health, move_speed_factor, body_radius_bonus
    text = re.sub(
        r"\.attributes = \{ \.extra_health = ([0-9.]+), \.body_radius_bonus = ([0-9.]+), \.move_speed_factor = ([0-9.]+) \}",
        r".attributes = { .extra_health = \1, .move_speed_factor = \3, .body_radius_bonus = \2 }",
        text,
    )
    # powder: move_speed, powder_max — move before powder OK
    # broccoli: splash_damage, splash_radius — both after wiki block, splash after hit_cycle area — OK relative
    # claw: execute_threshold, execute_factor — OK
    # wax: aura_radius, wax_block — aura before wax OK
    return text


def main() -> None:
    raw = CC.read_bytes()
    # detect encoding
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        text = raw.decode("utf-8", errors="replace")
    text = fix_em_dash_corruption(text)
    text = replace_descriptions(text)
    text = fix_attr_order(text)
    text = patch_drops(text)
    CC.write_text(text, encoding="utf-8", newline="\n")
    print("fixed", CC)


if __name__ == "__main__":
    main()

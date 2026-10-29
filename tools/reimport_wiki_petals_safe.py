#!/usr/bin/env python3
"""Restore StaticData.cc from backup, then inject wiki PETAL_DATA + MOB drops safely."""
from __future__ import annotations

import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CC = ROOT / "Shared" / "StaticData.cc"
BACKUP = Path(r"d:/ashflor/temp/release-20261024-1315/source/Shared/StaticData.cc")

# Import petal definitions from the big import module
import sys
sys.path.insert(0, str(ROOT / "tools"))
from import_wiki_petals import PETALS, petal_entries, RARITIES  # noqa: E402

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

# ATTR_ORDER matching PetalAttributes field declaration order
ATTR_ORDER = [
    "clump_radius", "secondary_reload", "constant_heal", "burst_heal", "mass",
    "armor", "poison_armor", "dandelion_inflict_seconds", "slow_inflict_seconds",
    "vision_factor", "extra_body_damage", "extra_rotation_speed", "extra_range",
    "extra_health", "damage_reflection", "extra_damage_factor", "extra_reload_factor",
    "move_speed_factor", "magnet_radius", "attraction_radius", "fixed_orbit_range",
    "aura_heal", "aura_radius", "aura_duration", "poison_damage", "burn_damage",
    "armor_penetration", "bonus_vs_poisoned", "stack_damage",
    "crit_chance", "crit_factor", "armor_shred", "armor_shred_seconds",
    "body_radius_bonus", "heal_amp", "damage_absorb", "knockback_taken",
    "solo_damage_factor", "petal_evasion", "flower_evasion", "nazar_charges",
    "nazar_mitigation", "petal_hp_factor", "luck", "clay_max_damage", "tomato_max_damage",
    "vs_projectile_factor", "lightning_damage", "lightning_chains", "battery_charges",
    "damage_delay_seconds", "on_hit_heal_ratio", "hit_interval_seconds", "phase_through",
    "aggro_shrink", "ankh_mark", "shovel_burrow", "death_delay_seconds",
    "totem_heal", "totem_seconds", "root_armor", "root_interval", "root_max_stacks",
    "wax_block", "mana_gen", "mana_cap", "mana_absorb_per", "mana_lightning_per",
    "mana_spawn_cost", "domino_max_damage", "card_random_stats", "mimic_left",
    "compass", "blood_sacrifice", "champion_crown", "soul_splitter", "paranormal",
    "corruption", "dark_mark", "overheal_to_shield", "privet_extra_poison",
    "powder_max_rarity", "bonus_vs_slowed", "hit_cycle", "hit_cycle_factor",
    "snare_seconds", "splash_damage", "splash_radius", "execute_threshold",
    "execute_factor", "taunt_radius", "on_hit_heal", "radial_pulse_damage",
    "radial_pulse_radius", "aura_slow_seconds", "defend_only", "icon_angle",
    "split_projectile", "rotation_style", "spawns", "spawn_count", "equipment",
]


def format_attrs(attrs: dict) -> str:
    if not attrs:
        return ""
    items = sorted(attrs.items(), key=lambda kv: ATTR_ORDER.index(kv[0]) if kv[0] in ATTR_ORDER else 999)
    parts = ", ".join(f".{k} = {v}" for k, v in items)
    return f",\n        .attributes = {{ {parts} }}"


def make_entries(name, desc, hp0, dmg0, reload, radius, count, attr_fn, unique_eq_super):
    from import_wiki_petals import scale3, fmt
    chunks = []
    for i, (_, rid) in enumerate(RARITIES):
        tier = 7 if (unique_eq_super and i == 8) else i
        hp = scale3(hp0, tier)
        dmg = scale3(dmg0, tier)
        attrs = attr_fn(i, tier) if attr_fn else {}
        attr_s = format_attrs(attrs)
        chunks.append(
            f"""    {{
        .name = "{name}",
        .description = "{desc}",
        .health = {fmt(hp)},
        .damage = {fmt(dmg)},
        .radius = {fmt(radius)},
        .reload = {fmt(reload)},
        .count = {count},
        .rarity = RarityID::{rid}{attr_s}
    }}"""
        )
    return ",\n".join(chunks)


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


def inject_petals(text: str) -> str:
    if '.name = "Lentil"' in text:
        print("Lentil already present")
        return text
    parts = []
    for p in PETALS:
        base, en, zh, desc_zh, hp, dmg, reload, radius, count, attr_fn, ues, drops, phase, tags = p
        if en is None:
            continue
        en_desc = EN_DESC.get(en, desc_zh)
        parts.append(make_entries(en, en_desc, hp, dmg, reload, radius, count, attr_fn, ues))
    blob = ",\n    // ---- Wiki import wave ----\n" + ",\n".join(parts)
    needle = "        .attributes = { .stack_damage = 26244.0 }\n    }\n}};"
    if needle not in text:
        raise SystemExit("Triangle Unique anchor missing in backup")
    return text.replace(needle, "        .attributes = { .stack_damage = 26244.0 }\n    }" + blob + "\n}};", 1)


def patch_mob_drops_only(text: str) -> str:
    mob_start = text.find("std::array<struct MobData")
    if mob_start < 0:
        raise SystemExit("MOB_DATA missing")
    head, body = text[:mob_start], text[mob_start:]
    for mob, pids in DROPS.items():
        pat = re.compile(
            rf'(\.name = "{re.escape(mob)}",\n(?:.*?\n)*?\s*\.drops = \{{)([^}}]*)(\}})',
            re.M,
        )

        def repl(m: re.Match, pids=pids) -> str:
            inner = m.group(2)
            add = [p for p in pids if p not in inner]
            if not add:
                return m.group(0)
            prefix = ", ".join(add)
            stripped = inner.strip().rstrip(",")
            if stripped:
                new_inner = "\n            " + prefix + ",\n            " + stripped
                if not new_inner.rstrip().endswith(","):
                    pass
            else:
                new_inner = "\n            " + prefix + "\n        "
            return m.group(1) + new_inner + m.group(3)

        body2, n = pat.subn(repl, body, count=1)
        if n == 0:
            print(f"drop miss: {mob}")
        else:
            print(f"drop ok: {mob}")
            body = body2
    return head + body


def main() -> None:
    if not BACKUP.exists():
        raise SystemExit(f"backup missing: {BACKUP}")
    shutil.copy2(BACKUP, CC)
    print("restored from", BACKUP)
    text = CC.read_text(encoding="utf-8")
    text = inject_petals(text)
    text = patch_mob_drops_only(text)
    CC.write_text(text, encoding="utf-8", newline="\n")
    # sanity
    t2 = CC.read_text(encoding="utf-8")
    print("Lentil count", t2.count('.name = "Lentil"'))
    print("Sawblade count", t2.count('.name = "Sawblade"'))
    print("Dice count", t2.count('.name = "Dice"'))
    print("Triangle count", t2.count('.name = "Triangle"'))
    # Baby Ant drops should not contain Dice
    m = re.search(r'\.name = "Baby Ant",.*?\.drops = \{([^}]*)\}', t2, re.S)
    if m:
        print("Baby Ant drops snippet:", m.group(1)[:200].replace("\n", " "))


if __name__ == "__main__":
    main()

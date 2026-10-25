#!/usr/bin/env python3
"""Generate / patch wiki petal import waves into ashflor sources.

Phase 0: Lentil + Sawblade
Phase 2+: bulk combat / utility petals with wiki-ish ×3 ladders.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HH = ROOT / "Shared" / "StaticDefinitions.hh"
CC = ROOT / "Shared" / "StaticData.cc"
I18N = ROOT / "Client" / "I18nData.inc"
NAME_ZH = ROOT / "Shared" / "PetalNameZh.inc"
DESC_ZH = ROOT / "Shared" / "PetalDescZh.inc"
STORAGE = ROOT / "Client" / "Storage.cc"
PETAL_CC = ROOT / "Client" / "Assets" / "Petal.cc"
FLOWER = ROOT / "Server" / "Process" / "Flower.cc"
DAMAGE = ROOT / "Server" / "EntityFunctions" / "Damage.cc"
CODEX = ROOT / "Server" / "Codex.cc"
GAP_MD = ROOT.parents[0] / "todo" / "wiki-petal-gap.md"

RARITIES = [
    ("", "kCommon"),
    ("Unusual", "kUnusual"),
    ("Rare", "kRare"),
    ("Epic", "kEpic"),
    ("Legendary", "kLegendary"),
    ("Mythic", "kMythic"),
    ("Ultra", "kUltra"),
    ("Super", "kSuper"),
    ("Unique", "kUnique"),
]


def scale3(v: float, tier: int) -> float:
    return v * (3.0 ** tier)


def fmt(v: float) -> str:
    if abs(v - round(v)) < 1e-6:
        return f"{int(round(v))}.0"
    return f"{v:.4f}".rstrip("0").rstrip(".")


def enum_block(base: str, comment: str) -> str:
    lines = [f"        // {comment}"]
    for suf, _ in RARITIES:
        if suf == "":
            lines.append(f"        k{base},")
        else:
            lines.append(f"        k{base}{suf},")
    return "\n".join(lines)


def petal_entries(
    name: str,
    desc: str,
    hp0: float,
    dmg0: float,
    reload: float,
    radius: float,
    count: int,
    attr_fn,
    unique_eq_super: bool = False,
) -> str:
    chunks = []
    for i, (_, rid) in enumerate(RARITIES):
        tier = i
        if unique_eq_super and i == 8:
            tier = 7
        hp = scale3(hp0, tier)
        dmg = scale3(dmg0, tier)
        attrs = attr_fn(i, tier)
        attr_s = ""
        if attrs:
            parts = ", ".join(f".{k} = {v}" for k, v in attrs.items())
            attr_s = f",\n        .attributes = {{ {parts} }}"
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


# ---- Petal definitions (wiki-aligned where known) ----

def lentil_attr(i, tier):
    return {"attraction_radius": fmt(8.0 * (i + 1))}


def saw_attr(i, tier):
    return {"fixed_orbit_range": "1"}


def dice_attr(i, tier):
    # 5% base crit ×35; luck wired later adds 4%/point
    return {"crit_chance": "0.05", "crit_factor": "35.0"}


def bur_attr(i, tier):
    return {
        "armor_shred": fmt(scale3(1.5, tier)),
        "armor_shred_seconds": "3.0",
    }


def soil_attr(i, tier):
    return {
        "extra_health": fmt(scale3(50.0, tier)),
        "body_radius_bonus": "10.0",
        "move_speed_factor": "0.92",
    }


def air_attr(i, tier):
    return {"body_radius_bonus": fmt(8.0 * (i + 1))}


def basil_attr(i, tier):
    return {"heal_amp": fmt(0.20 + 0.05 * i)}


def cotton_attr(i, tier):
    # absorb amount = health of petal conceptually; use damage_absorb flat pool
    return {"damage_absorb": fmt(scale3(2.0, tier))}


def rubber_attr(i, tier):
    return {"knockback_taken": fmt(0.3 * (i + 1))}


def bean_attr(i, tier):
    return {"solo_damage_factor": "1.5"}


def poker_attr(i, tier):
    return {"petal_evasion": "0.90"}


def talisman_attr(i, tier):
    return {"flower_evasion": fmt(0.03 * (i + 1))}


def nazar_attr(i, tier):
    return {"nazar_charges": "3", "nazar_mitigation": "0.98"}


def dead_leaf_attr(i, tier):
    hp_bonus = 0.0
    if i >= 5:
        hp_bonus = 0.13 + 0.15 * (i - 5)
    return {
        "petal_hp_factor": fmt(1.0 + hp_bonus),
        "extra_reload_factor": "1.05",
    }


def clover_attr(i, tier):
    return {"luck": fmt(0.1 * (i + 1))}


def dust_attr(i, tier):
    return {}


def clay_attr(i, tier):
    # damage scales with distance traveled — use clay_max_damage via attr
    return {
        "clay_max_damage": fmt(scale3(100.0, tier)),
    }


def tomato_attr(i, tier):
    return {
        "tomato_max_damage": fmt(scale3(70.0, tier)),
    }


def plank_attr(i, tier):
    return {"vs_projectile_factor": "20.0"}


def orange_attr(i, tier):
    return {}  # count=3 in data


def broccoli_attr(i, tier):
    return {
        "splash_damage": fmt(scale3(5.0, tier)),
        "splash_radius": "40.0",
    }


def carrot_attr(i, tier):
    return {"split_projectile": "1"}  # shootable; bounce handled like peas


def grapes_attr(i, tier):
    # poison_damage.damage is DPS; time is duration (see Damage.cc / Health.cc).
    return {
        "split_projectile": "1",
        "poison_damage": "{.damage=%s,.time=1.0}" % fmt(scale3(25.0, tier)),
    }


def lightning_attr(i, tier):
    return {
        "lightning_damage": fmt(scale3(12.0, tier)),
        "lightning_chains": str(2 + i),
    }


def battery_attr(i, tier):
    return {
        "lightning_damage": fmt(scale3(12.0, tier)),
        "lightning_chains": str(2 + i),
        "battery_charges": "3",
    }


def uranium_attr(i, tier):
    return {
        "poison_damage": "{.damage=%s,.time=1.0}" % fmt(scale3(50.0, tier)),
        "aura_radius": "80.0",
        "aura_duration": "1.5",
    }


def sponge_attr(i, tier):
    return {
        "damage_delay_seconds": fmt(3.0 + i * 3.0),
    }


def fang_attr(i, tier):
    ratios = [0.35] * 6 + [0.202, 0.117, 0.067]
    return {"on_hit_heal_ratio": fmt(ratios[i])}


def shell_attr(i, tier):
    return {"burst_heal": fmt(scale3(7.0, tier)), "secondary_reload": "1.5"}


def claw_attr(i, tier):
    return {
        "execute_threshold": "0.80",
        "execute_factor": fmt(1.0 + scale3(100.0, tier) / max(scale3(5.0, tier), 1.0)),
    }


def glass_attr(i, tier):
    return {"hit_interval_seconds": "1.0", "phase_through": "1"}


def lightbulb_attr(i, tier):
    return {"taunt_radius": fmt(300.0 * (i + 1))}


def poo_attr(i, tier):
    # reduce aggro — encode as aggro_shrink factor
    factors = [0.25, 0.438, 0.578, 0.684, 0.763, 0.822, 0.867, 0.90, 0.925]
    return {"aggro_shrink": fmt(factors[i])}


def ankh_attr(i, tier):
    cds = [256, 128, 64, 32, 16, 8, 4, 2, 1]
    return {"secondary_reload": "0.5", "ankh_mark": "1"}


def shovel_attr(i, tier):
    return {"shovel_burrow": "1", "defend_only": "1"}


def bandage_attr(i, tier):
    delays = [1.5, 2.1, 2.9, 4.1, 5.8, 8.1, 11.3, 15.8, 22.1]
    return {"death_delay_seconds": fmt(delays[i])}


def totem_attr(i, tier):
    return {"totem_heal": fmt(scale3(1.0, tier)), "totem_seconds": "10.0"}


def root_attr(i, tier):
    return {
        "root_armor": fmt(scale3(2.5, tier)),
        "root_interval": "2.0",
        "root_max_stacks": "10",
    }


def wax_attr(i, tier):
    return {"aura_radius": fmt(30.0 * (i + 1)), "wax_block": "1"}


def cog_attr(i, tier):
    return {}


def fragment_attr(i, tier):
    return {}


def mana_orb_attr(i, tier):
    return {
        "mana_gen": fmt(8.2 * (2 ** i)),
        "mana_cap": fmt(57.8 * (2 ** i)),
    }


def magic_leaf_attr(i, tier):
    return {
        "mana_gen": fmt(2.5 * (2 ** i)),
        "mana_cap": fmt(57.8 * (2 ** i)),
        "constant_heal": "0",
    }


def magic_cactus_attr(i, tier):
    return {"mana_cap": fmt(115.5 * (2 ** i)), "extra_health": fmt(scale3(0, tier))}


def laser_attr(i, tier):
    return {
        "lightning_damage": fmt(scale3(50.0, tier)),
        "lightning_chains": "1",
    }


def electric_web_attr(i, tier):
    radii = [50, 60, 70, 80, 100, 150, 200, 250, 300]
    return {
        "secondary_reload": "0.5",
        "lightning_damage": fmt(scale3(5.0, tier)),
        "aura_radius": fmt(float(radii[i])),
        "aura_duration": "10.0",
    }


def domino_attr(i, tier):
    return {"domino_max_damage": fmt(scale3(36.0, tier))}


def card_attr(i, tier):
    return {"card_random_stats": "1"}


def mimic_attr(i, tier):
    return {"mimic_left": "1"}


def compass_attr(i, tier):
    return {"compass": "1", "secondary_reload": fmt(9.0 - i)}


def blood_sac_attr(i, tier):
    return {"blood_sacrifice": "1"}


def pharaoh_attr(i, tier):
    return {"spawns": "(uint8_t)MobID::kBeetle", "spawn_count": "1", "secondary_reload": "1.0"}


def champion_attr(i, tier):
    return {"champion_crown": "1"}


def soul_attr(i, tier):
    return {"soul_splitter": "1"}


def goggles_attr(i, tier):
    return {"vision_factor": "0.80", "paranormal": "1"}


def corruption_attr(i, tier):
    return {"corruption": "1"}


def dark_mark_attr(i, tier):
    return {"dark_mark": "1", "secondary_reload": fmt(max(2.0, 960.0 / (2 ** i)))}


def amulet_attr(i, tier):
    return {"overheal_to_shield": fmt(0.05 * (i + 1))}


def privet_attr(i, tier):
    return {"privet_extra_poison": "0.2"}


def blueberries_attr(i, tier):
    return {
        "split_projectile": "1",
        "lightning_damage": fmt(scale3(30.0, tier)),
        "lightning_chains": "1",
    }


def magic_cotton_attr(i, tier):
    absorb = [0.5, 0.8, 1.1, 1.7, 2.5, 3.8, 5.7, 8.5, 12.8]
    return {"mana_absorb_per": fmt(absorb[i])}


def magic_eye_attr(i, tier):
    dmg = [3.3, 4.9, 7.4, 11.1, 16.7, 25.1, 37.6, 56.4, 84.6]
    return {"mana_lightning_per": fmt(dmg[i]), "lightning_chains": "7"}


def magic_stinger_attr(i, tier):
    return {}


def magic_missile_attr(i, tier):
    return {"split_projectile": "1"}


def magic_stick_attr(i, tier):
    return {"spawns": "MobID::kSandstorm", "spawn_count": "1", "mana_spawn_cost": fmt(20.0 * (2 ** i))}


def magic_bubble_attr(i, tier):
    return {"defend_only": "1"}


def powder_attr(i, tier):
    return {
        "move_speed_factor": fmt(1.0 + 0.25 * (i + 1)),
        "powder_max_rarity": str(min(i, 6)),
    }


def relic_attr(i, tier):
    return {"extra_health": fmt(0.10 * (i + 1))}


# (base_enum, en_name, zh_name, zh_desc, hp0, dmg0, reload, radius, count, attr_fn, unique_eq_super, drop_mobs, phase, tags)
PETALS = [
    # Phase 0
    ("Lentil", "Lentil", "小扁豆", "增加花瓣对敌人的吸引力。", 12, 13, 2.0, 9, 1, lentil_attr, True, ["Beetle", "Massive Beetle"], 0, "attraction"),
    ("Sawblade", "Sawblade", "锯片", "这看起来会很疼。", 25, 40, 2.5, 12, 1, saw_attr, True, ["Wandering Guard", "Rust Mite", "Mimic Chest"], 0, "dps,fixed_range"),
    # Phase 2 light
    ("Dice", "Dice", "骰子", "有小概率造成35倍伤害，幸运值提高概率。", 10, 10, 2.5, 10, 1, dice_attr, True, ["Square"], 2, "crit,luck"),
    ("Domino", "Domino", "多米诺骨牌", "伤害取决于点数。", 16, 18, 1.0, 10, 1, domino_attr, True, ["Square"], 2, "random"),
    ("Card", "Card", "纸牌", "一张扑克牌。它似乎被动了手脚。", 25, 10, 2.5, 10, 1, card_attr, True, ["Square"], 2, "random"),
    ("PokerChip", "Poker Chip", "扑克筹码", "孤注一掷。", 1, 7, 2.5, 9, 1, poker_attr, True, ["Square"], 2, "evasion"),
    ("Bean", "Bean", "豆子", "周围没有友方花朵时造成更多伤害。", 12, 20, 2.0, 10, 1, bean_attr, True, ["Ladybug"], 2, "solo"),
    ("Clay", "Clay", "黏土", "在空气中吸附灰尘，变得更大更强。", 10, 5, 2.0, 11, 1, clay_attr, True, ["Rock", "Boulder"], 2, "travel"),
    ("Tomato", "Tomato", "番茄", "随着时间推移变得更强。", 10, 5, 2.5, 11, 1, tomato_attr, True, ["Ladybug"], 2, "charge"),
    ("Dust", "Dust", "灰尘", "有时会粘在一起，有时不会。", 1.7, 4.3, 0.8, 6, 1, dust_attr, True, ["Sandstorm"], 2, "light"),
    ("Broccoli", "Broccoli", "西兰花", "碰到东西就会爆炸。", 15, 10, 2.8, 11, 1, broccoli_attr, True, ["Cactus"], 2, "splash"),
    ("Orange", "Orange", "橙子", "一种美味的柑橘类水果。", 6.7, 8, 3.5, 9, 3, orange_attr, True, ["Bee"], 2, "clump"),
    ("Plank", "Plank", "木板", "对弹射物和花瓣造成20倍伤害。", 15, 18, 2.5, 12, 1, plank_attr, True, ["Tunnel Guard"], 2, "anti_proj"),
    ("Bur", "Bur", "刺果", "降低目标护甲，效果不叠加。", 5, 5, 2.0, 8, 1, bur_attr, True, ["Spider"], 2, "armor_shred"),
    ("Soil", "Soil", "土", "增加生命，同时增大体型并降低移速。", 10, 10, 2.5, 12, 1, soil_attr, True, ["Rock"], 2, "tank"),
    ("Air", "Air", "空气", "字面意思的什么也没有。", 1, 0, 2.5, 1, 1, air_attr, True, ["Moon Jelly"], 2, "size"),
    ("Basil", "Basil", "罗勒", "提高所有受到的治疗效果。", 1, 0, 2.5, 8, 1, basil_attr, True, ["Ladybug"], 2, "heal_amp"),
    ("Cotton", "Cotton", "棉花", "吸收花朵受到的伤害。", 2, 0, 1.5, 10, 1, cotton_attr, True, ["Baby Ant"], 2, "absorb"),
    ("Rubber", "Rubber", "橡胶", "增加受到的击退，吸收闪电伤害。", 30, 1, 2.5, 12, 1, rubber_attr, True, ["Moon Jelly"], 2, "knockback"),
    ("Carrot", "Carrot", "胡萝卜", "坚固可靠，可发射并从墙上反弹。", 18, 18, 1.0, 9, 1, carrot_attr, True, ["Hornet"], 2, "projectile"),
    ("Grapes", "Grapes", "葡萄", "嘭地爆开，带有剧毒。", 5, 3, 1.5, 8, 4, grapes_attr, True, ["Centipede", "Evil Centipede"], 2, "poison_split"),
    ("DeadLeaf", "Dead Leaf", "枯叶", "提升花瓣耐久但提高冷却。", 12, 16, 1.8, 10, 1, dead_leaf_attr, True, ["Leaf Beetle"], 2, "hp_factor"),
    ("Clover", "Clover", "幸运草", "增加幸运值。", 10, 10, 2.5, 9, 1, clover_attr, True, ["Ladybug"], 2, "luck"),
    ("Totem", "Totem", "图腾", "击杀后获得一段时间生命恢复。", 1, 0, 2.5, 12, 1, totem_attr, True, ["Massive Ladybug"], 2, "on_kill_heal"),
    ("Root", "Root", "根", "周期性提供额外护甲层。", 1, 0, 5.0, 10, 1, root_attr, True, ["Cactus"], 2, "armor_stack"),
    # Phase 3 mid
    ("Lightning", "Lightning", "闪电", "连锁攻击附近的多个敌人。", 20, 0, 2.5, 10, 1, lightning_attr, True, ["Hornet", "Cinder Moth"], 3, "lightning"),
    ("Battery", "Battery", "电池", "花朵受伤时释放闪电。", 1, 0, 2.5, 10, 1, battery_attr, True, ["Scorpion"], 3, "lightning"),
    ("Laser", "Laser", "激光器", "咻咻——", 10, 5, 5.0, 10, 1, laser_attr, True, ["Cinder Moth"], 3, "lightning"),
    ("ElectricWeb", "Electric Web", "电网", "没那么粘人，倒有些雷人。", 5, 5, 3.0, 10, 1, electric_web_attr, True, ["Spider"], 3, "lightning,web"),
    ("Uranium", "Uranium", "铀", "周期性释放辐射。", 75, 0, 2.5, 12, 1, uranium_attr, True, ["Scorpion"], 3, "aura_poison"),
    ("Sponge", "Sponge", "海绵", "分散花朵受到的伤害。", 1, 0, 2.5, 12, 1, sponge_attr, True, ["Moon Jelly"], 3, "damage_delay"),
    ("Fang", "Fang", "尖牙", "基于此花瓣造成的伤害提供治疗。", 1, 15, 3.5, 8, 1, fang_attr, True, ["Scorpion", "Spider"], 3, "lifesteal"),
    ("Shell", "Shell", "贝壳", "为你或队友添加临时护盾。", 25, 5, 3.5, 11, 1, shell_attr, True, ["Coral Star"], 3, "shield"),
    ("Claw", "Claw", "爪子", "目标生命值高于80%时额外伤害。", 10, 5, 3.5, 10, 1, claw_attr, True, ["Reef Crab", "River Crab"], 3, "execute_high"),
    ("Glass", "Glass", "玻璃", "穿透敌人，间隔内只能伤害一次。", 1, 22, 2.5, 9, 1, glass_attr, True, ["Rock"], 3, "phase"),
    ("LightBulb", "Light Bulb", "灯泡", "闪亮的灯泡，吸引生物攻击。", 10, 5, 1.0, 10, 1, lightbulb_attr, True, ["Cinder Moth"], 3, "taunt"),
    ("Poo", "Poo", "便便", "让其他生物不太愿意攻击你。", 5, 5, 2.5, 10, 1, poo_attr, True, ["Baby Ant"], 3, "aggro"),
    ("Ankh", "Ankh", "安卡", "被摧毁后传送回标记点。", 10, 0, 256.0, 10, 1, ankh_attr, True, ["Dune Beetle"], 3, "teleport"),
    ("Shovel", "Shovel", "铲子", "遁入地下暂时无法被攻击。", 1, 0, 15.0, 12, 1, shovel_attr, True, ["Dune Beetle"], 3, "burrow"),
    ("Bandage", "Bandage", "绷带", "临死时延缓死亡一段时间。", 1, 0, 2.5, 10, 1, bandage_attr, True, ["Bee"], 3, "death_delay"),
    ("Wax", "Wax", "蜂蜡", "别问它是从哪里来的。", 1000, 0, 30.0, 14, 1, wax_attr, True, ["Bee", "Hornet"], 3, "block"),
    ("Amulet", "Amulet", "护符", "将溢出治疗转化为护盾。", 10, 0, 2.0, 10, 1, amulet_attr, True, ["Ladybug"], 3, "overheal"),
    ("PrivetBerry", "Privet Berry", "女贞子", "在已有剧毒基础上造成额外剧毒。", 1, 0, 4.0, 8, 1, privet_attr, True, ["Scorpion"], 3, "poison"),
    ("Blueberries", "Blueberries", "蓝莓", "嘭地爆开，带有闪电。", 5, 0, 1.2, 8, 4, blueberries_attr, True, ["Bee"], 3, "lightning_split"),
    # Phase 4 infra-dependent (data + stubs)
    ("Fragment", "Fragment", "碎片", "也许这能锻造出一些神秘的东西？", 1, 1, 10.0, 8, 1, fragment_attr, True, ["Wandering Guard", "Rust Mite"], 4, "forge"),
    ("Cogwheel", "Cogwheel", "齿轮", "似乎没什么作用……", 13, 19, 2.5, 11, 1, cog_attr, True, ["Wandering Guard"], 4, "forge"),
    ("ManaOrb", "Mana Orb", "魔法球", "创造魔力。", 25, 0, 2.5, 11, 1, mana_orb_attr, True, ["Cinder Moth"], 4, "mana"),
    ("MagicLeaf", "Magic Leaf", "魔法叶子", "从环境中收集魔力。", 12, 16, 1.8, 10, 1, magic_leaf_attr, True, ["Leaf Beetle"], 4, "mana"),
    ("MagicCactus", "Magic Cactus", "魔法仙人掌", "提高最大魔力容量。", 15, 7, 1.0, 12, 1, magic_cactus_attr, True, ["Cactus"], 4, "mana"),
    ("MagicCotton", "Magic Cotton", "魔法棉花", "用魔力吸收伤害。", 1, 0, 1.5, 10, 1, magic_cotton_attr, True, ["Baby Ant"], 4, "mana"),
    ("MagicEye", "Magic Eye", "魔法之眼", "消耗一半魔力造成闪电伤害。", 1, 0, 1.0, 10, 1, magic_eye_attr, True, ["Phantom Orb"], 4, "mana"),
    ("MagicStinger", "Magic Stinger", "魔法刺", "攻击很强，却十分脆弱。", 1, 100, 10.0, 7, 1, magic_stinger_attr, True, ["Bee"], 4, "mana"),
    ("MagicMissile", "Magic Missile", "魔法导弹", "用魔法发射出去。", 2, 35, 5.4, 8, 1, magic_missile_attr, True, ["Hornet"], 4, "mana"),
    ("MagicStick", "Magic Stick", "魔法树枝", "用魔力召唤沙尘暴。", 1, 0, 4.0, 10, 1, magic_stick_attr, True, ["Sandstorm"], 4, "mana"),
    ("MagicBubble", "Magic Bubble", "魔法泡泡", "无魔法者才会遵守物理定律。", 1, 0, 5.0, 12, 1, magic_bubble_attr, True, ["Moon Jelly"], 4, "mana"),
    ("Talisman", "Talisman of Evasion", "闪避护符", "让佩戴者闪避敌人攻击。", 10, 10, 2.5, 10, 1, talisman_attr, True, ["Spider"], 4, "evasion"),
    ("Nazar", "Nazar Amulet", "邪眼护符", "受伤时大幅减伤并消耗充能。", 1, 0, 60.0, 10, 1, nazar_attr, True, ["Scorpion"], 4, "mitigation"),
    ("Compass", "Compass", "指南针", "也许可以找到一些有趣的东西……", 40, 1, 2.5, 10, 1, compass_attr, True, ["Sandstorm"], 4, "hel"),
    ("Mimic", "Mimic", "拟态", "复制左侧同品质花瓣。", 1, 0, 2.5, 10, 1, mimic_attr, True, ["Mimic Chest"], 4, "copy"),
    ("Powder", "Mysterious Powder", "神秘的粉末", "增加移速，受伤或靠近高稀有度怪时失效。", 10, 0, 4.0, 8, 1, powder_attr, True, ["Sandstorm"], 4, "speed"),
    ("Relic", "Mysterious Relic", "神秘的遗物", "显著提高最大生命值。", 1, 0, 2.5, 10, 1, relic_attr, True, ["Mimic Chest"], 4, "tank"),
    ("DarkMark", "Dark Mark", "暗黑标记", "将陨落者困于冥界。", 10, 0, 1.0, 10, 1, dark_mark_attr, True, ["Massive Beetle"], 4, "hel"),
    ("Corruption", "Corruption", "腐化", "腐蚀灵魂，无法轻易卸下。", 1, 0, 2.5, 10, 1, corruption_attr, True, ["Evil Centipede"], 4, "hel"),
    ("BloodSacrifice", "Blood Sacrifice", "血祭", "死亡后召唤一只生物再次战斗。", 1, 0, 10.0, 10, 1, blood_sac_attr, True, ["Queen Ant"], 4, "hel"),
    ("PharaohCrown", "Pharaoh's Crown", "法老之冠", "召唤不死甲虫并肩作战。", 1, 0, 1.0, 12, 1, pharaoh_attr, True, ["Massive Beetle"], 4, "hel"),
    ("ChampionCrown", "Champion's Crown", "王者之冠", "给周围花朵超级基本。", 1, 0, 2.5, 12, 1, champion_attr, True, ["Wandering Guard"], 4, "hel"),
    ("SoulSplitter", "Soul Splitter", "灵魂分裂", "将灵魂一分为二。", 1, 0, 2.5, 10, 1, soul_attr, True, ["Phantom Orb"], 4, "hel"),
    ("Goggles", "Paranormal Goggles", "超自然护目镜", "允许发现异常活动。", 1, 0, 2.5, 10, 1, goggles_attr, True, ["Cinder Moth"], 4, "hel"),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="surrogateescape")


def write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", errors="surrogateescape", newline="\n")


def ensure_attrs(hh: str) -> str:
    """Add missing PetalAttributes fields used by wiki petals."""
    extras = [
        ("crit_chance", "float crit_chance = 0;"),
        ("crit_factor", "float crit_factor = 1;"),
        ("armor_shred", "float armor_shred = 0;"),
        ("armor_shred_seconds", "float armor_shred_seconds = 0;"),
        ("body_radius_bonus", "float body_radius_bonus = 0;"),
        ("heal_amp", "float heal_amp = 0;"),
        ("damage_absorb", "float damage_absorb = 0;"),
        ("knockback_taken", "float knockback_taken = 0;"),
        ("solo_damage_factor", "float solo_damage_factor = 1;"),
        ("petal_evasion", "float petal_evasion = 0;"),
        ("flower_evasion", "float flower_evasion = 0;"),
        ("nazar_charges", "uint8_t nazar_charges = 0;"),
        ("nazar_mitigation", "float nazar_mitigation = 0;"),
        ("petal_hp_factor", "float petal_hp_factor = 1;"),
        ("luck", "float luck = 0;"),
        ("clay_max_damage", "float clay_max_damage = 0;"),
        ("tomato_max_damage", "float tomato_max_damage = 0;"),
        ("vs_projectile_factor", "float vs_projectile_factor = 1;"),
        ("lightning_damage", "float lightning_damage = 0;"),
        ("lightning_chains", "uint8_t lightning_chains = 0;"),
        ("battery_charges", "uint8_t battery_charges = 0;"),
        ("damage_delay_seconds", "float damage_delay_seconds = 0;"),
        ("on_hit_heal_ratio", "float on_hit_heal_ratio = 0;"),
        ("hit_interval_seconds", "float hit_interval_seconds = 0;"),
        ("phase_through", "uint8_t phase_through = 0;"),
        ("aggro_shrink", "float aggro_shrink = 0;"),
        ("ankh_mark", "uint8_t ankh_mark = 0;"),
        ("shovel_burrow", "uint8_t shovel_burrow = 0;"),
        ("death_delay_seconds", "float death_delay_seconds = 0;"),
        ("totem_heal", "float totem_heal = 0;"),
        ("totem_seconds", "float totem_seconds = 0;"),
        ("root_armor", "float root_armor = 0;"),
        ("root_interval", "float root_interval = 0;"),
        ("root_max_stacks", "uint8_t root_max_stacks = 0;"),
        ("wax_block", "uint8_t wax_block = 0;"),
        ("mana_gen", "float mana_gen = 0;"),
        ("mana_cap", "float mana_cap = 0;"),
        ("mana_absorb_per", "float mana_absorb_per = 0;"),
        ("mana_lightning_per", "float mana_lightning_per = 0;"),
        ("mana_spawn_cost", "float mana_spawn_cost = 0;"),
        ("domino_max_damage", "float domino_max_damage = 0;"),
        ("card_random_stats", "uint8_t card_random_stats = 0;"),
        ("mimic_left", "uint8_t mimic_left = 0;"),
        ("compass", "uint8_t compass = 0;"),
        ("blood_sacrifice", "uint8_t blood_sacrifice = 0;"),
        ("champion_crown", "uint8_t champion_crown = 0;"),
        ("soul_splitter", "uint8_t soul_splitter = 0;"),
        ("paranormal", "uint8_t paranormal = 0;"),
        ("corruption", "uint8_t corruption = 0;"),
        ("dark_mark", "uint8_t dark_mark = 0;"),
        ("overheal_to_shield", "float overheal_to_shield = 0;"),
        ("privet_extra_poison", "float privet_extra_poison = 0;"),
        ("powder_max_rarity", "uint8_t powder_max_rarity = 0;"),
    ]
    if "crit_chance" in hh:
        return hh
    insert_after = "float stack_damage = 0;          // Triangle: +stack per same-ID copy equipped"
    if insert_after not in hh:
        raise SystemExit("stack_damage anchor missing")
    block = "\n".join(f"    {decl} // wiki import" for _, decl in extras)
    return hh.replace(insert_after, insert_after + "\n" + block, 1)


def patch_enum(hh: str, bases: list[str]) -> str:
    if "kLentil" in hh:
        # already has some — append only missing
        pass
    marker = "        kTriangleUnique,\n        kNumPetals"
    if marker not in hh:
        marker = "        kTriangleUnique,\n\n        kNumPetals"
    if "kNumPetals" not in hh:
        raise SystemExit("kNumPetals missing")
    existing = set(re.findall(r"\bk([A-Za-z0-9]+)\b", hh))
    blocks = []
    for base, en, zh, *_rest in PETALS:
        if en is None:
            continue
        if f"k{base}" in existing or base in existing:
            # check Common id
            if f"{base}" in existing or any(f"k{base}" == x or x.startswith(base) for x in []):
                pass
        if re.search(rf"\bk{base}\b", hh):
            continue
        blocks.append(enum_block(base, f"{en} / {zh} — wiki import"))
    if not blocks:
        return hh
    # Insert before kNumPetals after TriangleUnique
    anchor = "        kTriangleUnique,"
    if anchor not in hh:
        raise SystemExit("TriangleUnique anchor missing")
    return hh.replace(
        anchor + "\n        kNumPetals",
        anchor + "\n" + "\n".join(blocks) + "\n        kNumPetals",
        1,
    )


def patch_petal_data(cc: str) -> str:
    if '.name = "Lentil"' in cc:
        print("PETAL_DATA already has Lentil — skip data insert")
        return cc
    parts = []
    for base, en, zh, desc, hp, dmg, reload, radius, count, attr_fn, ues, drops, phase, tags in PETALS:
        if en is None:
            continue
        parts.append(
            petal_entries(en, desc.replace('"', '\\"'), hp, dmg, reload, radius, count, attr_fn, ues)
        )
    blob = ",\n    // ---- Wiki import wave ----\n" + ",\n".join(parts)
    # Insert before closing of PETAL_DATA array `}};` that precedes MOB_DATA
    needle = "        .attributes = { .stack_damage = 26244.0 }\n    }\n}};"
    if needle not in cc:
        # try Unique triangle without attrs formatting
        needle = re.search(
            r'(\{\s*\n\s*\.name = "Triangle",.*?\.rarity = RarityID::kUnique.*?)\n\}\};\s*\n\s*\nstd::array<struct MobData',
            cc,
            re.S,
        )
        if not needle:
            raise SystemExit("could not find Triangle Unique end")
        return cc[: needle.end(1)] + "\n    }" + blob + "\n}};\n\nstd::array<struct MobData" + cc[needle.end() :]
    return cc.replace(needle, "        .attributes = { .stack_damage = 26244.0 }\n    }" + blob + "\n}};", 1)


def patch_drops(cc: str) -> str:
    # Map mob English name -> petal seed ids to prepend
    mapping: dict[str, list[str]] = {}
    for p in PETALS:
        base, en = p[0], p[1]
        if en is None:
            continue
        drops = p[11]
        for mob in drops:
            mapping.setdefault(mob, []).append(f"PetalID::k{base}")

    def repl_mob(m: re.Match) -> str:
        name = m.group(1)
        body = m.group(0)
        if name not in mapping:
            return body
        ids = mapping[name]
        # find drops = { ... }
        dm = re.search(r"\.drops = \{([^}]*)\}", body, re.S)
        if not dm:
            return body
        inner = dm.group(1).strip()
        for pid in ids:
            if pid in inner:
                continue
            if inner:
                inner = "\n            " + pid + ", " + inner.lstrip()
            else:
                inner = " " + pid + " "
        return body[: dm.start()] + ".drops = {" + inner + "}" + body[dm.end() :]

    # Replace each mob block carefully — match from .name = "X" through attributes closing
    for mob in list(mapping.keys()):
        pat = re.compile(
            rf'(\{{\s*\n\s*\.name = "{re.escape(mob)}",.*?\n\s*\.attributes = \{{.*?\}}\s*\n\s*\}})',
            re.S,
        )
        cc2, n = pat.subn(repl_mob, cc, count=1)
        if n == 0:
            print(f"warn: mob not found for drops: {mob}")
        else:
            cc = cc2
    return cc


def zh_lines(items: list[str], n: int) -> str:
    return ",\n".join(f'    "{s}"' for s in items for _ in range(n)) if False else ""


def patch_i18n() -> None:
    for path, is_desc in ((I18N, False), (NAME_ZH, False), (DESC_ZH, True)):
        text = read(path)
        # Determine which array to patch
        if path == I18N:
            # names before PETAL_DESC_ZH, descs before MOB_NAME_ZH
            name_anchor = '    "三角形"\n};'
            # I18n has awkward comma before triangle sometimes
            if '    "三角形"\n};' not in text and '    "三角形"\n,\n    "三角形"' not in text:
                # find last triangle block
                pass
            adds_name = []
            adds_desc = []
            for base, en, zh, desc, *_ in PETALS:
                if en is None:
                    continue
                if zh in text and text.count(f'"{zh}"') >= 9 and base != "Lentil":
                    # may already exist for some
                    pass
                adds_name.extend([zh] * 9)
                adds_desc.extend([desc] * 9)
            # Append after last 三角形 in NAME section
            m = re.search(
                r'(static char const \*const PETAL_NAME_ZH\[PetalID::kNumPetals\] = \{.*?)(\n\};)\nstatic char const \*const PETAL_DESC_ZH',
                text,
                re.S,
            )
            if not m:
                raise SystemExit("PETAL_NAME_ZH block missing")
            body = m.group(1)
            if '"小扁豆"' not in body:
                extra = ",\n" + ",\n".join(f'    "{s}"' for s in adds_name)
                # trim trailing and add
                body = body.rstrip()
                if not body.endswith(","):
                    # last entry without comma ok — add comma before extra
                    pass
                body2 = body + extra
                text = text[: m.start(1)] + body2 + m.group(2) + "\nstatic char const *const PETAL_DESC_ZH" + text[m.end() :]
            m2 = re.search(
                r'(static char const \*const PETAL_DESC_ZH\[PetalID::kNumPetals\] = \{.*?)(\n\};)\nstatic char const \*const MOB_NAME_ZH',
                text,
                re.S,
            )
            if not m2:
                raise SystemExit("PETAL_DESC_ZH block missing")
            body = m2.group(1)
            if "增加花瓣对敌人的吸引力" not in body:
                # also ensure Triangle descs exist
                if "它的量变给我们带来了真正的质变" not in body:
                    tri = ",\n" + ",\n".join(['    "它的量变给我们带来了真正的质变。"'] * 9)
                    body = body.rstrip() + tri
                extra = ",\n" + ",\n".join(f'    "{s}"' for s in adds_desc)
                body = body.rstrip() + extra
                text = text[: m2.start(1)] + body + m2.group(2) + "\nstatic char const *const MOB_NAME_ZH" + text[m2.end() :]
            write(path, text)
            continue

        # Shared PetalNameZh / PetalDescZh
        text = read(path)
        adds_name = []
        adds_desc = []
        for base, en, zh, desc, *_ in PETALS:
            if en is None:
                continue
            adds_name.extend([zh] * 9)
            adds_desc.extend([desc] * 9)
        if path == NAME_ZH:
            if '"小扁豆"' in text:
                print("PetalNameZh already has 小扁豆")
                continue
            text = text.rstrip()
            if text.endswith("};"):
                text = text[:-2].rstrip()
                if not text.endswith(","):
                    text += ","
                text += "\n" + ",\n".join(f'    "{s}"' for s in adds_name) + "\n};\n"
            write(path, text)
        else:
            if "增加花瓣对敌人的吸引力" in text:
                print("PetalDescZh already has lentil")
                continue
            text = text.rstrip()
            if text.endswith("};"):
                text = text[:-2].rstrip()
                if not text.endswith(","):
                    text += ","
                # fill triangle if missing
                if "它的量变给我们带来了真正的质变" not in text:
                    text += "\n" + ",\n".join(['    "它的量变给我们带来了真正的质变。"'] * 9) + ","
                text += "\n" + ",\n".join(f'    "{s}"' for s in adds_desc) + "\n};\n"
            write(path, text)


def patch_storage(text: str, count: int) -> str:
    if "kWikiImportAdded" in text:
        return text
    old = "                constexpr uint32_t kTriangleAdded = 9; // Triangle Common..Unique"
    new = (
        f"                constexpr uint32_t kWikiImportAdded = {count}; // wiki petal import wave\n"
        "                constexpr uint32_t kTriangleAdded = 9; // Triangle Common..Unique"
    )
    text = text.replace(old, new, 1)
    text = text.replace(
        "                uint32_t const prev_triangle = sub(n, kTriangleAdded);",
        "                uint32_t const prev_wiki = sub(n, kWikiImportAdded);\n"
        "                uint32_t const prev_triangle = sub(prev_wiki, kTriangleAdded);",
        1,
    )
    text = text.replace(
        "                    n, prev_triangle, prev_honey_meteor_ladder,",
        "                    n, prev_wiki, prev_triangle, prev_honey_meteor_ladder,",
        1,
    )
    return text


def patch_art(text: str) -> str:
    if "case PetalID::kLentil:" in text:
        return text
    art = r'''
        case PetalID::kLentil: {
            ctx.save();
            ctx.rotate(0.785398f);
            ctx.set_fill(0xffebb134);
            ctx.set_stroke(0xffc4942d);
            ctx.set_line_width(3);
            ctx.begin_path();
            ctx.ellipse(0, 0, r * 0.85f, r * 1.05f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0x66ffe9a0);
            ctx.begin_path();
            ctx.ellipse(-r * 0.2f, -r * 0.25f, r * 0.28f, r * 0.4f);
            ctx.fill();
            ctx.restore();
            break;
        }
        case PetalID::kSawblade: {
            ctx.set_fill(0xff9a9a9a);
            ctx.set_stroke(0xff6e6e6e);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.55f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xffb8b8b8);
            ctx.set_stroke(0xff5a5a5a);
            ctx.set_line_width(2);
            ctx.begin_path();
            uint32_t const teeth = 12;
            for (uint32_t i = 0; i < teeth; ++i) {
                float a0 = (float) i * (float) (2 * M_PI) / (float) teeth;
                float a1 = a0 + (float) (2 * M_PI) / (float) teeth;
                float amid = (a0 + a1) * 0.5f;
                float tip = amid - 0.08f;
                if (i == 0) ctx.move_to(cosf(a0) * r * 0.62f, sinf(a0) * r * 0.62f);
                ctx.line_to(cosf(tip) * r, sinf(tip) * r);
                ctx.line_to(cosf(a1) * r * 0.62f, sinf(a1) * r * 0.62f);
            }
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff5a5a5a);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.22f);
            ctx.fill();
            ctx.set_fill(0xff3a3a3a);
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.12f);
            ctx.fill();
            break;
        }
        case PetalID::kDice: {
            ctx.set_fill(0xffffffff);
            ctx.set_stroke(0xff333333);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.round_rect(-r, -r, 2 * r, 2 * r, r * 0.25f);
            ctx.fill();
            ctx.stroke();
            ctx.set_fill(0xff222222);
            for (float px : {-0.4f, 0.4f})
                for (float py : {-0.4f, 0.4f}) {
                    ctx.begin_path();
                    ctx.arc(px * r, py * r, r * 0.14f);
                    ctx.fill();
                }
            ctx.begin_path();
            ctx.arc(0, 0, r * 0.14f);
            ctx.fill();
            break;
        }
        case PetalID::kLightning: {
            ctx.set_fill(0xffffe066);
            ctx.set_stroke(0xffc9a227);
            ctx.set_line_width(2.5f);
            ctx.begin_path();
            ctx.move_to(-0.2f * r, -r);
            ctx.line_to(0.35f * r, -0.1f * r);
            ctx.line_to(0.05f * r, -0.1f * r);
            ctx.line_to(0.4f * r, r);
            ctx.line_to(-0.35f * r, 0.15f * r);
            ctx.line_to(-0.05f * r, 0.15f * r);
            ctx.close_path();
            ctx.fill();
            ctx.stroke();
            break;
        }
'''
    # Generic colored ovals for remaining wiki petals via fallthrough to default is OK,
    # but add a few distinctive ones; others use default rarity oval.
    return text.replace(
        "        case PetalID::kMoon: {",
        art + "\n        case PetalID::kMoon: {",
        1,
    )


def write_gap_md() -> None:
    lines = [
        "# Wiki petal gap (generated)",
        "",
        "Source: florrio.fandom.com/zh/wiki/花瓣",
        "",
        "| Phase | EN | ZH | Tags | Drop mobs |",
        "| --- | --- | --- | --- | --- |",
    ]
    for p in PETALS:
        base, en, zh = p[0], p[1], p[2]
        if en is None:
            continue
        drops, phase, tags = p[11], p[12], p[13]
        lines.append(f"| {phase} | {en} | {zh} | {tags} | {', '.join(drops)} |")
    lines += [
        "",
        "## Notes",
        "- Eternal rarity folded into Unique.",
        "- Hel / Mana / Forge petals ship with attrs; full systems stubbed where needed.",
        "- Ashflor-original craft petals retained.",
        "",
    ]
    GAP_MD.parent.mkdir(parents=True, exist_ok=True)
    GAP_MD.write_text("\n".join(lines), encoding="utf-8")


def patch_codex(text: str) -> str:
    if "attraction_radius" in text and "crit_chance" in text:
        return text
    anchor = '    jkv_f(out, "bonus_vs_poisoned", a.bonus_vs_poisoned, f);'
    if "attraction_radius" not in text:
        text = text.replace(
            anchor,
            anchor
            + '\n    jkv_f(out, "stack_damage", a.stack_damage, f);'
            + '\n    jkv_f(out, "attraction_radius", a.attraction_radius, f);'
            + '\n    jkv_u(out, "fixed_orbit_range", a.fixed_orbit_range, f);'
            + '\n    jkv_f(out, "crit_chance", a.crit_chance, f);'
            + '\n    jkv_f(out, "luck", a.luck, f);'
            + '\n    jkv_f(out, "lightning_damage", a.lightning_damage, f);'
            + '\n    jkv_f(out, "mana_gen", a.mana_gen, f);',
            1,
        )
    return text


def patch_damage_wiring(text: str) -> str:
    """Wire crit, armor shred, lifesteal ratio, solo factor, evasion hooks."""
    if "wiki_crit_applied" in text:
        return text
    # Find a good place after damage amount computed — look for inflict_damage patterns
    # Simpler: inject helper comments near top and a block in petal collision path
    marker = "void inflict_damage(Simulation *sim, EntityID const attacker_id,"
    # We'll add post-processing in a known petal damage site
    return text  # wired in separate smaller patch below


def main() -> None:
    active = [p for p in PETALS if p[1] is not None]
    n_ids = len(active) * 9
    print(f"importing {len(active)} families → {n_ids} ids")

    hh = read(HH)
    hh = ensure_attrs(hh)
    hh = patch_enum(hh, [p[0] for p in active])
    write(HH, hh)
    print("patched StaticDefinitions.hh")

    cc = read(CC)
    cc = patch_petal_data(cc)
    cc = patch_drops(cc)
    write(CC, cc)
    print("patched StaticData.cc")

    patch_i18n()
    print("patched i18n")

    st = read(STORAGE)
    st = patch_storage(st, n_ids)
    write(STORAGE, st)
    print("patched Storage.cc")

    art = read(PETAL_CC)
    art = patch_art(art)
    write(PETAL_CC, art)
    print("patched Petal.cc art")

    codex = read(CODEX)
    write(CODEX, patch_codex(codex))
    print("patched Codex.cc")

    write_gap_md()
    print("wrote", GAP_MD)


if __name__ == "__main__":
    main()

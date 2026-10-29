#!/usr/bin/env python3
"""Batch 4 remainder: cross-map assemble products from 花瓣制作组合.txt."""
from __future__ import annotations

import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "retune_assemble_ladders", ROOT / "tools" / "retune_assemble_ladders.py"
)
mod = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(mod)

# Cross-map design defaults to Epic
E = "Epic"
mod.ANCHORS = {
    "Thorn Palm": {"base_rarity": E, "health": 90.0, "damage": 24.0, "damage_reflection": 0.15},
    "Pollen Sandstorm": {"base_rarity": E, "health": 40.0, "damage": 8.0, "slow_inflict_seconds": 3.0},
    "Sand Leaf Dart": {"base_rarity": E, "health": 20.0, "damage": 17.0},
    "Desert Root": {"base_rarity": E, "health": 160.0, "damage": 5.0},
    "Frost Bubble": {"base_rarity": E, "health": 25.0, "damage": 24.0, "slow_inflict_seconds": 2.0},
    "Frost Starfish": {"base_rarity": E, "health": 45.0, "damage": 26.0},
    "Ice Tide Ring": {"base_rarity": E, "health": 50.0, "damage": 18.0},
    "Deep Frost Pearl": {"base_rarity": E, "health": 85.0, "damage": 30.0},
    "Poison Ant Nest": {"base_rarity": E, "health": 55.0, "damage": 8.0},
    "Corrode Vine": {"base_rarity": E, "health": 40.0, "damage": 23.0},
    "Colony Core": {"base_rarity": E, "health": 50.0, "damage": 7.0},
    "Symbiote Shell": {"base_rarity": E, "health": 190.0, "damage": 5.0},
    "Forged Holy Sword": {"base_rarity": E, "health": 55.0, "damage": 48.0},
    "Flame Gear": {"base_rarity": E, "health": 60.0, "damage": 34.0},
    "Obsidian King Shield": {"base_rarity": E, "health": 240.0, "damage": 8.0, "damage_reflection": 0.10},
    "Holy Flame Core": {"base_rarity": E, "health": 70.0, "damage": 25.0},
    "Anemone Bloom": {"base_rarity": E, "health": 45.0, "damage": 10.0, "constant_heal": 5.0},
    "Wet Leaf": {"base_rarity": E, "health": 35.0, "damage": 22.0, "slow_inflict_seconds": 2.0},
    "Coral Hive": {"base_rarity": E, "health": 40.0, "damage": 12.0},
    "Life Lotus": {"base_rarity": E, "health": 50.0, "damage": 0.0, "constant_heal": 4.0},
    "Molten Sand Blade": {"base_rarity": E, "health": 40.0, "damage": 40.0},
    "Volcano Worm": {"base_rarity": E, "health": 80.0, "damage": 0.0},
    "Scorch Palm": {"base_rarity": E, "health": 130.0, "damage": 10.0, "damage_reflection": 0.15},
    "Glass Storm": {"base_rarity": E, "health": 35.0, "damage": 16.0},
    "Frost Knight Sword": {"base_rarity": E, "health": 50.0, "damage": 42.0, "slow_inflict_seconds": 2.0},
    "Holy Snowflake": {"base_rarity": E, "health": 40.0, "damage": 5.0, "constant_heal": 3.0},
    "Permafrost Gear": {"base_rarity": E, "health": 70.0, "damage": 28.0},
    "Frost Crown": {"base_rarity": E, "health": 150.0, "damage": 5.0},
    "Royal Thorn": {"base_rarity": E, "health": 55.0, "damage": 36.0},
    "Mech Poison Bee": {"base_rarity": E, "health": 35.0, "damage": 16.0},
    "Holy Spore": {"base_rarity": E, "health": 30.0, "damage": 4.0, "constant_heal": 2.0},
    "Treant Guard": {"base_rarity": E, "health": 260.0, "damage": 8.0},
}

if __name__ == "__main__":
    sys.exit(mod.main())

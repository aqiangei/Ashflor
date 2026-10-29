#!/usr/bin/env python3
"""Batch 3 specials + batch 5 legendary: retune base + resync ladders."""
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

mod.ANCHORS = {
    # Shop / utility — keep Unusual as design rung where that was the original
    "Magnet": {"base_rarity": "Unusual", "health": 10.0, "damage": 5.0},
    "Antennae": {"base_rarity": "Legendary", "health": 5.0, "damage": 3.0},
    "Accelerator": {"base_rarity": "Unusual", "health": 8.0, "damage": 5.0},
    "Third Eye": {"base_rarity": "Mythic", "health": 10.0, "damage": 5.0},
    "Observer": {"base_rarity": "Mythic", "health": 12.0, "damage": 5.0},
    "Cutter": {"base_rarity": "Epic", "health": 15.0, "damage": 25.0},
    "Bone": {"base_rarity": "Rare", "health": 40.0, "damage": 8.0},
    "Salt": {"base_rarity": "Unusual", "health": 10.0, "damage": 12.0},
    "Monstera": {"base_rarity": "Legendary", "health": 30.0, "damage": 5.0},
    "Heaviest": {"base_rarity": "Legendary", "health": 200.0, "damage": 10.0, "reload": 15.0},
    "Lotus": {"base_rarity": "Legendary", "health": 20.0, "damage": 0.0, "constant_heal": 3.0},
    "Yggdrasil": {"base_rarity": "Legendary", "health": 50.0, "damage": 0.0},
    "Moon": {"base_rarity": "Mythic", "health": 25.0, "damage": 15.0},
    "Honey": {"base_rarity": "Rare", "health": 15.0, "damage": 10.0},
    "Meteor": {"base_rarity": "Legendary", "health": 20.0, "damage": 40.0},
    "Golden Leaf": {"base_rarity": "Rare", "health": 15.0, "damage": 10.0, "constant_heal": 2.0},
    # Legendary assemble — design numbers sit on Legendary
    "Heart of Nature": {
        "base_rarity": "Legendary", "health": 220.0, "damage": 5.0,
        "constant_heal": 5.0, "extra_health": 35.0,
    },
    "Abyssal Tide": {"base_rarity": "Legendary", "health": 80.0, "damage": 55.0},
    "Infernal Core": {
        "base_rarity": "Legendary", "health": 180.0, "damage": 65.0,
        "burn_damage": 8.0, "burn_time": 4.0,
    },
    "Chrono Vortex": {
        "base_rarity": "Legendary", "health": 90.0, "damage": 20.0,
        "slow_inflict_seconds": 2.0,
    },
    "Ant King": {"base_rarity": "Legendary", "health": 100.0, "damage": 0.0},
    "World Tree Seed": {
        "base_rarity": "Legendary", "health": 120.0, "damage": 0.0, "constant_heal": 3.0,
    },
    "Skyblade": {"base_rarity": "Legendary", "health": 60.0, "damage": 90.0},
    "Chaos Prism": {"base_rarity": "Unique", "health": 70.0, "damage": 50.0},
}

if __name__ == "__main__":
    sys.exit(mod.main())

#!/usr/bin/env python3
"""Retune classic combat petal bases + resync ladders (batch 2).

Same ladder curve as assemble retune. Keeps mechanics attrs from lowest rung.
"""
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

# Classic florr-line / early combat — modest bumps where ladders felt flat.
mod.ANCHORS = {
    "Basic": {"health": 10.0, "damage": 10.0},
    "Light": {"health": 5.0, "damage": 8.0, "reload": 1.0},
    "Fast": {"health": 5.0, "damage": 8.0, "reload": 1.0},
    "Heavy": {"health": 20.0, "damage": 20.0, "reload": 4.5},
    "Stinger": {"health": 3.0, "damage": 70.0, "reload": 6.5},
    "Twin": {"health": 5.0, "damage": 8.0, "reload": 1.0, "count": 2},
    "Triplet": {"health": 5.0, "damage": 8.0, "reload": 1.0, "count": 3},
    "Leaf": {"health": 10.0, "damage": 8.0, "reload": 1.0, "constant_heal": 1.0},
    "Rose": {"health": 5.0, "damage": 5.0},
    "Iris": {"health": 5.0, "damage": 5.0},
    "Wing": {"health": 15.0, "damage": 10.0},
    "Peas": {"health": 5.0, "damage": 8.0},
    "Dandelion": {"health": 5.0, "damage": 5.0},
    "Cactus": {"health": 20.0, "damage": 5.0, "damage_reflection": 0.10},
    "Rock": {"health": 50.0, "damage": 5.0},
    "Egg": {"health": 20.0, "damage": 1.0},
    "Bubble": {"health": 1.0, "damage": 0.0},
    "Yin Yang": {"health": 15.0, "damage": 15.0},
    "Faster": {"health": 10.0, "damage": 8.0},
    "Pollen": {"health": 5.0, "damage": 8.0},
}

if __name__ == "__main__":
    sys.exit(mod.main())

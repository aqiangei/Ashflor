# -*- coding: utf-8 -*-
"""Restore Shared/StaticData.cc to English; Chinese lives in Client/I18n."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
STATIC = ROOT / "Shared" / "StaticData.cc"

# zh -> en (reverse of previous translation)
ZH2EN = {
    "无": "None",
    "你怎么能看到这个？": "How can you see this?",
    "基本": "Basic",
    "不错的花瓣，不算强也不算弱": "A nice petal, not too strong but not too weak",
    "轻": "Fast",
    "比大多数花瓣弱，但冷却很快": "Weaker than most petals, but reloads very quickly",
    "重": "Heavy",
    "非常坚韧且伤害更高，但冷却很慢": "Very resilient and deals more damage, but reloads very slowly",
    "刺": "Stinger",
    "打人很疼，但非常脆弱": "It really hurts, but it's really fragile",
    "叶子": "Leaf",
    "吸收阳光能量，被动治疗你的花": "Gathers energy from the sun to passively heal your flower",
    "双": "Twin",
    "为什么只要一个？为什么不来两个？！": "Why stop at one? Why not TWO?!",
    "玫瑰": "Rose",
    "治疗能力惊人，但不太擅长战斗": "Its healing properties are amazing. Not so good at combat though",
    "鸢尾": "Iris",
    "毒性很强，但生效需要一些时间": "Very poisonous, but takes a while to do its work",
    "导弹": "Missile",
    "这个真的可以发射出去": "You can actually shoot this one",
    "蒲公英": "Dandelion",
    "特殊效果会阻止受影响单位的治疗": "Its interesting properties prevent healing effects on affected units",
    "泡泡": "Bubble",
    "右键戳破可推动你的花": "You can right click to pop it and propel your flower",
    "更快": "Faster",
    "非常轻，能让其他花瓣转得更快": "It's so light it makes your other petals spin faster",
    "岩石": "Rock",
    "更加耐久，但装填更慢": "Even more durable, but slower to recharge",
    "仙人掌": "Cactus",
    "不太强，但能提高最大生命值": "Not very strong, but somehow increases your maximum health",
    "网": "Web",
    "非常黏": "It's really sticky",
    "翅膀": "Wing",
    "来来去去": "It comes and goes",
    "豌豆": "Peas",
    "四合一套餐": "4 in 1 deal",
    "沙子": "Sand",
    "又粗又糙，还会到处粘": "It's coarse, rough, and gets everywhere",
    "钳子": "Pincer",
    "短暂眩晕并毒害目标": "Stuns and poisons targets for a short duration",
    "三连": "Triplet",
    "来三个怎么样？！": "How about THREE?!",
    "蛋": "Egg",
    "说不定会蹦出有趣的东西": "Something interesting might pop out of this",
    "比普通鸢尾生效更快": "Deals its effects quicker than traditional irises",
    "花粉": "Pollen",
    "哮喘患者慎用": "Asthmatics beware",
    "四合一套餐，还加了秘密调料：毒": "4 in 1 deal, now with a secret ingredient: poison",
    "极其强大的玫瑰，几乎闻所未闻": "Extremely powerful rose, almost unheard of",
    "树枝": "Stick",
    "驾驭风的力量": "Harnesses the power of the wind",
    "触角": "Antennae",
    "让你的花能更远地感知敌人": "Allows your flower to sense foes from farther away",
    "最重": "Heaviest",
    "重到没有任何东西能挡住它": "This thing is so heavy that nothing gets in the way",
    "第三只眼": "Third Eye",
    "让花瓣能伸得更远": "Allows your flower to extend petals further out",
    "观察者": "Observer",
    "洞察一切之人": "The one who sees all",
    "让你的花带毒，敌人接触会中毒": "Turns your flower poisonous. Enemies will take poison damage on contact",
    "盐": "Salt",
    "反弹一部分对花造成的伤害，不可叠加": "Reflects some damage dealt to the flower. Does not stack with itself",
    "极其稀有却毫无用处": "Something incredibly rare and useless",
    "方块": "Square",
    "这个形状……看起来很眼熟……": "This shape... it looks familiar...",
    "月亮": "Moon",
    "这是从哪来的？": "Where did this come from?",
    "莲花": "Lotus",
    "吸收花受到的部分毒素伤害": "Absorbs some poison damage taken by the flower",
    "刀具": "Cutter",
    "提高花的体伤": "Increases the flower's body damage",
    "阴阳": "Yin Yang",
    "以有趣的方式改变花瓣旋转": "Alters the flower's petal rotation in interesting ways",
    "世界树": "Yggdrasil",
    "可惜它的力量在这里派不上用场": "Unfortunately, its powers are useless here",
    "大米": "Rice",
    "瞬间生成，但不太强": "Spawns instantly, but not very strong",
    "骨头": "Bone",
    "结实": "Sturdy",
    "丝兰": "Yucca",
    "只在防御姿态时治疗花朵": "Heals the flower, but only while in the defensive position",
    "玉米": "Corn",
    "生成很慢，但生命值很高": "Takes a long time to spawn, but has a lot of health",
    "让花瓣装填更慢但伤害更高": "Allows your petals to recharge slower but do more damage",
    "幼蚁": "Baby Ant",
    "弱小无防，但梦想很大。": "Weak and defenseless, but big dreams.",
    "工蚁": "Worker Ant",
    "脾气不好，大概是因为一直干活。": "It's temperamental, probably from working all the time.",
    "兵蚁": "Soldier Ant",
    "有翅膀，并且随时准备用上。": "It's got wings and it's ready to use them.",
    "蜜蜂": "Bee",
    "会蜇人。别碰。": "It stings. Don't touch it.",
    "瓢虫": "Ladybug",
    "可爱又无害。": "Cute and harmless.",
    "甲虫": "Beetle",
    "它饿了，花是它最爱的餐点。": "It's hungry and flowers are its favorite meal.",
    "巨型瓢虫": "Massive Ladybug",
    "大得多，但依然可爱。": "Much larger, but still cute.",
    "巨型甲虫": "Massive Beetle",
    "有人把它喂得太大了，下一个可能是你。": "Someone overfed this one, you might be next.",
    "可爱无害……只要你别惹它。": "Cute and harmless... if left unprovoked.",
    "黄蜂": "Hornet",
    "它们可不像小蜜蜂那么友好。": "These aren't quite as nice as the little bees.",
    "浑身是刺，也别碰。": "This one's prickly, don't touch it either.",
    "一块石头。没什么特别的。": "A rock. It doesn't do much.",
    "巨石": "Boulder",
    "更大的石头。同样没什么特别的。": "A bigger rock. It also doesn't do much.",
    "蜈蚣": "Centipede",
    "它只是在那儿干自己的事。": "It's just there doing its thing.",
    "邪恶蜈蚣": "Evil Centipede",
    "这个很喜欢花。": "This one loves flowers.",
    "沙漠蜈蚣": "Desert Centipede",
    "不喜欢你打断它的奔跑。": "It doesn't like it when you interrupt its run.",
    "沙暴": "Sandstorm",
    "相当难以预测。": "Quite unpredictable.",
    "蝎子": "Scorpion",
    "会蜇人，还带毒。": "This one stings, now with poison.",
    "蜘蛛": "Spider",
    "吓人。": "Spooky.",
    "蚁穴": "Ant Hole",
    "蚂蚁进进出出。无法解释。": "Ants go in, and come out. Can't explain that.",
    "蚁后": "Queen Ant",
    "如果它在追你，你一定惹大麻烦了。": "You must have done something really bad if she's chasing you.",
    "这个亮闪闪的……不知道意味着什么……": "This one is shiny... I wonder what it could mean...",
    "挖掘者": "Digger",
    "敌是友？你永远不会知道……": "Friend or foe? You'll never know...",
}

text = STATIC.read_text(encoding="utf-8")
# Collect Chinese name/desc in order before restore
names = re.findall(r'\.name = "([^"]*)"', text)
descs = re.findall(r'\.description = "([^"]*)"', text)
print("names", len(names), "descs", len(descs))

for zh, en in sorted(ZH2EN.items(), key=lambda x: -len(x[0])):
    text = text.replace(f'.name = "{zh}"', f'.name = "{en}"')
    text = text.replace(f'.description = "{zh}"', f'.description = "{en}"')
STATIC.write_text(text, encoding="utf-8")

# After restore, re-read English and pair with Chinese from ZH2EN
text2 = STATIC.read_text(encoding="utf-8")
en_names = re.findall(r'\.name = "([^"]*)"', text2)
en_descs = re.findall(r'\.description = "([^"]*)"', text2)

# PETAL then MOB blocks: first kNumPetals names are petals (50), rest mobs (24)
# Count by finding PETAL_DATA and MOB_DATA sections
petal_n = 50  # PetalID::kNumPetals
mob_n = 24

assert len(en_names) >= petal_n + mob_n, len(en_names)

def zh_of(en: str) -> str:
    for z, e in ZH2EN.items():
        if e == en:
            return z
    return en  # ??? etc.

petal_zh_names = [zh_of(en_names[i]) for i in range(petal_n)]
petal_zh_descs = [zh_of(en_descs[i]) for i in range(petal_n)]
mob_zh_names = [zh_of(en_names[petal_n + i]) for i in range(mob_n)]
mob_zh_descs = [zh_of(en_descs[petal_n + i]) for i in range(mob_n)]

out = ROOT / "Client" / "I18nData.inc"
lines = []
lines.append("// auto-generated by Scripts/zh_restore_and_gen_i18n.py\n")
lines.append("static char const *const PETAL_NAME_ZH[PetalID::kNumPetals] = {\n")
for s in petal_zh_names:
    lines.append(f'    u8"{s}",\n')
lines.append("};\n")
lines.append("static char const *const PETAL_DESC_ZH[PetalID::kNumPetals] = {\n")
for s in petal_zh_descs:
    lines.append(f'    u8"{s}",\n')
lines.append("};\n")
lines.append("static char const *const MOB_NAME_ZH[MobID::kNumMobs] = {\n")
for s in mob_zh_names:
    lines.append(f'    u8"{s}",\n')
lines.append("};\n")
lines.append("static char const *const MOB_DESC_ZH[MobID::kNumMobs] = {\n")
for s in mob_zh_descs:
    lines.append(f'    u8"{s}",\n')
lines.append("};\n")
out.write_text("".join(lines), encoding="utf-8")
print("wrote", out)
print("sample petal", en_names[1], "->", petal_zh_names[1])
print("sample mob", en_names[petal_n], "->", mob_zh_names[0])

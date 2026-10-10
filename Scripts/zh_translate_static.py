# -*- coding: utf-8 -*-
from pathlib import Path

p = Path(__file__).resolve().parents[1] / "Shared" / "StaticData.cc"
text = p.read_text(encoding="utf-8")

repl = {
    "None": "无",
    "How can you see this?": "你怎么能看到这个？",
    "Basic": "基本",
    "A nice petal, not too strong but not too weak": "不错的花瓣，不算强也不算弱",
    "Fast": "轻",
    "Weaker than most petals, but reloads very quickly": "比大多数花瓣弱，但冷却很快",
    "Heavy": "重",
    "Very resilient and deals more damage, but reloads very slowly": "非常坚韧且伤害更高，但冷却很慢",
    "Stinger": "刺",
    "It really hurts, but it's really fragile": "打人很疼，但非常脆弱",
    "Leaf": "叶子",
    "Gathers energy from the sun to passively heal your flower": "吸收阳光能量，被动治疗你的花",
    "Twin": "双",
    "Why stop at one? Why not TWO?!": "为什么只要一个？为什么不来两个？！",
    "Rose": "玫瑰",
    "Its healing properties are amazing. Not so good at combat though": "治疗能力惊人，但不太擅长战斗",
    "Iris": "鸢尾",
    "Very poisonous, but takes a while to do its work": "毒性很强，但生效需要一些时间",
    "Missile": "导弹",
    "You can actually shoot this one": "这个真的可以发射出去",
    "Dandelion": "蒲公英",
    "Its interesting properties prevent healing effects on affected units": "特殊效果会阻止受影响单位的治疗",
    "Bubble": "泡泡",
    "You can right click to pop it and propel your flower": "右键戳破可推动你的花",
    "Faster": "更快",
    "It's so light it makes your other petals spin faster": "非常轻，能让其他花瓣转得更快",
    "Rock": "岩石",
    "Even more durable, but slower to recharge": "更加耐久，但装填更慢",
    "Cactus": "仙人掌",
    "Not very strong, but somehow increases your maximum health": "不太强，但能提高最大生命值",
    "Web": "网",
    "It's really sticky": "非常黏",
    "Wing": "翅膀",
    "It comes and goes": "来来去去",
    "Peas": "豌豆",
    "4 in 1 deal": "四合一套餐",
    "Sand": "沙子",
    "It's coarse, rough, and gets everywhere": "又粗又糙，还会到处粘",
    "Pincer": "钳子",
    "Stuns and poisons targets for a short duration": "短暂眩晕并毒害目标",
    "Triplet": "三连",
    "How about THREE?!": "来三个怎么样？！",
    "Egg": "蛋",
    "Something interesting might pop out of this": "说不定会蹦出有趣的东西",
    "Deals its effects quicker than traditional irises": "比普通鸢尾生效更快",
    "Pollen": "花粉",
    "Asthmatics beware": "哮喘患者慎用",
    "4 in 1 deal, now with a secret ingredient: poison": "四合一套餐，还加了秘密调料：毒",
    "Extremely powerful rose, almost unheard of": "极其强大的玫瑰，几乎闻所未闻",
    "Stick": "树枝",
    "Harnesses the power of the wind": "驾驭风的力量",
    "Antennae": "触角",
    "Allows your flower to sense foes from farther away": "让你的花能更远地感知敌人",
    "Heaviest": "最重",
    "This thing is so heavy that nothing gets in the way": "重到没有任何东西能挡住它",
    "Third Eye": "第三只眼",
    "Allows your flower to extend petals further out": "让花瓣能伸得更远",
    "Observer": "观察者",
    "The one who sees all": "洞察一切之人",
    "Turns your flower poisonous. Enemies will take poison damage on contact": "让你的花带毒，敌人接触会中毒",
    "Salt": "盐",
    "Reflects some damage dealt to the flower. Does not stack with itself": "反弹一部分对花造成的伤害，不可叠加",
    "Something incredibly rare and useless": "极其稀有却毫无用处",
    "Square": "方块",
    "This shape... it looks familiar...": "这个形状……看起来很眼熟……",
    "Moon": "月亮",
    "Where did this come from?": "这是从哪来的？",
    "Lotus": "莲花",
    "Absorbs some poison damage taken by the flower": "吸收花受到的部分毒素伤害",
    "Cutter": "刀具",
    "Increases the flower's body damage": "提高花的体伤",
    "Yin Yang": "阴阳",
    "Alters the flower's petal rotation in interesting ways": "以有趣的方式改变花瓣旋转",
    "Yggdrasil": "世界树",
    "Unfortunately, its powers are useless here": "可惜它的力量在这里派不上用场",
    "Rice": "大米",
    "Spawns instantly, but not very strong": "瞬间生成，但不太强",
    "Bone": "骨头",
    "Sturdy": "结实",
    "Yucca": "丝兰",
    "Heals the flower, but only while in the defensive position": "只在防御姿态时治疗花朵",
    "Corn": "玉米",
    "Takes a long time to spawn, but has a lot of health": "生成很慢，但生命值很高",
    "Allows your petals to recharge slower but do more damage": "让花瓣装填更慢但伤害更高",
    "Baby Ant": "幼蚁",
    "Weak and defenseless, but big dreams.": "弱小无防，但梦想很大。",
    "Worker Ant": "工蚁",
    "It's temperamental, probably from working all the time.": "脾气不好，大概是因为一直干活。",
    "Soldier Ant": "兵蚁",
    "It's got wings and it's ready to use them.": "有翅膀，并且随时准备用上。",
    "Bee": "蜜蜂",
    "It stings. Don't touch it.": "会蜇人。别碰。",
    "Ladybug": "瓢虫",
    "Cute and harmless.": "可爱又无害。",
    "Beetle": "甲虫",
    "It's hungry and flowers are its favorite meal.": "它饿了，花是它最爱的餐点。",
    "Massive Ladybug": "巨型瓢虫",
    "Much larger, but still cute.": "大得多，但依然可爱。",
    "Massive Beetle": "巨型甲虫",
    "Someone overfed this one, you might be next.": "有人把它喂得太大了，下一个可能是你。",
    "Cute and harmless... if left unprovoked.": "可爱无害……只要你别惹它。",
    "Hornet": "黄蜂",
    "These aren't quite as nice as the little bees.": "它们可不像小蜜蜂那么友好。",
    "This one's prickly, don't touch it either.": "浑身是刺，也别碰。",
    "A rock. It doesn't do much.": "一块石头。没什么特别的。",
    "Boulder": "巨石",
    "A bigger rock. It also doesn't do much.": "更大的石头。同样没什么特别的。",
    "Centipede": "蜈蚣",
    "It's just there doing its thing.": "它只是在那儿干自己的事。",
    "Evil Centipede": "邪恶蜈蚣",
    "This one loves flowers.": "这个很喜欢花。",
    "Desert Centipede": "沙漠蜈蚣",
    "It doesn't like it when you interrupt its run.": "不喜欢你打断它的奔跑。",
    "Sandstorm": "沙暴",
    "Quite unpredictable.": "相当难以预测。",
    "Scorpion": "蝎子",
    "This one stings, now with poison.": "会蜇人，还带毒。",
    "Spider": "蜘蛛",
    "Spooky.": "吓人。",
    "Ant Hole": "蚁穴",
    "Ants go in, and come out. Can't explain that.": "蚂蚁进进出出。无法解释。",
    "Queen Ant": "蚁后",
    "You must have done something really bad if she's chasing you.": "如果它在追你，你一定惹大麻烦了。",
    "This one is shiny... I wonder what it could mean...": "这个亮闪闪的……不知道意味着什么……",
    "Digger": "挖掘者",
    "Friend or foe? You'll never know...": "敌是友？你永远不会知道……",
}

count = 0
for en, zh in sorted(repl.items(), key=lambda x: -len(x[0])):
    a = f'.name = "{en}"'
    b = f'.name = "{zh}"'
    n = text.count(a)
    if n:
        text = text.replace(a, b)
        count += n
    a = f'.description = "{en}"'
    b = f'.description = "{zh}"'
    n = text.count(a)
    if n:
        text = text.replace(a, b)
        count += n

p.write_text(text, encoding="utf-8")
print("replacements:", count)

import re

left = re.findall(r'\.(?:name|description) = "([^"]+)"', text)
eng = [s for s in left if re.search(r"[A-Za-z]{3,}", s)]
print("still latin-ish:", len(eng))
for s in eng:
    print(" ", s)

# -*- coding: utf-8 -*-
from pathlib import Path
p = Path(__file__).resolve().parents[1] / "Client" / "Ui" / "InGame" / "Tooltip.cc"
text = p.read_text(encoding="utf-8")
text = text.replace(
    '#include <Client/Ui/StaticText.hh>\n\n#include <Client/StaticData.hh>\n\n#include <Client/Game.hh>',
    '#include <Client/Ui/DynamicText.hh>\n#include <Client/Ui/StaticText.hh>\n\n#include <Client/StaticData.hh>\n#include <Client/I18n.hh>\n\n#include <Client/Game.hh>'
)
repls = [
    ('new Ui::StaticText(12, "Health:", { .fill = 0xff77ff77 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHealth); }, { .fill = 0xff77ff77 })'),
    ('new Ui::StaticText(12, "Damage:", { .fill = 0xffff7777 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamage); }, { .fill = 0xffff7777 })'),
    ('new Ui::StaticText(12, "Armor:", { .fill = 0xff777777 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kArmor); }, { .fill = 0xff777777 })'),
    ('new Ui::StaticText(12, "Damage Reflection:", { .fill = 0xff777777 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamageReflection); }, { .fill = 0xff777777 })'),
    ('new Ui::StaticText(12, "Heal:", { .fill = 0xffff96cb })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb })'),
    ('new Ui::StaticText(12, "Poison:", { .fill = 0xffce76db })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoison); }, { .fill = 0xffce76db })'),
    ('new Ui::StaticText(12, "Flower Health:", { .fill = 0xff77ff77 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kFlowerHealth); }, { .fill = 0xff77ff77 })'),
    ('new Ui::StaticText(12, "Body Damage:", { .fill = 0xffff7777 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBodyDamage); }, { .fill = 0xffff7777 })'),
    ('new Ui::StaticText(12, "Poison Armor:", { .fill = 0xffce76db })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoisonArmor); }, { .fill = 0xffce76db })'),
    ('new Ui::StaticText(12, "Rotation Speed:", { .fill = 0xffcde23b })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kRotationSpeed); }, { .fill = 0xffcde23b })'),
    ('new Ui::StaticText(12, "Contents:", { .fill = 0xffd2eb34 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kContents); }, { .fill = 0xffd2eb34 })'),
    ('new Ui::StaticText(12, "Spawns:", { .fill = 0xffd2eb34 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kSpawns); }, { .fill = 0xffd2eb34 })'),
    ('new Ui::StaticText(12, "Extra Vision:", { .fill = 0xffcde23b })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraVision); }, { .fill = 0xffcde23b })'),
    ('new Ui::StaticText(12, "Attack Range:", { .fill = 0xffcde23b })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kAttackRange); }, { .fill = 0xffcde23b })'),
    ('new Ui::StaticText(12, "Extra Damage:", { .fill = 0xffff7777 })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraDamage); }, { .fill = 0xffff7777 })'),
    ('new Ui::StaticText(12, "Extra Reload:", { .fill = 0xff7777ff })',
     'new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraReload); }, { .fill = 0xff7777ff })'),
]
for a,b in repls:
    if a not in text:
        print('MISSING', a[:40])
    text = text.replace(a,b)

old_contents = 'new Ui::StaticText(12, (petal_data.count > 1 ? format_number(petal_data.count) + "x " : "") + (MOB_DATA[attrs.spawns].name))'
new_contents = '''new Ui::DynamicText(12, [=](){
                return (petal_data.count > 1 ? format_number(petal_data.count) + "x " : "") + I18n::mob_name(attrs.spawns);
            })'''
text = text.replace(old_contents, new_contents)

old_spawns = 'new Ui::StaticText(12, format_number(attrs.spawn_count) + "x " + (MOB_DATA[attrs.spawns].name))'
new_spawns = '''new Ui::DynamicText(12, [=](){
                return format_number(attrs.spawn_count) + "x " + I18n::mob_name(attrs.spawns);
            })'''
text = text.replace(old_spawns, new_spawns)

text = text.replace(
    'new Ui::StaticText(20, PETAL_DATA[id].name, { .fill = 0xffffffff, .h_justify = Style::Left })',
    'new Ui::DynamicText(20, [id](){ return I18n::petal_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left })'
)
text = text.replace(
    'new Ui::StaticText(14, RARITY_NAMES[PETAL_DATA[id].rarity], { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left })',
    'new Ui::DynamicText(14, [id](){ return I18n::rarity_name(PETAL_DATA[id].rarity); }, { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left })'
)
text = text.replace(
    'new Ui::StaticText(12, PETAL_DATA[id].description, { .fill = 0xffffffff, .h_justify = Style::Left })',
    'new Ui::DynamicText(12, [id](){ return I18n::petal_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left })'
)
p.write_text(text, encoding='utf-8')
print('patched', p)

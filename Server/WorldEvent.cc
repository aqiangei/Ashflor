#include <Server/WorldEvent.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/TreasureSites.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <cmath>
#include <cstdint>
#include <string>

namespace {

constexpr float kTouchR = 70.f;
constexpr float kTouchR2 = kTouchR * kTouchR;
constexpr float kFleeApproachR = 130.f;
constexpr float kFleeApproachR2 = kFleeApproachR * kFleeApproachR;
constexpr float kFleeStep = 200.f;
constexpr float kSleepWakeR = 260.f;
constexpr float kSleepWakeR2 = kSleepWakeR * kSleepWakeR;
constexpr float kAmbushArmR = 420.f;
constexpr float kAmbushArmR2 = kAmbushArmR * kAmbushArmR;
constexpr float kInterestR = 1100.f;
constexpr float kInterestR2 = kInterestR * kInterestR;
constexpr float kBossSuppressR = 2200.f;
constexpr float kBossSuppressR2 = kBossSuppressR * kBossSuppressR;
constexpr uint8_t kMaxEventsPerMap = 12; // slots 0..11 in uint16_t claim mask
uint32_t const kTimedWindowTicks = 15 * TPS;
uint32_t const kFleeCdTicks = (uint32_t) (0.8f * TPS);
uint32_t const kSleepAlertTicks = (uint32_t) (1.5f * TPS);
uint32_t const kAmbushDelayTicks = (uint32_t) (1.5f * TPS);
constexpr float kSleepAggro = 520.f;

enum EventKind : uint8_t {
    kLoreSeed = 0,
    kTimedTouch = 1,
    kFleeBush = 2,
    kSleepMob = 3,
    kHeightOrder = 4,
    kAmbush = 5,  // warning marks → delay → spawn existing mobs
    kMirage = 6,  // fake chest dissipates on approach
};

struct EventDef {
    uint8_t map_id;
    uint8_t slot;
    EventKind kind;
    float ox, oy;
    float pad_off[3][2];
    MobID::T sleep_mob; // sleep / ambush mob
    char const *name_zh;
    char const *alert_zh;
    char const *awake_zh;
    char const *lore_zh;
    char const *lore_en;
};

static EventDef const kDefs[] = {
    // Garden
    { MapID::kGarden, 0, kLoreSeed, 3200.f, -800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "发光种子", "", "",
      "【种子】叶片与树枝记得拼成叶刃——花之心甲虫沉睡时，古心才会醒。",
      "[Seed] Leaf + Twig forge Leaf Blade — Ancient Heart wakes when the Flower Guardian sleeps." },
    { MapID::kGarden, 1, kLoreSeed, -2800.f, 2100.f, {{}, {}, {}}, MobID::kBabyAnt,
      "发光种子", "", "",
      "【种子】路边叶语会指路。推箱、听向导，残片自然会到手。",
      "[Seed] Wayside Koroks guide you. Push crates, listen — scraps find you." },
    { MapID::kGarden, 2, kTimedTouch, 800.f, -2800.f,
      {{ -90.f, -40.f }, { 90.f, -30.f }, { 10.f, 100.f }}, MobID::kBabyAnt,
      "限时花印", "", "",
      "【花印】限时内踏过三枚印记！", "[Marks] Touch all three marks in time!" },
    { MapID::kGarden, 3, kFleeBush, -1200.f, 3400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "可疑草丛", "", "",
      "【草丛】它终于不跑了……东北有推箱印记。",
      "[Bush] It stopped… NE for crate seals." },
    { MapID::kGarden, 4, kSleepMob, 2100.f, 2600.f, {{}, {}, {}}, MobID::kDarkLadybug,
      "睡觉的瓢虫", "警觉的瓢虫", "惊醒的瓢虫",
      "【瓢虫】它被吵醒了——先警觉，再追上来。",
      "[Ladybug] It woke — alert first, then it chases." },
    { MapID::kGarden, 5, kHeightOrder, -3400.f, -1500.f,
      {{ -110.f, 40.f }, { 0.f, -90.f }, { 110.f, 50.f }}, MobID::kBabyAnt,
      "蘑菇三角", "", "",
      "【蘑菇】从矮到高依次触碰。", "[Mushrooms] Touch short to tall." },
    { MapID::kGarden, 6, kAmbush, 1500.f, 3200.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kBeetle,
      "花丛伏击", "", "",
      "【伏击】地面纹路亮起……有东西要从土里出来！",
      "[Ambush] Ground marks flare… something rises!" },
    { MapID::kGarden, 7, kMirage, -2000.f, -3200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【幻影】箱子散作花粉……真正的宝箱在叶语附近。",
      "[Mirage] The chest dissolves into pollen… real chests sit by Koroks." },
    { MapID::kGarden, 8, kLoreSeed, 3800.f, 1400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "花粉风痕", "", "",
      "【花粉风】金色粉末曾掠过这里——花之心甲虫沉睡时，古心才会醒。",
      "[Pollen] Gold dust once swept here — Ancient Heart wakes when the Guardian sleeps." },
    { MapID::kGarden, 9, kTimedTouch, -3200.f, 800.f,
      {{ -80.f, -50.f }, { 85.f, -40.f }, { 5.f, 95.f }}, MobID::kBabyAnt,
      "奇怪的花圈", "", "",
      "【花圈】限时踏过三枚发光花印！", "[Flower Ring] Touch three glowing blooms in time!" },
    { MapID::kGarden, 10, kFleeBush, 600.f, -3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "抖动草丛", "", "",
      "【草丛】第三次它不再跑……叶语说东北有推箱。",
      "[Grass] After thrice it stays… Koroks say NE for crates." },
    { MapID::kGarden, 11, kLoreSeed, -3800.f, -800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "神秘种子·二", "", "",
      "【种子】V 组装叶刃：叶片×5+树枝×2。路边叶语会指路。",
      "[Seed] V-forge Leaf Blade: Leaf×5+Twig×2. Wayside Koroks guide you." },

    // Desert
    { MapID::kDesert, 0, kLoreSeed, 2100.f, 1400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "沙中残片", "", "",
      "【残片】沙粒与毒刺拼成沙刺。沙虫君主在更深处呼吸。",
      "[Scrap] Sand + Venom Barb forge Sand Spike." },
    { MapID::kDesert, 1, kTimedTouch, -900.f, -2600.f,
      {{ -80.f, 50.f }, { 85.f, 40.f }, { 5.f, -95.f }}, MobID::kBabyAnt,
      "限时沙印", "", "",
      "【沙印】沙暴前踏过三枚印记！", "[Sand Marks] Touch all three before the storm!" },
    { MapID::kDesert, 2, kFleeBush, 3600.f, -400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑沙堆", "", "",
      "【沙堆】沙停了……西南有沙印推箱。", "[Dune] Settled… SW for sand seals." },
    { MapID::kDesert, 3, kSleepMob, -2400.f, 800.f, {{}, {}, {}}, MobID::kScorpion,
      "睡觉的蝎子", "警觉的蝎子", "惊醒的蝎子",
      "【蝎子】沙里的蝎子醒了——钳爪先抬起，再扑上来。",
      "[Scorpion] It stirred — claws up, then it charges." },
    { MapID::kDesert, 4, kHeightOrder, 2800.f, 2200.f,
      {{ -100.f, 40.f }, { 0.f, -90.f }, { 100.f, 45.f }}, MobID::kBabyAnt,
      "仙人掌之秘", "", "",
      "【仙人掌】按花色从淡到艳依次触碰。", "[Cactus] Touch pale blossom to vivid." },
    { MapID::kDesert, 5, kAmbush, -3200.f, -1400.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kScorpion,
      "沙虫预警", "", "",
      "【预警】沙纹成环……蝎群要钻出来了！",
      "[Warning] Sand rings form… scorpions breach!" },
    { MapID::kDesert, 6, kMirage, 400.f, -3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】幻影散作热气……真宝箱在沙丘贩子附近。",
      "[Mirage] Heat haze… real chests near the Dune Dealer." },
    { MapID::kDesert, 7, kLoreSeed, -3800.f, 2200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "被沙掩埋的石碑", "", "",
      "【石碑】沙散开……沙虫君主沉睡时，带沙粒去组装沙刺。",
      "[Stele] Sand falls away… forge Sand Spike when the Monarch sleeps." },
    { MapID::kDesert, 8, kLoreSeed, 3400.f, -2000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "沙尘经过处", "", "",
      "【沙尘】风沙曾横扫此地——首版只留痕迹，不挡视线。",
      "[Dust] Sandstorms crossed here — traces only; vision stays clear." },
    { MapID::kDesert, 9, kLoreSeed, 1600.f, 3400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "流亡商休息点", "", "",
      "【帐篷】流亡商不另开货单——残片仍在既有商店与推箱礼。",
      "[Camp] Exiles sell no new stock — scraps stay in shop & crate rites." },
    { MapID::kDesert, 10, kFleeBush, -600.f, 3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "晃动沙丘", "", "",
      "【沙丘】它终于不动了……西南有沙印。",
      "[Dune] It settled… SW for sand seals." },
    { MapID::kDesert, 11, kTimedTouch, 4200.f, 600.f,
      {{ -75.f, 45.f }, { 80.f, 35.f }, { 0.f, -90.f }}, MobID::kBabyAnt,
      "外环沙印限时", "", "",
      "【沙印】再踏三枚！沙暴不等人。", "[Sand] Three more seals — the storm won't wait." },

    // Ocean
    { MapID::kOcean, 0, kLoreSeed, -1500.f, 2000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "潮间种子", "", "",
      "【种子】水珠与海盐拼成水泡弹。", "[Seed] Drop + Salt forge Water Bubble." },
    { MapID::kOcean, 1, kTimedTouch, 2800.f, -900.f,
      {{ -70.f, -60.f }, { 75.f, -50.f }, { 0.f, 90.f }}, MobID::kBabyAnt,
      "限时潮印", "", "",
      "【潮印】潮汐回来前踏过三枚光圈！", "[Tide Marks] Touch all three rings!" },
    { MapID::kOcean, 2, kFleeBush, -3200.f, -1200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑海草", "", "",
      "【海草】它不漂了……东南有光圈。", "[Kelp] Stopped… SE for light rings." },
    { MapID::kOcean, 3, kSleepMob, 1200.f, 2800.f, {{}, {}, {}}, MobID::kMoonJelly,
      "睡觉的水母", "警觉的水母", "惊醒的水母",
      "【水母】它鼓了一下——触须先张开，再漂过来。",
      "[Jelly] It pulsed — tentacles out, then drifts in." },
    { MapID::kOcean, 4, kHeightOrder, -2600.f, 600.f,
      {{ -95.f, 35.f }, { 5.f, -85.f }, { 95.f, 40.f }}, MobID::kBabyAnt,
      "发光贝壳", "", "",
      "【贝壳】从小到大依次触碰发光贝壳。", "[Shells] Touch small to large." },
    { MapID::kOcean, 5, kAmbush, 3400.f, 1600.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kMoonJelly,
      "潮环伏击", "", "",
      "【伏击】潮纹成环……水母群涌来！", "[Ambush] Tide rings… jellies surge!" },
    { MapID::kOcean, 6, kMirage, 600.f, -3400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】箱子化作泡沫……真宝箱在潜水员附近。",
      "[Mirage] Foam… real chests near Diver Mario." },
    { MapID::kOcean, 7, kLoreSeed, 4000.f, 400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "深海遗迹碎片", "", "",
      "【遗迹】石板刻着深渊巨口的纹——潮汐之核与深海珍珠。",
      "[Ruin] Carved Abyss Maw — Tide Core & Abyss Pearl." },
    { MapID::kOcean, 8, kLoreSeed, -800.f, -2800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "潮汐光纹", "", "",
      "【潮纹】蓝色波纹曾在此扩张——纯视觉，不改碰撞。",
      "[Tide Glow] Blue rings once expanded here — visual only." },
    { MapID::kOcean, 9, kFleeBush, 2200.f, 3200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "漂流木箱影", "", "",
      "【木箱】它靠岸了……东南有光圈推箱。",
      "[Driftwood] Ashore… SE for light-ring crates." },
    { MapID::kOcean, 10, kTimedTouch, -3600.f, 1400.f,
      {{ -70.f, -55.f }, { 75.f, -45.f }, { 5.f, 90.f }}, MobID::kBabyAnt,
      "漂浮气泡群", "", "",
      "【气泡】限时触碰三枚潮光！", "[Bubbles] Touch three tide-lights in time!" },
    { MapID::kOcean, 11, kLoreSeed, 800.f, 3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "寄居蟹足迹", "", "",
      "【蟹迹】寄居蟹会走向贝壳堆——跟脚印可到合法探索点。",
      "[Crab] Hermits walk to shell piles — prints lead to legal POIs." },

    // Polar
    { MapID::kPolar, 0, kLoreSeed, 1800.f, -2400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "霜晶籽", "", "",
      "【霜晶】冰晶与寒铁拼成冰锥。", "[Frost] Crystal + Cold Iron forge Ice Spike." },
    { MapID::kPolar, 1, kTimedTouch, -1400.f, 2000.f,
      {{ -85.f, -40.f }, { 90.f, -35.f }, { 5.f, 95.f }}, MobID::kBabyAnt,
      "限时霜印", "", "",
      "【霜印】暴雪前踏过三枚符文！", "[Frost Marks] Touch all three runes!" },
    { MapID::kPolar, 2, kFleeBush, 3200.f, 800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑雪堆", "", "",
      "【雪堆】雪停了……西北有符文推箱。", "[Drift] Settled… NW for rune crates." },
    { MapID::kPolar, 3, kSleepMob, -3000.f, -900.f, {{}, {}, {}}, MobID::kSnowMite,
      "睡觉的雪螨", "警觉的雪螨", "惊醒的雪螨",
      "【雪螨】霜壳裂开——它先抖雪，再冲过来。",
      "[Snow Mite] Frost cracked — shakes snow, then charges." },
    { MapID::kPolar, 4, kHeightOrder, 2400.f, 2400.f,
      {{ -90.f, 40.f }, { 0.f, -95.f }, { 95.f, 35.f }}, MobID::kBabyAnt,
      "冰晶顺序", "", "",
      "【冰晶】按闪光从弱到强依次触碰。", "[Ice] Touch dim glow to bright." },
    { MapID::kPolar, 5, kAmbush, -800.f, -3200.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kIceSpider,
      "雪堆伏击", "", "",
      "【伏击】雪圈震动……冰蛛钻出！", "[Ambush] Snow quakes… ice spiders!" },
    { MapID::kPolar, 6, kMirage, 3600.f, -1200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】幻影化作雪粉……真宝箱在霜铠残魂附近。",
      "[Mirage] Snowdust… real chests near Frost Remnant." },
    { MapID::kPolar, 7, kLoreSeed, -3600.f, 1600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "沉睡君王的寒光", "", "",
      "【寒光】冰晶映出冬之君——永冻之心与暴雪之眼。",
      "[Chillglow] Ice mirrors Winter Sovereign — Permafrost Heart & Blizzard Eye." },
    { MapID::kPolar, 8, kLoreSeed, 1000.f, 3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "暴风雪前兆", "", "",
      "【前兆】稀疏雪粒曾变密——首版不挡战斗。",
      "[Omen] Sparse flakes thickened once — combat stays clear." },
    { MapID::kPolar, 9, kFleeBush, -2000.f, 3200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "被雪覆盖的箱子影", "", "",
      "【雪箱】积雪散开……真宝箱走既有 Treasure。",
      "[Snow Chest] Snow clears… real loot uses Treasure rules." },
    { MapID::kPolar, 10, kLoreSeed, 2800.f, -3200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "雪地脚印", "", "",
      "【脚印】短暂足迹指向附近符文与霜铠——非永久箭头。",
      "[Prints] Brief tracks point to runes & Frost Remnant — no permanent arrow." },
    { MapID::kPolar, 11, kLoreSeed, -800.f, -3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "冰湖裂纹痕", "", "",
      "【裂纹】冰面曾扩散裂纹——仅视觉。",
      "[Crack] Ice once spidered — visual only." },

    // Jungle
    { MapID::kJungle, 0, kLoreSeed, -2200.f, -1800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "藤种", "", "",
      "【藤种】藤蔓与树脂拼成荆藤。", "[Vine] Vine + Resin forge Thorn Vine." },
    { MapID::kJungle, 1, kTimedTouch, 1600.f, 2400.f,
      {{ -95.f, 30.f }, { 80.f, 45.f }, { 10.f, -100.f }}, MobID::kBabyAnt,
      "限时藤印", "", "",
      "【藤印】藤蔓合拢前踏过三枚印记！", "[Vine Marks] Touch all three!" },
    { MapID::kJungle, 2, kFleeBush, -3800.f, 900.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑灌木", "", "",
      "【灌木】它停了……北向有藤匣。", "[Shrub] Stopped… north for vine crates." },
    { MapID::kJungle, 3, kHeightOrder, 900.f, -3200.f,
      {{ -100.f, 50.f }, { 0.f, -85.f }, { 105.f, 40.f }}, MobID::kBabyAnt,
      "树脂菇阵", "", "",
      "【树脂菇】从矮到高依次触碰。", "[Resin Caps] Short to tall." },
    { MapID::kJungle, 4, kSleepMob, 2800.f, -600.f, {{}, {}, {}}, MobID::kPoisonToad,
      "睡觉的毒蟾", "警觉的毒蟾", "惊醒的毒蟾",
      "【毒蟾】它鼓起气囊——先瞪你一眼，再跳过来。",
      "[Toad] Throat swells — a stare, then it leaps." },
    { MapID::kJungle, 5, kAmbush, -1400.f, 2800.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kCarnivorousPlant,
      "咬人花丛", "", "",
      "【伏击】花丛骤然张开……食人花！", "[Ambush] Flowers snap open… carnivores!" },
    { MapID::kJungle, 6, kMirage, 3600.f, 1200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】箱子化作孢子……真宝箱在森境贤者附近。",
      "[Mirage] Spores… real chests near Grove Sage." },
    { MapID::kJungle, 7, kLoreSeed, -3200.f, -2400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "古树低语", "", "",
      "【古树】树叶发光……远古树人——远古树芯与树脂。",
      "[Elder] Leaves glow… Ancient Treant — Heartwood & Resin." },
    { MapID::kJungle, 8, kLoreSeed, 2000.f, 3400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "萤火虫群迹", "", "",
      "【萤火】绿光曾绕树旋转十秒——纯视觉。",
      "[Fireflies] Green sparks once circled trees — visual only." },
    { MapID::kJungle, 9, kFleeBush, 400.f, 3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "伪装蘑菇", "", "",
      "【蘑菇】抖动停了……北向有藤匣。",
      "[Shroom] Quake stops… north for vine crates." },
    { MapID::kJungle, 10, kTimedTouch, -2800.f, 2000.f,
      {{ -90.f, 40.f }, { 85.f, 35.f }, { 5.f, -95.f }}, MobID::kBabyAnt,
      "藤蔓机关·二", "", "",
      "【藤蔓】再按序踏三枚节点！", "[Vines] Touch three nodes in order again!" },
    { MapID::kJungle, 11, kLoreSeed, 3200.f, -2800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "引路猴足迹", "", "",
      "【猴迹】小猴子会停三次回头——终点必是合法探索点。",
      "[Monkey] It stops thrice and looks back — ends at a legal POI." },

    // Ant Hell
    { MapID::kAntHell, 0, kLoreSeed, 2400.f, -1600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "巢芯残片", "", "",
      "【巢芯】蚁壳与触角拼成蚁牙。", "[Hive] Shell + Antenna forge Ant Fang." },
    { MapID::kAntHell, 1, kTimedTouch, -2000.f, 1800.f,
      {{ -70.f, 55.f }, { 75.f, 40.f }, { 0.f, -90.f }}, MobID::kBabyAnt,
      "限时巢印", "", "",
      "【巢印】信息素散尽前踏过三枚巢印！", "[Nest Marks] Touch all three!" },
    { MapID::kAntHell, 2, kFleeBush, 1100.f, 3100.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑蚁丘", "", "",
      "【蚁丘】土停了……东北有巢印推箱。", "[Mound] Settled… NE for nest crates." },
    { MapID::kAntHell, 3, kSleepMob, -900.f, -2800.f, {{}, {}, {}}, MobID::kSoldierAnt,
      "睡觉的兵蚁", "警觉的兵蚁", "惊醒的兵蚁",
      "【兵蚁】触角竖起——它先警戒，再扑向入侵者。",
      "[Soldier] Antennae up — alert, then charge." },
    { MapID::kAntHell, 4, kHeightOrder, 3000.f, 900.f,
      {{ -85.f, 45.f }, { 0.f, -90.f }, { 90.f, 40.f }}, MobID::kBabyAnt,
      "菌圃节点", "", "",
      "【菌圃】按亮起顺序从暗到亮触碰。", "[Fungus] Touch dim to bright." },
    { MapID::kAntHell, 5, kAmbush, -3400.f, 400.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kSoldierAnt,
      "虫卵震动", "", "",
      "【伏击】虫卵裂开……兵蚁涌出！", "[Ambush] Eggs crack… soldiers pour!" },
    { MapID::kAntHell, 6, kMirage, 800.f, -3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】幻影散作土屑……真宝箱在蚁国向导附近。",
      "[Mirage] Dust… real chests near Ant Guide." },
    { MapID::kAntHell, 7, kLoreSeed, -3600.f, -1600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "隧道回声", "", "",
      "【回声】洞口声波……蚁后——群巢之心与蚁后信息素。",
      "[Echo] Cave ripples… Queen Ant — Hive Heart & Queen Pheromone." },
    { MapID::kAntHell, 8, kLoreSeed, 3600.f, 2000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "地道落石痕", "", "",
      "【落石】灰尘与碎石曾洒落——不改通路。",
      "[Scree] Dust and pebbles fell — path unchanged." },
    { MapID::kAntHell, 9, kLoreSeed, -1400.f, 3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "运输队经过处", "", "",
      "【运输】装饰工蚁曾搬运大圆物——不可拾取。",
      "[Caravan] Decor workers hauled a globe — not lootable." },
    { MapID::kAntHell, 10, kFleeBush, 2800.f, -2800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "晃动虫卵影", "", "",
      "【虫卵】晃动停了……东北有巢印。",
      "[Egg] Quake stops… NE for nest seals." },
    { MapID::kAntHell, 11, kLoreSeed, 400.f, -2000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "蚁群迁徙痕", "", "",
      "【迁徙】装饰蚁列曾路过——领队才有事件态。",
      "[Migration] Decor ant lines once passed — only the lead is stateful." },

    // Volcano
    { MapID::kVolcano, 0, kLoreSeed, -1900.f, 2200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "灼热余烬", "", "",
      "【余烬】火晶与硫磺拼成火球。", "[Ember] Fire Crystal + Sulfur forge Fireball." },
    { MapID::kVolcano, 1, kTimedTouch, 2200.f, -1800.f,
      {{ -80.f, -50.f }, { 85.f, -40.f }, { 5.f, 100.f }}, MobID::kBabyAnt,
      "限时灼印", "", "",
      "【灼印】熔岩涌出前踏过三枚印记！", "[Sear Marks] Touch all three!" },
    { MapID::kVolcano, 2, kFleeBush, -3500.f, -600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑灰堆", "", "",
      "【灰堆】灰停了……东向有灼印推箱。", "[Ash] Settled… east for sear crates." },
    { MapID::kVolcano, 3, kSleepMob, 800.f, 3000.f, {{}, {}, {}}, MobID::kLavaBeetle,
      "睡觉的熔岩甲虫", "警觉的熔岩甲虫", "惊醒的熔岩甲虫",
      "【熔岩甲虫】壳缝冒火星——它先抖灰，再冲上来。",
      "[Lava Beetle] Sparks — shakes ash, then charges." },
    { MapID::kVolcano, 4, kHeightOrder, -2800.f, 1400.f,
      {{ -90.f, 40.f }, { 5.f, -90.f }, { 95.f, 45.f }}, MobID::kBabyAnt,
      "硫磺晶序", "", "",
      "【硫晶】按灼热程度从弱到强触碰。", "[Sulfur] Touch cool crystal to hot." },
    { MapID::kVolcano, 5, kAmbush, 3200.f, 600.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kFireSpider,
      "岩浆伏击", "", "",
      "【伏击】岩环开裂……火蛛爬出！", "[Ambush] Rock rings crack… fire spiders!" },
    { MapID::kVolcano, 6, kMirage, -600.f, -3400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】箱子化作余烬……真宝箱在熔火囚人附近。",
      "[Mirage] Embers… real chests near Ash Prisoner." },
    { MapID::kVolcano, 7, kLoreSeed, -3200.f, 2800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "君王余烬", "", "",
      "【余烬碑】熔火霸主——地脉熔核与凤凰余烬。",
      "[Ember Stele] Molten Overlord — Leyline Core & Phoenix Ember." },
    { MapID::kVolcano, 8, kLoreSeed, 3600.f, -2400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "灰烬飘落处", "", "",
      "【灰烬】缓慢飘落的灰曾掠过——不挡主战斗区。",
      "[Ashfall] Slow ash once drifted — combat stays clear." },
    { MapID::kVolcano, 9, kLoreSeed, 1400.f, -3600.f, {{}, {}, {}}, MobID::kBabyAnt,
      "喷发预警痕", "", "",
      "【预警】红色脉冲圈曾亮起……随后是喷发粒子。",
      "[Erupt Omen] Red pulse rings flared… then burst particles." },
    { MapID::kVolcano, 10, kFleeBush, -2400.f, -2000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "熔岩裂缝", "", "",
      "【裂缝】微光停了……东向有灼印。",
      "[Fissure] Glow fades… east for sear seals." },
    { MapID::kVolcano, 11, kLoreSeed, 400.f, 3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "岩浆气泡痕", "", "",
      "【气泡】橙色气泡曾胀破——纯视觉无伤害。",
      "[Magma Bubble] Orange bubbles once burst — visual, no damage." },

    // Royal
    { MapID::kRoyalCapital, 0, kLoreSeed, 2600.f, 900.f, {{}, {}, {}}, MobID::kBabyAnt,
      "锈蚀圣印", "", "",
      "【圣印】精钢与齿轮拼成精钢剑。", "[Seal] Steel + Gear forge Steel Sword." },
    { MapID::kRoyalCapital, 1, kTimedTouch, -2800.f, 1600.f,
      {{ -90.f, 35.f }, { 80.f, 50.f }, { 5.f, -95.f }}, MobID::kBabyAnt,
      "限时王印", "", "",
      "【王印】号角响起前踏过三枚王印！", "[Royal Marks] Touch all three!" },
    { MapID::kRoyalCapital, 2, kFleeBush, 1500.f, -3000.f, {{}, {}, {}}, MobID::kBabyAnt,
      "逃跑旗帜", "", "",
      "【旗帜】布停了……西向有王印推箱。", "[Banner] Settled… west for royal crates." },
    { MapID::kRoyalCapital, 3, kSleepMob, -1600.f, -2200.f, {{}, {}, {}}, MobID::kRustMite,
      "睡觉的锈螨", "警觉的锈螨", "惊醒的锈螨",
      "【锈螨】锈皮裂响——它先亮齿，再扑向行人。",
      "[Rust Mite] Rust cracks — fangs, then leaps." },
    { MapID::kRoyalCapital, 4, kHeightOrder, 3200.f, -800.f,
      {{ -85.f, 40.f }, { 0.f, -90.f }, { 90.f, 45.f }}, MobID::kBabyAnt,
      "纹章石序", "", "",
      "【纹章】按磨损从旧到新依次触碰。", "[Crests] Touch worn to polished." },
    { MapID::kRoyalCapital, 5, kAmbush, -800.f, 3200.f,
      {{ 120.f, 0.f }, { -60.f, 104.f }, { -60.f, -104.f }}, MobID::kWanderingGuard,
      "巡逻伏击", "", "",
      "【伏击】锈甲脚步逼近……卫兵！", "[Ambush] Rusted boots… guards!" },
    { MapID::kRoyalCapital, 6, kMirage, -3600.f, -400.f, {{}, {}, {}}, MobID::kBabyAnt,
      "海市蜃楼箱", "", "",
      "【蜃楼】箱子化作锈屑……真宝箱在锈铠掮客附近。",
      "[Mirage] Rust flakes… real chests near Rust Broker." },
    { MapID::kRoyalCapital, 7, kLoreSeed, 1800.f, 3200.f, {{}, {}, {}}, MobID::kBabyAnt,
      "皇家石碑", "", "",
      "【石碑】陨落之王——王权核心与皇家圣印。流亡商卖残片。",
      "[Stele] Fallen King — Sovereign Core & Royal Seal. Exiles sell scrap." },
    { MapID::kRoyalCapital, 8, kLoreSeed, -3200.f, -2800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "君王回响", "", "",
      "【回响】王冠轮廓一闪……不召唤 Boss，不发君王核。",
      "[Echo] A crown silhouette flashes — no Boss, no Sovereign Core drop." },
    { MapID::kRoyalCapital, 9, kFleeBush, 3800.f, 1800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "破损旗帜影", "", "",
      "【旗帜】飘动停了……西向有王印。",
      "[Banner] Flutter stops… west for royal seals." },
    { MapID::kRoyalCapital, 10, kLoreSeed, -400.f, -3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "旧日游行痕", "", "",
      "【游行】装饰行列曾走过王道——无独立经济。",
      "[Parade] Decor columns once marched — no separate economy." },
    { MapID::kRoyalCapital, 11, kLoreSeed, 800.f, 3800.f, {{}, {}, {}}, MobID::kBabyAnt,
      "失落宝箱指引", "", "",
      "【废墟】角落真宝箱走既有 Treasure，不另开物品。",
      "[Ruin] Corner chests use Treasure rules — no extra loot table." },
};

constexpr uint8_t kDefCount = (uint8_t) (sizeof(kDefs) / sizeof(kDefs[0]));

enum SleepPhase : uint8_t { kAsleep = 0, kAlert = 1, kAwake = 2 };

struct EventRuntime {
    EntityID prop = NULL_ENTITY;
    EntityID pads[3] = { NULL_ENTITY, NULL_ENTITY, NULL_ENTITY };
    float x = 0, y = 0;
    float pad_x[3] = {}, pad_y[3] = {};
    uint8_t def_index = 0;
    uint8_t alive = 0;
    uint8_t kind = 0;
    uint8_t flee_count = 0;
    uint32_t flee_cd = 0;
    uint8_t sleep_phase = kAsleep;
    uint32_t alert_ticks = 0;
    uint8_t ambush_phase = 0; // 0 idle 1 armed 2 fired
    uint32_t ambush_ticks = 0;
};

struct MapRuntime {
    EventRuntime events[kMaxEventsPerMap] = {};
    uint8_t count = 0;
    uint8_t spawned = 0;
};

static MapRuntime g_maps[MapID::kNumMaps];

static uint8_t near_boss(uint8_t mid, float x, float y) {
    float bx = 0, by = 0;
    uint8_t mob = 0;
    if (!Map::boss_anchor(mid, bx, by, mob)) return 0;
    float dx = x - bx, dy = y - by;
    return (dx * dx + dy * dy) <= kBossSuppressR2;
}

static void place_xy(uint8_t map_id, float &x, float &y) {
    Map::resolve_circle_terrain(map_id, x, y, 40.f);
    x = fclamp(x, 200.f, (float) ARENA_WIDTH - 200.f);
    y = fclamp(y, 200.f, (float) ARENA_HEIGHT - 200.f);
}

static Entity &spawn_pad(Simulation *sim, float x, float y, char const *name, uint8_t named) {
    return alloc_mob(sim, MobID::kYahahaPad, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        BitMath::set(ent.flags, EntityFlags::kHasCulling);
        BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
        ent.score_reward = 0;
        ent.damage = 0;
        ent.mass = 10000.f;
        ent.friction = 1.f;
        ent.health = ent.max_health = 500000.f;
        if (named && name != nullptr && name[0] != '\0') {
            ent.set_nametag_visible(1);
            ent.set_name(name);
        }
    });
}

static Entity &spawn_sleep_mob(Simulation *sim, MobID::T mob_id, float x, float y,
                               char const *name) {
    return alloc_mob(sim, mob_id, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kHasCulling);
        ent.detection_radius = 0.f;
        ent.acceleration.set(0, 0);
        ent.velocity = Vector(0, 0);
        if (name != nullptr && name[0] != '\0') {
            ent.set_nametag_visible(1);
            ent.set_name(name);
        }
    });
}

static Entity &spawn_mirage_chest(Simulation *sim, float x, float y, char const *name) {
    return alloc_mob(sim, MobID::kLootChest, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        BitMath::set(ent.flags, EntityFlags::kHasCulling);
        BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
        ent.score_reward = 0;
        ent.damage = 0;
        ent.mass = 10000.f;
        ent.health = ent.max_health = 500000.f;
        ent.set_nametag_visible(1);
        if (name != nullptr && name[0] != '\0')
            ent.set_name(name);
    });
}

static void spawn_ambush_mobs(Simulation *sim, uint8_t mid, EventRuntime &ev, MobID::T mob) {
    for (uint8_t i = 0; i < 2; ++i) {
        float ax = ev.x + (i ? 80.f : -80.f);
        float ay = ev.y + (i ? -60.f : 60.f);
        place_xy(mid, ax, ay);
        alloc_mob(sim, mob, ax, ay, NULL_ENTITY, [&](Entity &ent) {
            BitMath::set(ent.flags, EntityFlags::kHasCulling);
            BitMath::set(ent.flags, EntityFlags::kSpawnedFromZone);
        });
    }
}

static void grant_explore(Client *client, uint8_t mid, uint32_t coins, uint8_t mat_n,
                          char const *toast) {
    if (client == nullptr) return;
    char const *title = toast != nullptr ? toast : "探索";
    if (coins > 0) {
        client_award_coins(client, coins);
        client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins, title);
    }
    // Map-themed scrap only (primary / secondary / tertiary biome pool).
    uint8_t given = 0;
    std::string mat_line;
    for (uint8_t i = 0; i < mat_n; ++i) {
        PetalID::T mat = TreasureSites::pick_explore_mat(mid);
        if (mat == PetalID::kNone) continue;
        if (!client_award_petal(client, mat)) continue;
        ++given;
        client_send_loot_toast(client, mat, 1, title);
        char const *pname = (mat < PetalID::kNumPetals && PETAL_DATA[mat].name)
            ? PETAL_DATA[mat].name : "残片";
        if (!mat_line.empty()) mat_line += "、";
        mat_line += pname;
    }
    if (coins || given) {
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            std::string{"【探索奖励】钱币 +"} + std::to_string(coins)
            + (given ? "，" + mat_line : ""));
    }
}

static char const *height_pad_name(uint8_t mid, uint8_t p) {
    static char const *kMush[3] = { "矮菇", "中菇", "高菇" };
    static char const *kCact[3] = { "淡花仙人掌", "暖花仙人掌", "艳花仙人掌" };
    static char const *kShell[3] = { "小贝壳", "中贝壳", "大贝壳" };
    static char const *kIce[3] = { "淡冰晶", "寒冰晶", "烈冰晶" };
    static char const *kResin[3] = { "矮树脂菇", "中树脂菇", "高树脂菇" };
    static char const *kFung[3] = { "暗菌", "亮菌", "耀菌" };
    static char const *kSulf[3] = { "温硫晶", "热硫晶", "灼硫晶" };
    static char const *kCrest[3] = { "旧纹章", "褪色纹章", "新纹章" };
    switch (mid) {
    case MapID::kDesert: return kCact[p];
    case MapID::kOcean: return kShell[p];
    case MapID::kPolar: return kIce[p];
    case MapID::kJungle: return kResin[p];
    case MapID::kAntHell: return kFung[p];
    case MapID::kVolcano: return kSulf[p];
    case MapID::kRoyalCapital: return kCrest[p];
    default: return kMush[p];
    }
}

} // namespace

void WorldEvent::clear_client(Client *client) {
    if (client == nullptr) return;
    client->timed_touch_mask = 0;
    client->timed_touch_event = 255;
    client->timed_touch_ticks = 0;
    client->height_order_step = 0;
    client->height_order_event = 255;
}

void WorldEvent::spawn_for_map(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapRuntime &mr = g_maps[mid];
    if (mr.spawned) return;
    Simulation *sim = &game->simulation;
    mr.count = 0;
    for (uint8_t i = 0; i < kDefCount && mr.count < kMaxEventsPerMap; ++i) {
        if (kDefs[i].map_id != mid) continue;
        EventRuntime &ev = mr.events[mr.count];
        float x = MAP_SPAWN_X + kDefs[i].ox;
        float y = MAP_SPAWN_Y + kDefs[i].oy;
        place_xy(mid, x, y);
        ev = EventRuntime{};
        ev.x = x;
        ev.y = y;
        ev.def_index = i;
        ev.kind = kDefs[i].kind;
        if (kDefs[i].kind == kTimedTouch || kDefs[i].kind == kHeightOrder
            || kDefs[i].kind == kAmbush) {
            for (uint8_t p = 0; p < 3; ++p) {
                float px = x + kDefs[i].pad_off[p][0];
                float py = y + kDefs[i].pad_off[p][1];
                place_xy(mid, px, py);
                ev.pad_x[p] = px;
                ev.pad_y[p] = py;
                char const *nm = nullptr;
                if (kDefs[i].kind == kHeightOrder)
                    nm = height_pad_name(mid, p);
                ev.pads[p] = spawn_pad(sim, px, py, nm, nm != nullptr).id;
            }
            float mx = ev.pad_x[0], my = ev.pad_y[0] - 50.f;
            place_xy(mid, mx, my);
            ev.prop = spawn_pad(sim, mx, my, kDefs[i].name_zh, 1).id;
        } else if (kDefs[i].kind == kSleepMob) {
            ev.prop = spawn_sleep_mob(sim, kDefs[i].sleep_mob, x, y, kDefs[i].name_zh).id;
        } else if (kDefs[i].kind == kMirage) {
            ev.prop = spawn_mirage_chest(sim, x, y, kDefs[i].name_zh).id;
        } else {
            ev.prop = spawn_pad(sim, x, y, kDefs[i].name_zh, 1).id;
        }
        ev.alive = 1;
        ++mr.count;
    }
    mr.spawned = 1;
}

void WorldEvent::tick(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapRuntime &mr = g_maps[mid];
    if (!mr.spawned || mr.count == 0) return;
    Simulation *sim = &game->simulation;

    for (uint8_t ei = 0; ei < mr.count; ++ei) {
        EventRuntime &ev = mr.events[ei];
        if (ev.flee_cd > 0) --ev.flee_cd;

        if (ev.kind == kSleepMob && sim->ent_alive(ev.prop)) {
            EventDef const &sdef = kDefs[ev.def_index];
            Entity &bug = sim->get_ent(ev.prop);
            if (ev.sleep_phase == kAsleep || ev.sleep_phase == kAlert) {
                bug.acceleration.set(0, 0);
                bug.velocity = Vector(0, 0);
                bug.detection_radius = 0.f;
            }
            if (ev.sleep_phase == kAlert) {
                if (ev.alert_ticks > 0) --ev.alert_ticks;
                if (ev.alert_ticks == 0) {
                    ev.sleep_phase = kAwake;
                    bug.detection_radius = kSleepAggro;
                    if (sdef.awake_zh != nullptr && sdef.awake_zh[0] != '\0')
                        bug.set_name(sdef.awake_zh);
                }
            }
        }

        if (ev.kind == kAmbush && ev.ambush_phase == 1) {
            if (ev.ambush_ticks > 0) --ev.ambush_ticks;
            if (ev.ambush_ticks == 0) {
                EventDef const &adef = kDefs[ev.def_index];
                spawn_ambush_mobs(sim, mid, ev, adef.sleep_mob);
                ev.ambush_phase = 2;
            }
        }
    }

    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float px = player.get_x(), py = player.get_y();
        uint8_t boss_zone = near_boss(mid, px, py);

        if (client->timed_touch_ticks > 0) {
            --client->timed_touch_ticks;
            if (client->timed_touch_ticks == 0) {
                client->timed_touch_mask = 0;
                client->timed_touch_event = 255;
                Chat::send_to(client, ChatChannel::kChatSystem, "",
                    "【印记】时间到了……再试一次吧。");
            }
        }

        for (uint8_t ei = 0; ei < mr.count; ++ei) {
            EventRuntime &ev = mr.events[ei];
            if (!ev.alive) continue;
            EventDef const &def = kDefs[ev.def_index];
            uint16_t bit = (uint16_t) (1u << def.slot);
            if ((client->world_event_claimed[mid] & bit) && def.kind != kSleepMob
                && def.kind != kAmbush)
                continue;

            float edx = px - ev.x, edy = py - ev.y;
            float ed2 = edx * edx + edy * edy;
            if (ed2 > kInterestR2) continue;

            // Boss 战圈内不新开探索干扰（进行中的限时/睡觉仍可收尾）
            if (boss_zone && def.kind != kSleepMob
                && !(def.kind == kTimedTouch && client->timed_touch_ticks > 0
                     && client->timed_touch_event == ei)
                && !(def.kind == kHeightOrder && client->height_order_event == ei))
                continue;

            if (def.kind == kLoreSeed) {
                if (client->world_event_claimed[mid] & bit) continue;
                if (ed2 > kTouchR2) continue;
                client->world_event_claimed[mid] |= bit;
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                grant_explore(client, mid, 100, 2, def.name_zh);
                continue;
            }

            if (def.kind == kFleeBush) {
                if (client->world_event_claimed[mid] & bit) continue;
                if (!sim->ent_alive(ev.prop)) continue;
                Entity &bush = sim->get_ent(ev.prop);
                float bx = bush.get_x(), by = bush.get_y();
                float dx = px - bx, dy = py - by;
                float d2 = dx * dx + dy * dy;
                if (ev.flee_count < 3) {
                    if (d2 > kFleeApproachR2 || ev.flee_cd > 0) continue;
                    float len = std::sqrt(d2);
                    if (len < 1.f) len = 1.f;
                    float nx = bx - dx / len * kFleeStep;
                    float ny = by - dy / len * kFleeStep;
                    place_xy(mid, nx, ny);
                    bush.set_x(nx); bush.set_y(ny);
                    bush.velocity = Vector(0, 0);
                    ev.x = nx; ev.y = ny;
                    ++ev.flee_count;
                    ev.flee_cd = kFleeCdTicks;
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"【"} + def.name_zh + "】逃开了（"
                        + std::to_string(ev.flee_count) + "/3）");
                    continue;
                }
                if (d2 > kTouchR2) continue;
                client->world_event_claimed[mid] |= bit;
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                grant_explore(client, mid, 150, 3, def.name_zh);
                continue;
            }

            if (def.kind == kSleepMob) {
                if (!sim->ent_alive(ev.prop)) continue;
                Entity &bug = sim->get_ent(ev.prop);
                float bx = bug.get_x(), by = bug.get_y();
                float dx = px - bx, dy = py - by;
                float d2 = dx * dx + dy * dy;
                if (ev.sleep_phase == kAsleep && d2 <= kSleepWakeR2 && !boss_zone) {
                    ev.sleep_phase = kAlert;
                    ev.alert_ticks = kSleepAlertTicks;
                    if (def.alert_zh && def.alert_zh[0]) bug.set_name(def.alert_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                    if (!(client->world_event_claimed[mid] & bit)) {
                        client->world_event_claimed[mid] |= bit;
                        grant_explore(client, mid, 120, 2, def.name_zh);
                    }
                } else if (ev.sleep_phase == kAwake && !sim->ent_alive(bug.target)
                           && d2 <= kSleepWakeR2) {
                    bug.target = player.id;
                    bug.detection_radius = kSleepAggro;
                    bug.ai_state = AIState::kBasicAggro;
                }
                continue;
            }

            if (def.kind == kAmbush) {
                if (ev.ambush_phase == 0 && ed2 <= kAmbushArmR2 && !boss_zone) {
                    ev.ambush_phase = 1;
                    ev.ambush_ticks = kAmbushDelayTicks;
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                }
                if (ev.ambush_phase >= 1 && !(client->world_event_claimed[mid] & bit)
                    && ed2 <= kAmbushArmR2) {
                    client->world_event_claimed[mid] |= bit;
                    grant_explore(client, mid, 150, 2, def.name_zh);
                }
                continue;
            }

            if (def.kind == kMirage) {
                if (client->world_event_claimed[mid] & bit) continue;
                if (!sim->ent_alive(ev.prop)) {
                    client->world_event_claimed[mid] |= bit;
                    continue;
                }
                if (ed2 > kTouchR2) continue;
                sim->request_delete(ev.prop);
                ev.prop = NULL_ENTITY;
                client->world_event_claimed[mid] |= bit;
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                grant_explore(client, mid, 80, 1, def.name_zh);
                continue;
            }

            if (def.kind == kHeightOrder) {
                if (client->world_event_claimed[mid] & bit) continue;
                if (client->height_order_event != 255
                    && client->height_order_event != ei) continue;
                for (uint8_t p = 0; p < 3; ++p) {
                    float pdx = px - ev.pad_x[p], pdy = py - ev.pad_y[p];
                    if (pdx * pdx + pdy * pdy > kTouchR2) continue;
                    if (client->height_order_event == 255) {
                        client->height_order_event = ei;
                        client->height_order_step = 0;
                        Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                        Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                    }
                    if (p == client->height_order_step) {
                        ++client->height_order_step;
                        if (client->height_order_step >= 3) {
                            client->world_event_claimed[mid] |= bit;
                            client->height_order_step = 0;
                            client->height_order_event = 255;
                            Chat::send_to(client, ChatChannel::kChatSystem, "",
                                "【机关】顺序正确——印记依次亮起。");
                            grant_explore(client, mid, 220, 3, def.name_zh);
                        } else {
                            Chat::send_to(client, ChatChannel::kChatSystem, "",
                                std::string{"【机关】"}
                                + height_pad_name(mid, p) + "（"
                                + std::to_string(client->height_order_step) + "/3）");
                        }
                    } else if (p > client->height_order_step) {
                        client->height_order_step = 0;
                        client->height_order_event = 255;
                        Chat::send_to(client, ChatChannel::kChatSystem, "",
                            "【机关】顺序错了……从最弱/最矮的开始。");
                    }
                    break;
                }
                continue;
            }

            if (def.kind != kTimedTouch) continue;
            if (client->world_event_claimed[mid] & bit) continue;
            if (client->timed_touch_ticks > 0 && client->timed_touch_event != ei)
                continue;
            for (uint8_t p = 0; p < 3; ++p) {
                float pdx = px - ev.pad_x[p], pdy = py - ev.pad_y[p];
                if (pdx * pdx + pdy * pdy > kTouchR2) continue;
                uint8_t pbit = (uint8_t) (1u << p);
                if (client->timed_touch_mask & pbit) continue;
                if (client->timed_touch_ticks == 0) {
                    client->timed_touch_ticks = kTimedWindowTicks;
                    client->timed_touch_mask = 0;
                    client->timed_touch_event = ei;
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.lore_en);
                }
                client->timed_touch_mask |= pbit;
                uint8_t n = 0;
                for (uint8_t q = 0; q < 3; ++q)
                    if (client->timed_touch_mask & (1u << q)) ++n;
                if (n < 3) {
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"【印记】"} + std::to_string(n) + "/3");
                    continue;
                }
                client->world_event_claimed[mid] |= bit;
                client->timed_touch_ticks = 0;
                client->timed_touch_mask = 0;
                client->timed_touch_event = 255;
                Chat::send_to(client, ChatChannel::kChatSystem, "",
                    "【印记】三枚印记同时记得你。");
                grant_explore(client, mid, 200, 3, def.name_zh);
            }
        }
    }
}

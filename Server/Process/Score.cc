#include <Server/Process.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

// Legacy hook: PvP used to set flower.score_reward = score/2 so the killer
// stole half the victim's score. PvP is gone — keep score_reward at 0 for
// flowers. Mob XP is set on spawn from MOB_DATA.xp, not here.
void tick_score_behavior(Simulation *, Entity &ent) {
    if (ent.has_component(kFlower))
        ent.score_reward = 0;
}
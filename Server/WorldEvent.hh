#pragma once

class GameInstance;
class Client;

// Minimal unified event framework (design Phase 2).
// Start with no-reward lore discoveries; expand templates later.
namespace WorldEvent {
    void spawn_for_map(GameInstance *game);
    void tick(GameInstance *game);
    // Clear in-progress timed-touch on death / respawn / map leave.
    void clear_client(Client *client);
}

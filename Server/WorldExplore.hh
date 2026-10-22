#pragma once

class GameInstance;
class Client;

// Exploration: 3-touch pads + map-level guide spirit (center → dest → re-roll).
// Spirit is NOT player-owned — players may follow it; it ignores players.
namespace WorldExplore {
    void spawn_for_map(GameInstance *game);
    void tick(GameInstance *game);
    // Clears per-client puzzle bookkeeping only — never deletes map spirits.
    void clear_client(Client *client);
    void on_player_spawned(Client *client);
}

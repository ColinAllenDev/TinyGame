#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot 
{
    class Player;
    class Ball;
    class Node3D;
    class InputManager;
    class ResourceLoader;

    enum GameState {AwaitPlayers, MatchInProgress, MatchFinished};

    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)
    private:
        // Attributes
        double round_start_timer;
        double score_timer;

        // References
        HashMap<int, Player*> players;
        Player* team_players[2][2];
        int team_scores[2];
        int round_team;
        int round_serve;

        // Components
        Node* game_scene;
        GameState game_state;
        Ball* ball;
        Node3D* marker;
        HashMap<int, Node3D*> spawns;
        InputManager* input_manager;
        ResourceLoader* resource_loader;
    protected:
        static void _bind_methods();
    public:
        GameManager() {}
        ~GameManager() {}

        void _ready();

        void init_game();
        void init_spawns();
        void add_player();
        void remove_player(int p_id);

        void start_match();
        void start_round();
        void reset_round();

        // Listeners
        void _on_input_request_player_join(int p_id);
        void _on_player_served(Vector3 p_position, Vector3 p_direction);
        void _on_player_striked(Vector3 p_to);
        void _on_team_scored(int p_team, int p_player);

        // Getters and Setters
        GameState get_game_state() const;
        int get_team_score(int p_team) const;
    };
}

#endif // GAMEMANAGER_H
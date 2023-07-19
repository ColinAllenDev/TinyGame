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
        // Data
        GameState game_state;
        uint round_count;
        double round_start_timer;
        double reset_timer;
        int team_scores[2];

        // References
        Node* game_scene;
        HashMap<int, Player*> players;
        Player* player_scored;
        Ball* ball;
        Node3D* marker;
        Vector3 spawns[2][4];
        InputManager* input_manager;
        ResourceLoader* resource_loader;
        
    protected:
        static void _bind_methods();
    public:
        GameManager() {}
        ~GameManager() {}

        void _process(double delta);
        void _ready();

        void init_game();
        void init_spawns();
        void add_player();
        void remove_player(int p_id);

        void start_match();
        void start_round();
        void reset_round();

        void set_spawns(Player* p_serving);

        // Listeners
        void _on_input_request_player_join(int p_id);
        void _on_player_served(Vector3 p_position, Vector3 p_direction);
        void _on_player_striked(Vector3 p_to);
        void _on_team_scored(int p_team, int p_player);

        // Getters and Setters
        GameState get_game_state() const { return game_state; }
        int get_team_score(int p_team) const { return team_scores[p_team]; }
    };
}

#endif // GAMEMANAGER_H
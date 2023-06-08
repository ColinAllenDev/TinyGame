#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot 
{
    class Player;
    class InputManager;
    class InputEvent;
    
    enum GameState {AwaitPlayers, MatchInProgress, MatchFinished};

    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)
    private:
        GameState game_state;
        HashMap<int, Player*> players;
        InputManager* input_manager;

    protected:
        static void _bind_methods();
    public:
        GameManager() {}
        ~GameManager() {}

        void _ready();

        void add_player(int p_id);
        void remove_player(int p_id);      

        // Listeners
        void _on_input_request_player_join(int p_id);

        // Getters and Setters
        GameState get_game_state() const;
    };
}

#endif // GAMEMANAGER_H
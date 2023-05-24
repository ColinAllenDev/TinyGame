#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot 
{
    class Player;
    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)
    private:
        // Player
        //Vector<Player*> players; 
        int num_players;
 
    protected:
        static void _bind_methods();
    public:
        GameManager() {}
        ~GameManager() {}

        void _ready();

        void add_player(int player_index);
        void remove_player(int player_index);        
    };
}

#endif // GAMEMANAGER_H
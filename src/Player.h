#ifndef PLAYER_H
#define PLAYER_H

#include <godot_cpp/classes/node3d.hpp>

namespace godot 
{
    class PlayerController;

    enum PlayerState {Serving, Idle, Moving};

    class Player : public Node3D 
    {
        GDCLASS(Player, Node3D)
    private:
        PlayerState player_state;
        int player_id;
        int device_id;
    protected:
        static void _bind_methods();
    public:
        Player() {}
        ~Player() {}

        void _ready();

        PlayerState get_player_state() const;
        void set_player_state(PlayerState p_state);
        int get_player_id() const;
        void set_player_id(const int p_id);
        int get_device_id() const;
        void set_device_id(const int p_device_id);
    };
}

#endif // PLAYER_H
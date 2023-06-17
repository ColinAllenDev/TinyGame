#ifndef PLAYER_H
#define PLAYER_H

#include <godot_cpp/classes/node3d.hpp>

namespace godot 
{
    class PlayerController;

    enum PlayerState {Serving, Moving};

    class Player : public Node3D 
    {
        GDCLASS(Player, Node3D)
    private:
        PlayerState player_state;
        int player_id;
        int device_id;
        int team;
        int score;
    protected:
        static void _bind_methods();
    public:
        Player() {}
        ~Player() {}

        void _ready();

        PlayerState get_player_state() const { return player_state; }
        void set_player_state(PlayerState p_state);
        int get_player_id() const { return player_id; }
        void set_player_id(const int p_id) { player_id = p_id; }
        int get_device_id() const { return device_id; }
        void set_device_id(const int p_device_id) { device_id = p_device_id; }
        int get_player_team() const { return team; }
        void set_player_team(const int p_team) { team = p_team; }
        int get_player_score() const { return score; }
        void set_player_score(const int p_score) { score = p_score; }
    };
}

#endif // PLAYER_H
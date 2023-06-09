#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{
    class Player;
    class Input;
    class InputMap;

    class PlayerController : public CharacterBody3D
    {
        GDCLASS(PlayerController, CharacterBody3D)
    private:
        Player* player;
        int player_id;

        // Input
        Input* input;
        InputMap* input_map;
        Vector2 input_direction;

        // Movement/Physics
        double movement_deadzone;
        double max_speed;
        double max_acceleration;
        double max_fall_acceleration;
        double jump_impulse;
        Vector3 look_direction;
        Vector3 movement_direction;
        Vector3 aim_direction;
        Vector3 current_velocity;
    protected:
        static void _bind_methods();
    public:
        PlayerController();
        ~PlayerController() {}

        void _ready();
        void _process(double delta);
        void _physics_process(double delta);
        
        void serve();
        void handle_input();

        // Getters and Setters
        double get_movement_deadzone() const;
        void set_movement_deadzone(const double p_movement_deadzone);
        double get_max_speed() const;
        void set_max_speed(const double p_speed);
        double get_max_acceleration() const;
        void set_max_acceleration(const double p_acceleration);
        double get_max_fall_acceleration() const;
        void set_max_fall_acceleration(const double p_acceleration);
        double get_jump_impulse() const;
        void set_jump_impulse(const double p_impulse);
    };  
}

#endif
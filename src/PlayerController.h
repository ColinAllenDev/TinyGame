#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{
    class Player;
    class Input;
    class InputMap;
    class Area3D;
    class Timer;

    class PlayerController : public CharacterBody3D
    {
        GDCLASS(PlayerController, CharacterBody3D)
    private:
        // Instances
        Node* current_scene;

        // Attributes
        int player_id;
        double strike_rate;
        double strike_force;
        double strike_angle;
        
        // Components
        Player* player_instance;
        Area3D* strike_area;

        // Input
        Input* input;
        InputMap* input_map;
        Vector2 input_direction;

        // Action Handling
        bool can_strike;

        // Movement/Physics
        double movement_deadzone;
        double max_speed;
        double max_acceleration;
        double max_fall_acceleration;
        double jump_impulse;
        Vector3 look_direction;
        Vector3 movement_direction;
        Vector3 current_velocity;
    protected:
        static void _bind_methods();
    public:
        PlayerController();
        ~PlayerController() {}

        void _ready();
        void _process(double delta);
        void _physics_process(double delta);
        
        void handle_input();

        void serve();
        void strike();
        
        // Action timer listeners
        void _on_can_strike() { can_strike = true; }

        // Getters and Setters
        int get_player_id() const { return player_id; }
        double get_strike_force() const { return strike_force; }
        void set_strike_force(double p_force) { strike_force = p_force; }
        double get_movement_deadzone() const { return movement_deadzone; }
        void set_movement_deadzone(const double p_movement_deadzone) { movement_deadzone = p_movement_deadzone; }
        double get_max_speed() const { return max_speed; }
        void set_max_speed(const double p_speed) { max_speed = p_speed; }
        double get_max_acceleration() const { return max_acceleration; }
        void set_max_acceleration(const double p_acceleration) { max_acceleration = p_acceleration; }
        double get_max_fall_acceleration() const { return max_fall_acceleration; }
        void set_max_fall_acceleration(const double p_acceleration) { max_fall_acceleration = p_acceleration; }
        double get_jump_impulse() const { return jump_impulse; }
        void set_jump_impulse(const double p_impulse) { jump_impulse = p_impulse; }
    };  
}

#endif
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{
    class Player;
    class Input;
    class InputMap;
    class Area3D;
    class Ball;

    class PlayerController : public CharacterBody3D
    {
        GDCLASS(PlayerController, CharacterBody3D)
    private:
        // References
        Player* player_instance;

        // Attributes
        int player_id;
        int player_state;
        int player_team;
        double strike_rate;
                
        // Components
        Area3D* strike_area;
        Node3D* pivot;

        // Input
        Input* input;
        InputMap* input_map;
        Vector2 input_direction;

        // Conditionals
        bool can_strike;
        bool can_move;
        bool has_input;
        bool has_movement_input;
        bool has_strike_input;
        bool has_jump_input;
        
        // Movement/Physics
        double movement_deadzone;
        double max_speed;
        double max_acceleration;
        double max_fall_acceleration;
        double jump_impulse;
        int default_face;
        Vector3 facing_direction;
        Vector3 pivot_direction;
        Vector3 movement_direction;
        Vector3 current_velocity;
        Vector3 target_velocity;

    protected:
        static void _bind_methods();

        void handle_input();
        void handle_movement(double delta);
        void handle_actions();
    public:
        PlayerController();
        ~PlayerController() {}

        void _ready();
        void _process(double delta);
        void _physics_process(double delta);

        void serve();
        void strike();

        // Action timer listeners
        void _on_player_state_changed(int p_state) { player_state = p_state; }
        void _on_can_strike() { can_strike = true; }

        // Getters and Setters
        int get_player_id() const { return player_id; }
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
        Node3D* get_pivot() const { return pivot; }
    };  
}

#endif
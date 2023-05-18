#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{
    class Input;
    
    class PlayerController : public CharacterBody3D
    {
        GDCLASS(PlayerController, CharacterBody3D)
    private:
        // Input
        double input_deadzone;
        double movement_deadzone;
        Vector2 input_direction;
        Input* input;

        // Movement/Physics
        double max_speed;
        double max_acceleration;
        Vector3 movement_direction;
        Vector3 current_velocity;
    protected:
        static void _bind_methods();
    public:
        PlayerController();
        ~PlayerController();

        void _process_input();
        void _process(double delta);
        void _physics_process(double delta);

        double get_input_deadzone() const;
        void set_input_deadzone(const double p_input_deadzone);
        double get_movement_deadzone() const;
        void set_movement_deadzone(const double p_movement_deadzone);

        double get_max_speed() const;
        void set_max_speed(const double p_speed);
        double get_max_acceleration() const;
        void set_max_acceleration(const double p_acceleration);
    };  
}

#endif
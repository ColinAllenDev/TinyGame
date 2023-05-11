#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{
    class PlayerController : public CharacterBody3D
    {
        GDCLASS(PlayerController, CharacterBody3D)
    private:
        double max_speed;
        Vector2 input_direction = Vector2(0.0f, 0.0f);
        Vector3 current_direction = Vector3(0.0f, 0.0f ,0.0f);
        Vector3 current_velocity = Vector3(0.0f, 0.0f ,0.0f);
    protected:
        static void _bind_methods();
    public:
        PlayerController();
        ~PlayerController();

        void _process_input();
        void _process(double delta);
        void _physics_process(double delta);

        double get_max_speed() const;
        void set_max_speed(const double p_speed);
    };  
}

#endif
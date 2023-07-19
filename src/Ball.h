#ifndef BALL_H
#define BALL_H

#include <godot_cpp/classes/rigid_body3d.hpp>

namespace godot 
{
    class Area3D;
    class RigidBody3D;
    class CollisionShape3D;
    class MeshInstance3D;
    class PlayerController;
    class Vector3;
    class Player;

    class Ball : public RigidBody3D 
    {
        GDCLASS(Ball, RigidBody3D)
    private:
        // Attributes
        double serve_force;
        double serve_height;
        double max_strike_height;
        double min_strike_height;

        // Components
        RigidBody3D* body;
        Area3D* strike_area;
        Area3D* detect_area;
        CollisionShape3D* body_collider;
        CollisionShape3D* area_collider;

        // Conditionals
        bool can_score;

        // Misc
        int last_player_id;
    protected:
        static void _bind_methods();
    public:
        Ball();
        ~Ball() {}

        void _ready();
        void serve(Vector3 p_position);

        // Listeners
        void _on_strike_area_entered(Area3D* area);
        void _on_detect_area_entered(Area3D* area);
        void _on_strike_area_exited(Area3D* area);
        void _on_player_striked(Vector3 p_to);
        
        // Util Functions
        double get_strike_pitch(Vector3 p_displacement, Vector3 p_up, double p_height);
        double get_strike_speed(Vector3 p_displacement, Vector3 p_up, double p_height, double p_angle);
        Vector3 project_on_plane(Vector3 p_vector, Vector3 p_normal);
        double get_signed_angle(Vector3 p_a, Vector3 p_b, Vector3 p_axis);

        // Getters and Setters
        double get_serve_force() const { return serve_force; }
        void set_serve_force(double p_force) { serve_force = p_force; }
        void set_can_score(bool p_value) { can_score = p_value; }
    };
}

#endif
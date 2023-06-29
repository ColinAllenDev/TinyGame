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
        double max_height;

        // Components
        RigidBody3D* body;
        Area3D* strike_area;
        Area3D* detect_area;
        CollisionShape3D* body_collider;
        CollisionShape3D* area_collider;

        // Temporary
        MeshInstance3D* pivot_mesh;

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
        void _on_player_striked(Vector3 p_to, float p_force);
        
        // Temporary Util Functions
        Vector3 project_on_plane(Vector3 p_vector, Vector3 p_normal);
        double get_signed_angle(Vector3 p_a, Vector3 p_b, Vector3 p_axis);

        // Getters and Setters
        double get_serve_force() const { return serve_force; }
        void set_serve_force(double p_force) { serve_force = p_force; }
    };
}

#endif
#ifndef BALL_H
#define BALL_H

#include <godot_cpp/classes/rigid_body3d.hpp>

namespace godot 
{
    class Area3D;
    class RigidBody3D;
    class CollisionShape3D;
    class PlayerController;
    class Vector3;

    class Ball : public RigidBody3D 
    {
        GDCLASS(Ball, RigidBody3D)
    private:
        // Attributes
        double serve_force;

        // Components
        RigidBody3D* body;
        Area3D* strike_area;
        CollisionShape3D* body_collider;
        CollisionShape3D* area_collider;
    protected:
        static void _bind_methods();
    public:
        Ball();
        ~Ball() {}

        void _ready();

        void serve(Vector3 p_position);

        void _on_strike_area_entered(Area3D* area);
        void _on_strike_area_exited(Area3D* area);
        void _on_player_striked(Node3D* pivot, float p_force);
        double get_serve_force() const { return serve_force; }
        void set_serve_force(double p_force) { serve_force = p_force; }
    };
}

#endif
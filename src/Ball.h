#ifndef BALL_H
#define BALL_H

#include <godot_cpp/classes/rigid_body3d.hpp>

namespace godot 
{
    class Area3D;
    class RigidBody3D;
    class CollisionShape3D;

    class Ball : public RigidBody3D 
    {
        GDCLASS(Ball, RigidBody3D)
    private:
        RigidBody3D* body;
        Area3D* strike_area;
        CollisionShape3D* body_collider;;
        CollisionShape3D* area_collider;
    protected:
        static void _bind_methods();
    public:
        Ball() {}
        ~Ball() {}

        void _ready();
        
        void _on_strike_area_entered(Node3D* body);
    };
}

#endif
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <godot_cpp/classes/camera3d.hpp>

namespace godot 
{
    class Ball;

    class CameraController : public Camera3D 
    {
        GDCLASS(CameraController, Camera3D);
    private:
        Vector3 initial_position;
        Vector3 initial_rotation;
        Vector3 min_bounds;
        Vector3 max_bounds;

        Vector3 target_position;
        Ball* ball_target;
        double smooth_time;
        double camera_speed;
        bool can_follow;
    protected:
        static void _bind_methods();
    
    public:
        CameraController() {};
        ~CameraController() {};

        void _ready();
        void _physics_process(double delta);

        void smooth_follow(double delta);

        void _on_node_added(Node* node);
        void _on_target_position_changed(Vector3 p_target);
    };
}

#endif
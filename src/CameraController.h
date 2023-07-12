#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <godot_cpp/classes/camera3d.hpp>

namespace godot 
{
    class CameraController : public Camera3D 
    {
        GDCLASS(CameraController, Camera3D);
    private:
    protected:
        static void _bind_methods();
    public:
        CameraController() {};
        ~CameraController() {};

        void _ready();
        void _physics_process(double delta);

        void smooth_follow();

        void _on_node_added(Node* node);
        void _on_target_position_changed(Vector3 p_target);
    };
}

#endif
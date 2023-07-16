#include "CameraController.h"
#include "GameManager.h"
#include "Ball.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void CameraController::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("_on_node_added", "node"), &CameraController::_on_node_added);
    ClassDB::bind_method(D_METHOD("_on_target_position_changed", "p_target"), &CameraController::_on_target_position_changed);
}

void CameraController::_ready() 
{    
    if (Engine::get_singleton()->is_editor_hint()) return;

    target_position = get_global_position();
    camera_speed = 0.025;
    min_bounds = -4.0;
    max_bounds = 4.0;

    SceneTree* main_scene = get_tree();
    main_scene->connect("node_added", Callable(this, "_on_node_added"));
}

double t = 0.0;
void CameraController::_physics_process(double delta) 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    if (can_follow) 
    {
        smooth_follow(delta);
    }
}

void CameraController::smooth_follow(double delta) 
{
    t += delta * camera_speed;
    Vector3 initial_position = get_global_position();
    set_global_position(initial_position.lerp(target_position, t));

    if (initial_position == target_position) 
    {
        t = 0.0;
        can_follow = false;
    }
}

void CameraController::_on_node_added(Node* node) 
{
    if (node->get_class() == "Ball") 
    {  
        ball_target = (Ball*) node;
        ball_target->connect("target_position_changed", Callable(this, "_on_target_position_changed"));
    }
}

void CameraController::_on_target_position_changed(Vector3 p_target) 
{
    Vector3 initial_position = get_global_position();
    double target_x = p_target.x;

    target_position = Vector3(Math::clamp(target_x, min_bounds, max_bounds), initial_position.y, initial_position.z);
    can_follow = true;
}
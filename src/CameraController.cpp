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

    SceneTree* main_scene = get_tree();
    main_scene->connect("node_added", Callable(this, "_on_node_added"));
}

void CameraController::_physics_process(double delta) 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    smooth_follow();
}

void CameraController::smooth_follow() 
{
    
}

void CameraController::_on_node_added(Node* node) 
{
    if (node->get_class() == "Ball") 
    {
        node_target = (Node3D*) node;
    }
}

void CameraController::_on_target_position_changed(Vector3 p_target) 
{
    
}
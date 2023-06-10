#include "Ball.h"
#include "PlayerController.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void Ball::_bind_methods() 
{
}

void Ball::_ready() 
{   
    // Get scene object and root node
    Node* current_scene = get_tree()->get_current_scene();
    Node* root_instance = current_scene->get_node<Ball>("Ball");

    // Initialize component pointers
    body = (RigidBody3D*)root_instance;
    strike_area = root_instance->get_node<Area3D>("StrikeArea");
    body_collider = root_instance->get_node<CollisionShape3D>("BodyCollider");
    area_collider = strike_area->get_node<CollisionShape3D>("StrikeCollider");

    strike_area->connect("area_entered", Callable(this, "_on_strike_area_entered"));
}

void Ball::_physics_process(double delta) 
{
    UtilityFunctions::print("Ball Position: ", body->get_position());
}

void Ball::_on_strike_area_entered(Area3D area) {
    UtilityFunctions::print("Area Entered!");
}
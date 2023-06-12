#include "Ball.h"
#include "Player.h"
#include "PlayerController.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void Ball::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("_on_strike_area_entered", "area"), &Ball::_on_strike_area_entered);
    ClassDB::bind_method(D_METHOD("_on_strike_area_exited", "area"), &Ball::_on_strike_area_exited);
    ClassDB::bind_method(D_METHOD("_on_player_striked", "p_position", "p_direction", "p_force"), &Ball::_on_player_striked);
}

void Ball::_ready() 
{   
    // Disables logic below inside editor
    if (Engine::get_singleton()->is_editor_hint()) return;
    
    // Get scene object and root node
    Node* current_scene = get_tree()->get_current_scene();
    Node* root_instance = current_scene->get_node<Ball>("Ball");

    // Initialize component pointers
    body = (RigidBody3D*)root_instance;
    strike_area = root_instance->get_node<Area3D>("StrikeArea");
    body_collider = root_instance->get_node<CollisionShape3D>("BodyCollider");
    area_collider = strike_area->get_node<CollisionShape3D>("StrikeCollider");

    // Connect signals
    strike_area->connect("area_entered", Callable(this, "_on_strike_area_entered"));
    strike_area->connect("area_exited", Callable(this, "_on_strike_area_exited"));
}

void Ball::_on_strike_area_entered(Area3D* area) {    
    if (area->get_parent()->get_class() == "PlayerController") 
    {
        PlayerController* controller_instance = (PlayerController*)area->get_parent();
        if (!controller_instance->is_connected("player_striked", Callable(this, "_on_player_striked")))
            controller_instance->connect("player_striked", Callable(this, "_on_player_striked"));
    }
}

void Ball::_on_strike_area_exited(Area3D* area) {
    if (area->get_parent()->get_class() == "PlayerController") 
    {
        PlayerController* controller_instance = (PlayerController*)area->get_parent();
        if (controller_instance->is_connected("player_striked", Callable(this, "_on_player_striked")))
            controller_instance->disconnect("player_striked", Callable(this, "_on_player_striked"));
    }
}

void Ball::_on_player_striked(PlayerController* p_player, Vector3 p_position, Vector3 p_direction, float p_force) 
{
    UtilityFunctions::print("Ball Struck by ", p_player->get_player_id(), " with direction ", p_direction);
}
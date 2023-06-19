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
    if (Engine::get_singleton()->is_editor_hint()) return;

    ClassDB::bind_method(D_METHOD("_on_strike_area_entered", "area"), &Ball::_on_strike_area_entered);
    ClassDB::bind_method(D_METHOD("_on_detect_area_entered", "area"), &Ball::_on_detect_area_entered);
    ClassDB::bind_method(D_METHOD("_on_strike_area_exited", "area"), &Ball::_on_strike_area_exited);
    ClassDB::bind_method(D_METHOD("_on_player_striked", "p_pivot", "p_force"), &Ball::_on_player_striked);

    ClassDB::bind_method(D_METHOD("get_serve_force"), &Ball::get_serve_force);
    ClassDB::bind_method(D_METHOD("set_serve_force", "p_force"), &Ball::set_serve_force);
    ClassDB::add_property("Ball", PropertyInfo(Variant::FLOAT, "serve_force"), "set_serve_force", "get_serve_force");

    ADD_SIGNAL(MethodInfo("team_scored", 
        PropertyInfo(Variant::INT, "p_team"),
        PropertyInfo(Variant::INT, "p_player")));
}

Ball::Ball() 
{
    serve_force = 100.0;
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
    detect_area = root_instance->get_node<Area3D>("DetectArea");
    body_collider = root_instance->get_node<CollisionShape3D>("BodyCollider");
    area_collider = strike_area->get_node<CollisionShape3D>("StrikeCollider");

    // Connect signals
    strike_area->connect("area_entered", Callable(this, "_on_strike_area_entered"));
    detect_area->connect("area_entered", Callable(this, "_on_detect_area_entered"));
    strike_area->connect("area_exited", Callable(this, "_on_strike_area_exited"));

    // Temporary, may change this
    set_lock_rotation_enabled(true);
}

void Ball::_process(double delta) 
{
    UtilityFunctions::print(get_global_position().y);
}

void Ball::serve(Vector3 p_position) 
{
    float target_height = 6.0;
    float gravity = get_gravity_scale() * 9.8;
    float velocity_y = Math::sqrt(2 * gravity * target_height);
    
    Vector3 velocity = Vector3(0, velocity_y, 0);

    apply_central_impulse(velocity);
}

void Ball::_on_strike_area_entered(Area3D* area) {    
    if (area->get_parent()->get_class() == "PlayerController") 
    {
        PlayerController* controller_instance = (PlayerController*)area->get_parent();
        if (!controller_instance->is_connected("player_striked", Callable(this, "_on_player_striked")))
            controller_instance->connect("player_striked", Callable(this, "_on_player_striked"));
    }
}

void Ball::_on_detect_area_entered(Area3D* area) 
{
    Vector3 hit_position = area->get_global_position();
    int team_number = hit_position.x > 0 ? 0 : 1;

    emit_signal("team_scored", team_number, last_player_id);
}

void Ball::_on_strike_area_exited(Area3D* area) {
    if (area->get_parent()->get_class() == "PlayerController") 
    {
        PlayerController* controller_instance = (PlayerController*)area->get_parent();
        if (controller_instance->is_connected("player_striked", Callable(this, "_on_player_striked")))
            controller_instance->disconnect("player_striked", Callable(this, "_on_player_striked"));

        last_player_id = controller_instance->get_player_id();
    }
}

void Ball::_on_player_striked(Node3D* p_pivot, float p_force) 
{
    float delta_time = get_physics_process_delta_time();
    float strike_force = 20.0;
    float max_height = 6.0;
    float current_velocity = get_linear_velocity().y;
    float target_height = max_height - get_global_position().y;

    Vector3 strike_impulse = Vector3(0, 2 * Math::abs(current_velocity), 0);
    
    apply_central_impulse(strike_impulse * strike_force * get_physics_process_delta_time());
}
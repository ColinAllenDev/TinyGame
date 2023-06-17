#include "PlayerController.h"
#include "Player.h"
#include "Ball.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

using namespace godot;

void PlayerController::_bind_methods() 
{
    // Attributes
    ClassDB::bind_method(D_METHOD("get_player_id"), &PlayerController::get_player_id);
    
    ClassDB::bind_method(D_METHOD("get_strike_force"), &PlayerController::get_strike_force);
    ClassDB::bind_method(D_METHOD("set_strike_force", "p_force"), &PlayerController::set_strike_force);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "strike_force"), "set_strike_force", "get_strike_force");

    // Movement
    ClassDB::bind_method(D_METHOD("get_movement_deadzone"), &PlayerController::get_movement_deadzone);
    ClassDB::bind_method(D_METHOD("set_movement_deadzone", "p_input_deadzone"), &PlayerController::set_movement_deadzone);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "movement_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_movement_deadzone", "get_movement_deadzone");

    // Speed
    ClassDB::bind_method(D_METHOD("get_speed"), &PlayerController::get_max_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &PlayerController::set_max_speed);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_speed"), "set_speed", "get_speed");

    // Acceleration
    ClassDB::bind_method(D_METHOD("get_acceleration"), &PlayerController::get_max_acceleration);
    ClassDB::bind_method(D_METHOD("set_acceleration", "p_acceleration"), &PlayerController::set_max_acceleration);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_acceleration"), "set_acceleration", "get_acceleration");

    ClassDB::bind_method(D_METHOD("get_fall_acceleration"), &PlayerController::get_max_fall_acceleration);
    ClassDB::bind_method(D_METHOD("set_fall_acceleration", "p_acceleration"), &PlayerController::set_max_fall_acceleration);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_fall_acceleration"), "set_fall_acceleration", "get_fall_acceleration");
    
    ClassDB::bind_method(D_METHOD("get_jump_impulse"), &PlayerController::get_jump_impulse);
    ClassDB::bind_method(D_METHOD("set_jump_impulse", "p_impulse"), &PlayerController::set_jump_impulse);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "jump_impulse"), "set_jump_impulse", "get_jump_impulse");

    ClassDB::bind_method(D_METHOD("get_pivot"), &PlayerController::get_pivot);

    // Listeners
    ClassDB::bind_method(D_METHOD("_on_player_state_changed", "p_state"), &PlayerController::_on_player_state_changed);
    ClassDB::bind_method(D_METHOD("_on_can_strike"), &PlayerController::_on_can_strike);

    // Signals
    ADD_SIGNAL(MethodInfo("player_served", 
        PropertyInfo(Variant::VECTOR3, "player_position"),
        PropertyInfo(Variant::VECTOR3, "aim_direction")));

    ADD_SIGNAL(MethodInfo("player_striked", 
        PropertyInfo(Variant::OBJECT, "pivot"),
        PropertyInfo(Variant::FLOAT, "p_force")));
}

PlayerController::PlayerController() 
{
    // Action Defaults
    strike_rate = 1;
    strike_force = 20.0;
    can_strike = true;

    // Movement Defaults
    movement_deadzone = 0.32;
    max_speed = 14.0f;
    max_acceleration = 45.0f;
    max_fall_acceleration = 75.0f;
    jump_impulse = 30.0f;
    default_face = 1;
}

void PlayerController::_ready() 
{
    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Get relevant attributes from parent node
    player_instance = (Player*)get_parent();
    player_instance->connect("player_state_changed", Callable(this, "_on_player_state_changed"));
    player_state = player_instance->get_player_state();

    // Component and Attribute Initializaton
    Node* controller_instance = player_instance->get_node<PlayerController>("PlayerController");
    input = Input::get_singleton();
    strike_area = controller_instance->get_node<Area3D>("StrikeArea");
    player_id = player_instance->get_player_id();

    // TEMPORARY
    if (player_id == 1) default_face = -1;
    facing_direction = Vector3(default_face, 0, 0);

    // Pivot
    pivot = controller_instance->get_node<Node3D>("Pivot");
    pivot_direction = facing_direction;

    look_at(get_global_position() + facing_direction);
}

void PlayerController::_process(double delta) 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    handle_input();
}

void PlayerController::_physics_process(double delta) 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Handle movmenet, aiming, and actions
    if(player_state == PlayerState::Moving) 
    {
        handle_movement(delta);
    }

    handle_aiming();
    handle_actions();

    // Look towards facing direction
    if (has_input) {
        facing_direction = Vector3(input_direction.x, 0, input_direction.y);
    }
    look_at(get_global_position() + facing_direction);

    // Set velocity on y axis
    current_velocity.y = target_velocity.y;

    set_velocity(current_velocity);
    move_and_slide();
}

void PlayerController::handle_input() 
{
    // Vectors
    input_direction.x =
        input->get_action_strength("move_right_"+String::num(player_id)) - input->get_action_strength("move_left_"+String::num(player_id));
    input_direction.y =
        input->get_action_strength("move_down_"+String::num(player_id)) - input->get_action_strength("move_up_"+String::num(player_id));

    // Conditionals
    has_input = (input_direction.length() > 0);
    has_movement_input = (input_direction.length() > movement_deadzone*sqrt(2.0f));
    has_strike_input = input->is_action_pressed("strike_"+String::num(player_id));
    has_jump_input = input->is_action_pressed("jump_"+String::num(player_id));
}

void PlayerController::handle_movement(double delta) 
{
    // Handle movement direction based on input
    if (has_movement_input) 
    {
        movement_direction = Vector3(input_direction.x, 0.0f, input_direction.y).normalized();
    } else {
        movement_direction = Vector3(0.0f, 0.0f, 0.0f); 
    }

    // Update our target velocity based on movement direction and speed
    target_velocity = Vector3(movement_direction.x, 0.0f, movement_direction.z) * max_speed;

    float max_speed_delta = max_acceleration * (float)delta;

    current_velocity.x =
        ABS(target_velocity.x - current_velocity.x) <= 
            max_speed_delta ? target_velocity.x : current_velocity.x + 
                SIGN(target_velocity.x - current_velocity.x) * max_speed_delta;

    current_velocity.z =
        ABS(target_velocity.z - current_velocity.z) <= 
            max_speed_delta ? target_velocity.z : current_velocity.z + 
                SIGN(target_velocity.z - current_velocity.z) * max_speed_delta;

    // Update movement based on velocity
    if (!is_on_floor()) 
    {
        target_velocity.y = current_velocity.y - (max_fall_acceleration * (float)delta);
    }
}

void PlayerController::handle_aiming() 
{
    // Handle pivot direction
    if (has_input) 
    {        
        // TODO: Handle facing opposite direction of the ball
        pivot_direction = Vector3(input_direction.x, 0.0f, input_direction.y);
        pivot_direction = pivot_direction.clamp(Vector3(default_face, 0, -1), Vector3(default_face, 0, 1));

    } else {
        pivot_direction = Vector3(0, 1, 0);
    }
}

void PlayerController::handle_actions() 
{
    // Jump logic
    if (has_jump_input && is_on_floor() && player_state != PlayerState::Serving) 
    {
        target_velocity.y = jump_impulse;
    }

    // Strike logic
    if (has_strike_input && can_strike)  {
        //  Start action timer
        can_strike = false;
        get_tree()->create_timer(strike_rate, false)->connect("timeout", Callable(this, "_on_can_strike"));

        // Player is serving
        if (player_state == PlayerState::Serving && is_on_floor())     
        {
            serve();
        }

        // Emit strike signal?
        if (player_state != PlayerState::Serving) 
        {
            strike();
        }
    }
}

void PlayerController::serve() 
{
    // 1. Instantiate Ball in the air
    Vector3 serve_direction = pivot_direction;
    Vector3 serve_offset = Vector3(1, 0, 0);
    emit_signal("player_served", get_position() + serve_direction + serve_offset, serve_direction); 

    // TODO: Add check if player succesfully served
    player_instance->set_player_state(PlayerState::Moving); 
}

void PlayerController::strike() 
{
    // Valid collision area logic performed by collision layers in godot
    if (strike_area->has_overlapping_areas()) 
    {
        emit_signal("player_striked", pivot, strike_force);
    }
}
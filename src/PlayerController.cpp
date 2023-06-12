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

using namespace godot;

void PlayerController::_bind_methods() 
{
    // Attributes
    ClassDB::bind_method(D_METHOD("get_player_id"), &PlayerController::get_player_id);

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

    // Listeners
    ClassDB::bind_method(D_METHOD("_on_can_strike"), &PlayerController::_on_can_strike);

    // Signals
    ADD_SIGNAL(MethodInfo("player_served", 
        PropertyInfo(Variant::VECTOR3, "player_position"),
        PropertyInfo(Variant::VECTOR3, "aim_direction")));

    ADD_SIGNAL(MethodInfo("player_striked", 
        PropertyInfo(Variant::VECTOR3, "player_position"),
        PropertyInfo(Variant::VECTOR3, "aim_direction"),
        PropertyInfo(Variant::FLOAT, "strike_force")));
}

PlayerController::PlayerController() 
{
    // Action Defaults
    strike_rate = 0.24;
    can_strike = true;

    // Movement Defaults
    movement_deadzone = 0.32;
    max_speed = 14.0f;
    max_acceleration = 45.0f;
    max_fall_acceleration = 75.0f;
    jump_impulse = 20.0f;
}

void PlayerController::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Get scene object and root node
    current_scene = get_tree()->get_current_scene();
    player_instance = current_scene->get_node<Player>("Player");
    Node* controller_instance = player_instance->get_node<PlayerController>("PlayerController");

    // Component and Attribute Initializaton
    input = Input::get_singleton();
    strike_area = controller_instance->get_node<Area3D>("StrikeArea");
    player_id = player_instance->get_player_id();
}

void PlayerController::_process(double delta) 
{
    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    handle_input();
}

void PlayerController::_physics_process(double delta) 
{
    // NOTE: Might need to move input logic to the main _process method as input might be missed
    // during physics update.

    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Handle movement direction based on input
    if (input_direction.length() > movement_deadzone*sqrt(2.0f)) 
    {
        movement_direction = Vector3(input_direction.x, 0.0f, input_direction.y).normalized();
    } else {
        movement_direction = Vector3(0.0f, 0.0f, 0.0f); 
    }

    // Update our target velocity based on movement direction and speed
    Vector3 target_velocity = Vector3(movement_direction.x, 0.0f, movement_direction.z) * max_speed;

    float max_speed_delta = max_acceleration * (float)delta;

    current_velocity.x =
        ABS(target_velocity.x - current_velocity.x) <= 
            max_speed_delta ? target_velocity.x : current_velocity.x + 
                SIGN(target_velocity.x - current_velocity.x) * max_speed_delta;

    current_velocity.z =
        ABS(target_velocity.z - current_velocity.z) <= 
            max_speed_delta ? target_velocity.z : current_velocity.z + 
                SIGN(target_velocity.z - current_velocity.z) * max_speed_delta;

    // Player is falling
    if (!is_on_floor()) 
    {
        target_velocity.y = current_velocity.y - (max_fall_acceleration * (float)delta);
    }
    
    // Player is on the floor and pressing "jump" input
    if (is_on_floor() && input->is_action_pressed("jump_"+String::num(player_id))) 
    {
        target_velocity.y = jump_impulse;
    }

    // Strike logic
    if (input->is_action_pressed("strike_"+String::num(player_id)) && can_strike) {
        //  Start action timer
        can_strike = false;
        get_tree()->create_timer(strike_rate, false)->connect("timeout", Callable(this, "_on_can_strike"));

        // Player is serving
        if (player_instance->get_player_state() == PlayerState::Serving && is_on_floor())     
        {
            serve();
        }

        // Emit strike signal?
        if (player_instance->get_player_state() != PlayerState::Serving) 
        {
            strike();
        }
    }

    // Set velocity on y axis
    current_velocity.y = target_velocity.y;

    // Update movement based on velocity
    set_velocity(current_velocity);
    move_and_slide();
}

void PlayerController::handle_input() 
{
    // Movement
    input_direction.x =
        input->get_action_strength("move_right_"+String::num(player_id)) - input->get_action_strength("move_left_"+String::num(player_id));
    input_direction.y =
        input->get_action_strength("move_down_"+String::num(player_id)) - input->get_action_strength("move_up_"+String::num(player_id));
}

void PlayerController::serve() 
{
    // 1. Instantiate Ball in the air
    Vector3 serve_direction = Vector3(0, 1, 0);
    Vector3 serve_offset = Vector3(1.5f, 0, 0);
    emit_signal("player_served", get_position() + serve_direction + serve_offset, serve_direction); 

    // TODO: Add check if player succesfully served
    player_instance->set_player_state(PlayerState::Moving); 
}

void PlayerController::strike() 
{
    // Temporary
    Vector3 strike_direction = Vector3(0.5, 0.5, 0);
    float strike_force = 14.0f;

    // Valid collision area logic performed by collision layers in godot
    if (strike_area->has_overlapping_areas()) 
    {
        emit_signal("player_striked", this, get_position(), strike_direction, strike_force);
    }
}

#pragma region Getters-Setters
int PlayerController::get_player_id() const 
{
    return player_id;
}

double PlayerController::get_movement_deadzone() const 
{
    return movement_deadzone;
}

void   PlayerController::set_movement_deadzone(const double p_movement_deadzone) 
{
    movement_deadzone = p_movement_deadzone;
}

double PlayerController::get_max_speed() const 
{
    return max_speed;
}

void PlayerController::set_max_speed(double p_speed) 
{
    max_speed = p_speed;
}

double PlayerController::get_max_acceleration() const 
{
    return max_acceleration;
}

void PlayerController::set_max_acceleration(double p_acceleration) 
{
    max_acceleration = p_acceleration;
}

double PlayerController::get_max_fall_acceleration() const 
{
    return max_fall_acceleration;
}

void PlayerController::set_max_fall_acceleration(double p_acceleration) 
{
    max_fall_acceleration = p_acceleration;
}

double PlayerController::get_jump_impulse() const 
{
    return jump_impulse;
}

void PlayerController::set_jump_impulse(const double p_impulse) 
{
    jump_impulse = p_impulse;
}
#pragma endregion Getters-Setters
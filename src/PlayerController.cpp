#include "PlayerController.h"
#include "Player.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

void PlayerController::_bind_methods() 
{
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

    // Signals
    ADD_SIGNAL(MethodInfo("player_served", 
        PropertyInfo(Variant::VECTOR3, "player_position"),
        PropertyInfo(Variant::VECTOR3, "aim_direction")));
}

PlayerController::PlayerController() 
{
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

    player = (Player*)get_parent();
    player_id = player->get_player_id();
}

void PlayerController::_process(double delta) 
{
    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    handle_input();
}

void PlayerController::_physics_process(double delta) 
{
    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    if (input_direction.length() > movement_deadzone*sqrt(2.0f)) 
    {
        movement_direction = Vector3(input_direction.x, 0.0f, input_direction.y).normalized();
    } else {
        movement_direction = Vector3(0.0f, 0.0f, 0.0f); 
    }

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

    if (!is_on_floor()) 
    {
        target_velocity.y = current_velocity.y - (max_fall_acceleration * (float)delta);
    }

    if (is_on_floor() && input->is_action_pressed("jump_"+String::num(player_id))) 
    {
        target_velocity.y = jump_impulse;
    }

    current_velocity.y = target_velocity.y;

    set_velocity(current_velocity);
    move_and_slide();

    if (input->is_action_pressed("strike_"+String::num(player_id))) {
        if (is_on_floor() && player->get_player_state() == PlayerState::Serving)
            serve();
    }
}

void PlayerController::serve() 
{
    // Temporary
    Vector3 local_forward = get_transform().basis.xform(Vector3(0, 0, -1));
    UtilityFunctions::print(local_forward);
    
    aim_direction = Vector3(0.5, 0.5, 0.0);

    // if player handles serve input, send out a signal that we served to other game entities
    emit_signal("player_served", get_position(), aim_direction); 

    // TODO: Add check if player succesfully served
    player->set_player_state(PlayerState::Moving); 
}

void PlayerController::handle_input() 
{
    input = Input::get_singleton();
    
    // Movement
    input_direction.x =
        input->get_action_strength("move_right_"+String::num(player_id)) - input->get_action_strength("move_left_"+String::num(player_id));
    input_direction.y =
        input->get_action_strength("move_down_"+String::num(player_id)) - input->get_action_strength("move_up_"+String::num(player_id));
}

#pragma region Getters-Setters
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
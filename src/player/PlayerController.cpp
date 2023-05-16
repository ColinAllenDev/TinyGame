#include "PlayerController.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PlayerController::_bind_methods() 
{
    // Input
    ClassDB::bind_method(D_METHOD("get_input_deadzone"), &PlayerController::get_input_deadzone);
    ClassDB::bind_method(D_METHOD("set_input_deadzone", "p_input_deadzone"), &PlayerController::set_input_deadzone);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "input_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_input_deadzone", "get_input_deadzone");
    
    ClassDB::bind_method(D_METHOD("get_movement_deadzone"), &PlayerController::get_movement_deadzone);
    ClassDB::bind_method(D_METHOD("set_movement_deadzone", "p_input_deadzone"), &PlayerController::set_movement_deadzone);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "movement_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_movement_deadzone", "get_movement_deadzone");

    // Speed
    ClassDB::bind_method(D_METHOD("get_speed"), &PlayerController::get_max_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &PlayerController::set_max_speed);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_speed", PROPERTY_HINT_RANGE, "1, 2000, 1"), "set_speed", "get_speed");

    // Acceleration
    ClassDB::bind_method(D_METHOD("get_acceleration"), &PlayerController::get_max_acceleration);
    ClassDB::bind_method(D_METHOD("set_acceleration", "p_acceleration"), &PlayerController::set_max_acceleration);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_acceleration", PROPERTY_HINT_RANGE, "1, 2000, 1"), "set_acceleration", "get_acceleration");

    // Time Emit
    //ADD_SIGNAL(MethodInfo("position_changed", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::VECTOR2, "new_pos")));
}

PlayerController::PlayerController() 
{
    // Input Configuration
    input_deadzone = 0.1;
    movement_deadzone = 0.32;
    
    input = Input::get_singleton();

    InputMap* input_map = InputMap::get_singleton();
    input_map->action_set_deadzone("move_right", input_deadzone);
    input_map->action_set_deadzone("move_left", input_deadzone);
    input_map->action_set_deadzone("move_forward", input_deadzone);
    input_map->action_set_deadzone("move_backward", input_deadzone);

    // Movement configuration
    max_speed = 14.0f;
    max_acceleration = 45.0f;
}

PlayerController::~PlayerController() 
{

}

void PlayerController::_process_input() 
{
    input_direction.x = 
        input->get_action_strength("move_right") - input->get_action_strength("move_left");

    input_direction.y = 
        input->get_action_strength("move_backward") - input->get_action_strength("move_forward");

    if (input_direction.length() > movement_deadzone*sqrt(2.0f)) 
    {
        movement_direction = Vector3(input_direction.x, 0.0f, input_direction.y).normalized();
    } else {
        movement_direction = Vector3(0.0f, 0.0f, 0.0f); 
    }
}

void PlayerController::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    _process_input();
}

void PlayerController::_physics_process(double delta) 
{
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

    set_velocity(current_velocity);
    move_and_slide();
}

double PlayerController::get_input_deadzone() const 
{
    return input_deadzone;
}
void PlayerController::set_input_deadzone(const double p_input_deadzone) 
{
    input_deadzone = p_input_deadzone;
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
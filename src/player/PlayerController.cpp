#include "PlayerController.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PlayerController::_bind_methods() 
{
    // Speed
    ClassDB::bind_method(D_METHOD("get_speed"), &PlayerController::get_max_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &PlayerController::set_max_speed);
    ClassDB::add_property("PlayerController", PropertyInfo(Variant::FLOAT, "max_speed", PROPERTY_HINT_RANGE, "100, 500, 1"), "set_speed", "get_speed");

    // Time Emit
    //ADD_SIGNAL(MethodInfo("position_changed", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::VECTOR2, "new_pos")));
}

PlayerController::PlayerController() 
{
    max_speed = 1.0;

    UtilityFunctions::print("Player instance created");
}

PlayerController::~PlayerController() 
{

}

void PlayerController::_process_input() 
{
    Input* in_singleton = Input::get_singleton();

    input_direction = in_singleton->get_vector(
        "move_left", "move_right", "move_forward", "move_back");

    if (!input_direction.is_zero_approx()) 
    {
        // Normalize input
        real_t l = input_direction.x * input_direction.x + input_direction.y * input_direction.y;
        if (l != 0) {
            l = Math::sqrt(l);
            input_direction.x /= l;
            input_direction.y /= l;
        }
    
        // Print Input
        UtilityFunctions::print("input_direction(", input_direction.x, ", ", input_direction.y, ")");
    }
}

void PlayerController::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    _process_input();
}

void PlayerController::_physics_process(double delta) 
{
    current_direction = Vector3(input_direction.x, 0, input_direction.y);
    current_velocity = current_direction * max_speed;

    set_velocity(current_velocity * (float)delta);
    move_and_slide();
}

double PlayerController::get_max_speed() const 
{
    return max_speed;
}

void PlayerController::set_max_speed(double p_speed) 
{
    max_speed = p_speed;
}
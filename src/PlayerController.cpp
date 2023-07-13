#include "PlayerController.h"
#include "Player.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>

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
    ClassDB::bind_method(D_METHOD("_on_player_state_changed", "p_state"), &PlayerController::_on_player_state_changed);
    ClassDB::bind_method(D_METHOD("_on_can_strike"), &PlayerController::_on_can_strike);

    // Signals
    ADD_SIGNAL(MethodInfo("player_served", 
        PropertyInfo(Variant::VECTOR3, "player_position"),
        PropertyInfo(Variant::VECTOR3, "aim_direction")));

    ADD_SIGNAL(MethodInfo("player_striked",
        PropertyInfo(Variant::VECTOR3, "p_to")));
}

PlayerController::PlayerController() 
{
    // Action Defaults
    strike_rate = 0.5;
    can_strike = true;

    // Movement Defaults
    movement_deadzone = 0.32;
    max_speed = 10.0f;
    max_acceleration = 45.0f;
    max_fall_acceleration = 60.0f;
    jump_impulse = 18.0f;
    default_face = 1;
}

void PlayerController::_ready() 
{
    // Disables component logic outside gameplay
    if (Engine::get_singleton()->is_editor_hint()) return;

    // TEMPORARY: may remove later

    // Get relevant attributes from parent node
    player_instance = (Player*)get_parent();
    player_instance->connect("player_state_changed", Callable(this, "_on_player_state_changed"));
    player_state = player_instance->get_player_state();
    player_id = player_instance->get_player_id();
    player_team = player_instance->get_player_team();

    // Component and Attribute Initializaton
    Node* controller_instance = player_instance->get_node<PlayerController>("PlayerController");
    input = Input::get_singleton();
    strike_area = controller_instance->get_node<Area3D>("StrikeArea");

    // Set default facing direction
    double player_face = player_team == 0 ? 0 : 3.1459;
    set_rotation(Vector3(0, player_face, 0));
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
    
    // Handle gravity regardless of state
    if (!is_on_floor()) 
    {
        target_velocity.y = current_velocity.y - (max_fall_acceleration * (float)delta);
    }

    handle_actions();

    // Look towards facing direction
    if (has_input)
    {   
        facing_direction = Vector3(input_direction.x, 0, input_direction.y);
        double angle = Math::atan2(-facing_direction.z, facing_direction.x);
        set_rotation(Vector3(0, angle, 0));
    }

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
    Vector3 serve_direction = get_basis()[0];
    emit_signal("player_served", get_global_position(), serve_direction); 

    // TODO: Add check if player succesfully served
    player_instance->set_player_state(PlayerState::Moving); 
}

void PlayerController::strike() 
{
    // Strike the ball if it is in range
    if (strike_area->has_overlapping_areas()) 
    {

        // Define court boundaries based on player team
        int court_adj = player_team == 0 ? -1 : 1;
        //double min_height = 6.0;
        Vector3 court_bounds = Vector3(12, 0, 5);
        
        // Check if player is facing net
        // TODO: Change this it's a dumb way of doing things
        if (get_basis()[0][0] > 0) 
        {
            court_adj = -court_adj;
        }
        
        // Select a random point in this area
        RandomNumberGenerator rng;
        float x_point = rng.randf_range(0, court_bounds.x * court_adj);
        //float y_point = rng.randf_range(min_height, court_bounds.y);
        float z_point = rng.randf_range(-court_bounds.z, court_bounds.z);
        rng.randomize();        
        Vector3 court_point = Vector3(x_point, 0, z_point);
        // Emit signal
        emit_signal("player_striked", court_point);
    }

    // Do a diving-strike if not
}
#include "Ball.h"
#include "Player.h"
#include "PlayerController.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/physics_direct_body_state3d.hpp>

using namespace godot;

void Ball::_bind_methods() 
{        
    if (Engine::get_singleton()->is_editor_hint()) return;

    ClassDB::bind_method(D_METHOD("_on_strike_area_entered", "area"), &Ball::_on_strike_area_entered);
    ClassDB::bind_method(D_METHOD("_on_detect_area_entered", "area"), &Ball::_on_detect_area_entered);
    ClassDB::bind_method(D_METHOD("_on_strike_area_exited", "area"), &Ball::_on_strike_area_exited);
    ClassDB::bind_method(D_METHOD("_on_player_striked", "p_to"), &Ball::_on_player_striked);

    ClassDB::bind_method(D_METHOD("get_serve_force"), &Ball::get_serve_force);
    ClassDB::bind_method(D_METHOD("set_serve_force", "p_force"), &Ball::set_serve_force);
    ClassDB::add_property("Ball", PropertyInfo(Variant::FLOAT, "serve_force"), "set_serve_force", "get_serve_force");

    ADD_SIGNAL(MethodInfo("team_scored", 
        PropertyInfo(Variant::INT, "p_team"),
        PropertyInfo(Variant::INT, "p_player")));

    ADD_SIGNAL(MethodInfo("target_position_changed",
        PropertyInfo(Variant::VECTOR3, "p_target")));
}

Ball::Ball() 
{
    serve_force = 12.0;
    max_height = 2.0;
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

// TODO: Change this logic
void Ball::serve(Vector3 p_position) 
{
    float target_height = 16.0;
    float gravity = get_gravity_scale() * 9.8;
    float velocity_y = Math::sqrt(2 * gravity * target_height);

    set_mass(0.28);
    set_gravity_scale(3);

    Vector3 velocity = Vector3(0, velocity_y, 0);

    apply_central_impulse(velocity);

    // Temporary
    Vector3 target_position = Vector3(get_global_position().x, target_height, get_global_position().z);
    emit_signal("target_position_changed", target_position);
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
    // TODO: change this
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

void Ball::_on_player_striked(Vector3 p_to) 
{
    //== Reset Rotation ==//
    set_rotation(Vector3(0, 0, 0));

    //== Local Variables ==//
    Vector3 forward = get_basis()[0]; // (1, 0, 0)
    Vector3 up = get_basis()[1]; // (0, 1, 0)

    //== Calculate Angle (yaw) from Ball to Court Point around Y-axis ==//    
    Vector3 displacement = p_to - get_global_position();
    float yaw_angle = -get_signed_angle(displacement, forward, up);   
    rotate_y(yaw_angle);
    
    // == Get Pitch and Speed of Ball ==//
    double height = 8.0;
    double pitch_angle = get_strike_pitch(displacement, up, height);
    double speed = get_strike_speed(displacement, up, height, pitch_angle);

    //== Apply Transformations to Ball ==//
    // Rotate by pitch angle
    rotate_z(-pitch_angle);

    // Get forward direction
    Vector3 basis = get_basis()[0];
    Vector3 direction = Vector3(basis.x, basis.y, -basis.z);

    // Update linear velocity
    set_linear_velocity(speed * direction);
}

double Ball::get_strike_pitch(Vector3 p_displacement, Vector3 p_up, double p_height) 
{
    //== Calculate pitch angle and launch speed ==//
    // Get y, projected x values of displacement vector
    double d_y = p_displacement.y;
    double d_x = project_on_plane(p_displacement, p_up).length();
    // Get gravity
    double g = get_gravity_scale() * 9.8;
    // Do some math
    double p1 = Math::sqrt(2 * g * p_height);
    double p2 = Math::sqrt(2 * g * (p_height - d_y));
    double t1 = (-p1 + p2) / -g;
    double t2 = (-p1 - p2) / -g;
    double t = Math::max(t1, t2);
    // Calculate pitch angle and return
    return Math::atan(p1 * t / d_x);
}

double Ball::get_strike_speed(Vector3 p_displacement, Vector3 p_up, double p_height, double p_angle) 
{
    double d_y = p_displacement.y;
    double d_x = project_on_plane(p_displacement, p_up).length();
    double g = get_gravity_scale() * 9.8;
    double p1 = Math::sqrt(2 * g * p_height);

    return p1 / Math::sin(p_angle);
}

Vector3 Ball::project_on_plane(Vector3 p_vector, Vector3 p_normal) 
{
    // Get the dot product of the given vector and plane normal
    double projection_length = p_vector.dot(p_normal);
    // Define a new vector that is the product of the normal and the projection length
    Vector3 plane_vector = p_normal * projection_length;
    // Subtract this value from the original vector and return the result
    return p_vector - plane_vector;
}

double Ball::get_signed_angle(Vector3 p_a, Vector3 p_b, Vector3 p_axis) 
{
    // Get input vectors aligned on same plane as p_axis
    Vector3 a_plane = project_on_plane(p_a, p_axis);
    Vector3 b_plane = project_on_plane(p_b, p_axis);

    // Normalize
    Vector3 a_plane_n = a_plane.normalized();
    Vector3 b_plane_n = b_plane.normalized();

    // Get the normalized cross product vector...
    Vector3 ab_cross = a_plane_n.cross(b_plane_n).normalized();
    // and use the dot product of that and p_axis to find the sign of the angle
    double sign = ab_cross.dot(p_axis);
    // Get the angle in radians
    double radian = Math::acos(a_plane_n.dot(b_plane_n));
    // Return signed angle
    return radian * sign;
}
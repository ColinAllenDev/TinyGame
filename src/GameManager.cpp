#include "GameManager.h"
#include "InputManager.h"
#include "Player.h"
#include "PlayerController.h"
#include "Ball.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <godot_cpp/classes/rigid_body3d.hpp>

using namespace godot;

void GameManager::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("_on_input_request_player_join", "p_id"), &GameManager::_on_input_request_player_join);
    ClassDB::bind_method(D_METHOD("_on_player_served", "p_position", "p_direction"), &GameManager::_on_player_served);
}

void GameManager::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Set main game scene
    game_scene = get_tree()->get_current_scene();

    // Get scene components
    input_manager = game_scene->get_node<InputManager>("InputManager");
    resource_loader = ResourceLoader::get_singleton();

    // Start game by awaiting players
    game_state = GameState::AwaitPlayers;

    // Add initital player to scene (TODO: make dynamic)
    add_player(0);

    // Connect listener to input signal
    input_manager->connect("input_request_player_join", Callable(this, "_on_input_request_player_join"));
}

void GameManager::add_player(int p_id) 
{
    // Code below this statement only valid during runtime
    if (Engine::get_singleton()->is_editor_hint()) return;

    Ref<PackedScene> player_scene = resource_loader->load("res://scenes/Player.tscn");
    Node* player_node = player_scene->instantiate();
    game_scene->call_deferred("add_child", player_node);

    Player* player_class = (Player*)player_node;
    player_class->set_player_id(p_id);
    players.insert(p_id, player_class);

    PlayerController* player_controller = (PlayerController*)player_node->get_child(0);
    player_controller->connect("player_served", Callable(this, "_on_player_served"));

    UtilityFunctions::print("Added Player: ", player_class->get_player_id());

    if (p_id == 0) {
        player_class->set_player_score(10);
    }

    UtilityFunctions::print("Player ", p_id, " Score: ", player_class->get_player_score());
}

void GameManager::remove_player(int p_id) 
{
}

void GameManager::_on_input_request_player_join(int p_id) 
{
    if (!players.has(p_id)) 
    {
        add_player(p_id);
    } else {
        UtilityFunctions::print("Player ", p_id ," already exists");
    }
}

void GameManager::_on_player_served(Vector3 p_position, Vector3 p_direction) 
{
    Ref<PackedScene> ball_scene = resource_loader->load("res://scenes/Ball.tscn");
    Node* ball_node = ball_scene->instantiate();
    game_scene->call_deferred("add_child", ball_node);

    Node3D* ball_node_3d = (Node3D*)ball_node;
    ball_node_3d->set_position(p_position + p_direction);

    // Temporary Code
    RigidBody3D* ball_rbody_3d = (RigidBody3D*)ball_node;
    ball_rbody_3d->apply_impulse(p_direction * 10.0f, p_position);
}

GameState GameManager::get_game_state() const 
{
    return game_state; 
}
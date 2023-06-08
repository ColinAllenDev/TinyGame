#include "GameManager.h"
#include "InputManager.h"
#include "Player.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>


using namespace godot;

void GameManager::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("_on_input_request_player_join", "p_id"), &GameManager::_on_input_request_player_join);
}

void GameManager::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Get scene components
    input_manager = get_tree()->get_current_scene()->get_node<InputManager>("InputManager");

    // Start game by awaiting players
    game_state = GameState::AwaitPlayers;

    // Connect to signals
    int8_t _ret = input_manager->connect("input_request_player_join", Callable(this, "_on_input_request_player_join"));
    if (_ret != 0) UtilityFunctions::print("Error {", _ret, "} connecting 'Input' signed '_on_input_request_player_join.");

    // Temp: add player to scene
    add_player(0);
}

void GameManager::add_player(int p_id) 
{
    // Code below this statement only valid during runtime
    if (Engine::get_singleton()->is_editor_hint()) return;

    ResourceLoader* resource_loader = ResourceLoader::get_singleton();
    Ref<PackedScene> player_scene = resource_loader->load("res://scenes/Player.tscn");
    Node* player_node = player_scene->instantiate();
    get_tree()->get_current_scene()->call_deferred("add_child", player_node);

    Player* player_class = (Player*)player_node;
    player_class->set_player_id(p_id);
    players.insert(p_id, player_class);

    UtilityFunctions::print("Added Player: ", player_class->get_player_id());
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

GameState GameManager::get_game_state() const 
{
    return game_state; 
}
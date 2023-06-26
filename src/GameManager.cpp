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
    ClassDB::bind_method(D_METHOD("_on_player_striked", "p_to", "p_force"), &GameManager::_on_player_striked);
    ClassDB::bind_method(D_METHOD("_on_team_scored", "p_team", "p_player"), &GameManager::_on_team_scored);
}

void GameManager::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Singletons
    resource_loader = ResourceLoader::get_singleton();

    // Get scene components
    game_scene = get_tree()->get_current_scene();
    input_manager = game_scene->get_node<InputManager>("InputManager");

    // Start game by awaiting players
    game_state = GameState::AwaitPlayers;

    // Initialize scores
    scores.insert(0, 0);
    scores.insert(1, 0);

    // Initialize spawn points
    init_spawns();

    // Add initital player to scene (TODO: make dynamic)
    add_player();

    // Connect listener to input signal
    input_manager->connect("input_request_player_join", Callable(this, "_on_input_request_player_join"));
}

void GameManager::init_spawns() 
{
    Node* spawns_node = game_scene->get_node<Node>("Spawns");
    for (int i = 0; i < spawns_node->get_child_count(); i++) 
    {
        Node3D* spawn = (Node3D*)spawns_node->get_child(i); 
        spawns.insert(i, spawn);
    }
}

void GameManager::add_player() 
{
    // Code below this statement only valid during runtime
    if (Engine::get_singleton()->is_editor_hint()) return;

    int player_id = players.size();

    Ref<PackedScene> player_scene = resource_loader->load("res://scenes/Player.tscn");
    Node* player_node = player_scene->instantiate();
    get_tree()->get_current_scene()->call_deferred("add_child", player_node);

    Player* player_class = (Player*)player_node;
    player_class->set_player_id(player_id);
    players.insert(player_id, player_class);

    int team = (player_id % 2) == 0 ? 0 : 1;
    player_class->set_player_team(team);
    player_class->set_player_score(0);
    // temporary
    player_class->set_player_state(PlayerState::Serving);

    PlayerController* player_controller = (PlayerController*)player_node->get_child(0);
    player_controller->set_global_position(spawns[player_id]->get_global_position());
    player_controller->connect("player_served", Callable(this, "_on_player_served"));
    player_controller->connect("player_striked", Callable(this, "_on_player_striked"));

    UtilityFunctions::print("Player: ", player_class->get_player_id(), " joined on Team ", player_class->get_player_team());
}

void GameManager::remove_player(int p_id) 
{
}

void GameManager::_on_input_request_player_join(int p_id) 
{
    if (!players.has(p_id)) 
    {
        add_player();
    } else {
        UtilityFunctions::print("Player ", p_id ," already exists");
    }
}

void GameManager::_on_player_served(Vector3 p_position, Vector3 p_direction) 
{
    Ref<PackedScene> ball_scene = resource_loader->load("res://scenes/Ball.tscn");
    Node* ball_node = ball_scene->instantiate();
    ball = (Ball*)ball_node;
    game_scene->call_deferred("add_child", ball_node);

    Node3D* ball_node_3d = (Node3D*)ball_node;
    ball_node_3d->set_global_position(p_position + p_direction);

    // Note: may need to change this to a signal
    ball->serve(p_position);
    ball->connect("team_scored", Callable(this, "_on_team_scored"));

    // Temporary: once the player has served, the match has started
    game_state = GameState::MatchInProgress;
}

void GameManager::_on_player_striked(Vector3 p_to, float p_force) 
{
    // Instantiate marker if not done already
    if (!game_scene->has_node("Marker")) 
    {
        Ref<PackedScene> marker_scene = resource_loader->load("res://scenes/Marker.tscn");
        Node* marker_node = marker_scene->instantiate();
        game_scene->call_deferred("add_child", marker_node);
        
        marker = (Node3D*) marker_node;
        marker->set_global_position(p_to);
    } else {
        // Position marker based on p_to
        marker->set_global_position(p_to);
    }   
}

void GameManager::_on_team_scored(int p_team, int p_player) 
{
    int current_score = scores.get(p_team);
    scores.insert(p_team, current_score + 1);

    //UtilityFunctions::print("Player ", p_player, " from Team ", p_team, " scored!");
    //UtilityFunctions::print("Team R: ", scores.get(0), " Team L: ", scores.get(1));
}

GameState GameManager::get_game_state() const 
{
    return game_state; 
}

int GameManager::get_team_score(int p_team) const 
{
    return scores.get(p_team);
}
#include "GameManager.h"
#include "InputManager.h"
#include "Player.h"
#include "PlayerController.h"
#include "Ball.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
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
    ClassDB::bind_method(D_METHOD("_on_player_striked", "p_to"), &GameManager::_on_player_striked);
    ClassDB::bind_method(D_METHOD("_on_team_scored", "p_team", "p_player"), &GameManager::_on_team_scored);
    ClassDB::bind_method(D_METHOD("reset_round"), &GameManager::reset_round);

    ADD_SIGNAL(MethodInfo("player_joined",
        PropertyInfo(Variant::OBJECT, "p_player"), 
        PropertyInfo(Variant::OBJECT, "p_controller")));
}

void GameManager::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Singletons
    resource_loader = ResourceLoader::get_singleton();

    // Get scene components
    game_scene = get_tree()->get_current_scene();
    input_manager = game_scene->get_node<InputManager>("InputManager");

    // Initialize Game
    init_game();
}

void GameManager::_process(double delta) 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Note: All this is for testing purposes
    Input* input = Input::get_singleton();
    if(input->is_key_pressed(Key::KEY_T)) 
    {
        start_round();
    }
}


void GameManager::init_game() 
{
    // Set Attributes
    round_count = 0;
    round_start_timer = 5.0;
    reset_timer = 5.0;

    // Start game by awaiting players
    game_state = GameState::AwaitPlayers;

    // Initialize   
    team_scores[0] = 0;
    team_scores[1] = 0;

    // Initialize spawn points
    init_spawns();

    // Add initital player to scene (TODO: make dynamic)
    add_player();

    // Connect listener to input signal
    input_manager->connect("input_request_player_join", Callable(this, "_on_input_request_player_join"));
}

void GameManager::init_spawns() 
{
    Ref<PackedScene> spawns_scene = resource_loader->load("res://scenes/Spawns.tscn");
    Node* spawns_node = spawns_scene->instantiate();

    int count = spawns_node->get_child_count();
    for (int i = 0, j = 0; i < count; i++) 
    {
        Node3D* spawn = (Node3D*) spawns_node->get_child(i);
        
        spawns[i % 2][j] = spawn->get_position();

        if (i % 2 == 1) j++;
    }
}

void GameManager::add_player() 
{
    // Code below this statement only valid during runtime
    if (Engine::get_singleton()->is_editor_hint()) return;

    int player_id = players.size();
    int player_team = player_id % 2;

    Ref<PackedScene> player_scene = resource_loader->load("res://scenes/Player.tscn");
    Node* player_node = player_scene->instantiate();
    get_tree()->get_current_scene()->call_deferred("add_child", player_node);
    
    Player* player_class = (Player*) player_node;
    player_class->set_player_id(player_id);
    player_class->set_player_team(player_team);
    player_class->set_player_state(PlayerState::Waiting);
    player_class->set_player_score(0);
    player_class->call_deferred("set_global_position", spawns[player_team][player_id]);

    PlayerController* player_controller = (PlayerController*)player_node->get_child(0);    
    player_controller->connect("player_served", Callable(this, "_on_player_served"));
    player_controller->connect("player_striked", Callable(this, "_on_player_striked"));

    players.insert(player_id, player_class);
    emit_signal("player_joined", player_class, player_controller);
    UtilityFunctions::print("Player: ", player_class->get_player_id(), " joined on Team ", player_class->get_player_team());
}

void GameManager::remove_player(int p_id) 
{
}

void GameManager::start_match() 
{
    // Disable players from joining match once it starts
    input_manager->disconnect("input_request_player_join", Callable(this, "_on_input_request_player_join"));

    // Emit signal
    emit_signal("match_started");

    // Start Round
    get_tree()->create_timer(round_start_timer, false)->connect("timeout", Callable(this, "start_round"));
}

void GameManager::start_round() 
{
    // Set initial spawns
    set_spawns(players[0]);

    // Set serving player
    players[0]->set_player_state(PlayerState::Serving);
}

void GameManager::reset_round() 
{
    // Remove leftover objects
    ball->queue_free();
    marker->queue_free();

    // Reset spawns with player who scored last having serve
    set_spawns(player_scored);

}


void GameManager::set_spawns(Player* p_serving) 
{
    int round_mod = round_count % 2;
    for (auto [id, player] : players) 
    {
        int team = player->get_player_team();
        if (player == p_serving) 
        {
            player->set_spawn_point(spawns[team][round_mod]);
            player->respawn();
        }
        else
        {
            // Note: I try to avoid using hardcoded values. 2 = (spawns[team].length() / 2)
            player->set_spawn_point(spawns[team][2 + round_mod]); 
            player->respawn();
        }
    }
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
    game_scene->call_deferred("add_child", ball_node);

    Node3D* ball_node_3d = (Node3D*) ball_node;
    ball_node_3d->call_deferred("set_global_position", p_position + p_direction);

    // Note: may need to change this to a signal
    ball = (Ball*) ball_node;
    ball->serve(p_position);
    ball->connect("team_scored", Callable(this, "_on_team_scored"));

    // Temporary: once the player has served, the match has started
    game_state = GameState::MatchInProgress;
}

void GameManager::_on_player_striked(Vector3 p_to) 
{
    // Instantiate marker if not done already
    if (!game_scene->has_node("Marker")) 
    {
        Ref<PackedScene> marker_scene = resource_loader->load("res://scenes/Marker.tscn");
        Node* marker_node = marker_scene->instantiate();
        game_scene->call_deferred("add_child", marker_node);
        
        marker = (Node3D*) marker_node;
        marker->call_deferred("set_global_position", p_to + Vector3(0, 0.1, 0));
    } else {
        // Position marker based on p_to
        marker->set_global_position(p_to);
    }   
}

void GameManager::_on_team_scored(int p_team, int p_player) 
{
    int current_score = team_scores[p_team];
    team_scores[p_team] = current_score + 1;

    for (auto [id, player] : players) 
    {
        player->set_player_state(PlayerState::Waiting);
    }

    player_scored = players[p_player];
    emit_signal("player_scored", player_scored, p_team);

    get_tree()->create_timer(reset_timer, false)->connect("timeout", Callable(this, "reset_round"));

    UtilityFunctions::print("Player ", p_player, " from Team ", p_team, " scored!");
    UtilityFunctions::print("Team R: ", team_scores[0], " Team L: ", team_scores[1]);
}
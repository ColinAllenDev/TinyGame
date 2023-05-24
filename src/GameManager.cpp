#include "GameManager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

void GameManager::_bind_methods() 
{
}

void GameManager::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    Node3D* node = memnew(Node3D);
    node->set_name("TestNode");
    add_child(node);
    node->set_owner(this);
}

void GameManager::add_player(int player_index) 
{
    num_players++;
}

void GameManager::remove_player(int player_index) 
{
    num_players--;
}
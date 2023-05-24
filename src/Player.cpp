#include "Player.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/string_name.hpp>

using namespace godot;

void Player::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("get_player_id"), &Player::get_player_id);
    ClassDB::bind_method(D_METHOD("get_device_id"), &Player::get_device_id);
    ClassDB::bind_method(D_METHOD("set_device_id", "p_device_id"), &Player::set_device_id);
}

void Player::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    set_player_state(PlayerState::Serving);
}

PlayerState Player::get_player_state() const 
{
    return player_state;
}

void Player::set_player_state(PlayerState p_state) 
{
    player_state = p_state;
}

int Player::get_player_id() const 
{
    return player_id;
}

int Player::get_device_id() const 
{
    return device_id;
}

void Player::set_device_id(const int p_device_id) 
{
    device_id = p_device_id;
}
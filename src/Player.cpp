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
    ClassDB::bind_method(D_METHOD("get_player_score"), &Player::get_player_score);
    ClassDB::bind_method(D_METHOD("set_player_score", "p_score"), &Player::set_player_score);

    ADD_SIGNAL(MethodInfo("player_state_changed", PropertyInfo(Variant::INT, "p_state")));
}

Player::Player() 
{
    score = 0;
    // Temporary: initial state will be idle or moving
    player_state = PlayerState::Serving;
}

void Player::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    UtilityFunctions::print("Player ", player_id, " joined the game!");
}

PlayerState Player::get_player_state() const 
{
    return player_state;
}

void Player::set_player_state(PlayerState p_state) 
{
    player_state = p_state;
    emit_signal("player_state_changed", (int)p_state);
}

int Player::get_player_id() const 
{
    return player_id;
}

void Player::set_player_id(const int p_id) 
{
    player_id = p_id;
}

int Player::get_device_id() const 
{
    return device_id;
}

void Player::set_device_id(const int p_device_id) 
{
    device_id = p_device_id;
}

int Player::get_player_score() const 
{
    return score;
}

void Player::set_player_score(const int p_score) 
{
    score = p_score;
}
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

void Player::_ready() 
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    score = 0;

    UtilityFunctions::print("Player ", player_id, " joined the game!");
}

void Player::set_player_state(PlayerState p_state) 
{
    player_state = p_state;
    emit_signal("player_state_changed", (int)p_state);
}
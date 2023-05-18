#include "GameManager.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

void GameManager::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("get_input_deadzone"), &GameManager::get_input_deadzone);
    ClassDB::bind_method(D_METHOD("set_input_deadzone", "p_deadzone"), &GameManager::set_input_deadzone);
    ClassDB::add_property("GameManager", PropertyInfo(Variant::FLOAT, "input_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_input_deadzone", "get_input_deadzone");

    // Reminder here to always bind your methods if you need to reference them (in my case using the input->connect method)
    ClassDB::bind_method(D_METHOD("_on_joy_connection_changed", "device", "connected"), &GameManager::_on_joy_connection_changed);
}

GameManager::GameManager() 
{
    // REMEMBER THIS IS HERE THIS COULD CAUSE SOME FUCKY SHIT (potentially)
    if (Engine::get_singleton()->is_editor_hint()) return;

    // Input Defaults
    input = Input::get_singleton();
    input_deadzone = 0.1;

    // Connect 'joy_connection_changed' signal to _on_joy_connection_changed method
    int8_t _ret = input->connect("joy_connection_changed", Callable(this, "_on_joy_connection_changed"));
    if (_ret != 0) UtilityFunctions::print("Error {", _ret, "} connecting 'Input' signed '_on_joy_connection_changed.");
}

GameManager::~GameManager() 
{
}

void GameManager::_process(double delta) 
{

}

void GameManager::_input(const Ref<InputEvent> &event) 
{
    // Handle first input device received
    // Note: I don't know if I want this to be final way of doing this as it seems kinda hacky but I can't seem to find
    // a more elegant solution when it comes to listening for the first input device between keyboards and joypads.
    if (event->is_pressed() && input_devices.is_empty()) 
    {
        UtilityFunctions::print("Input Device of Pressed Input: ", event->get_device());
    }
}

void GameManager::_on_joy_connection_changed(int device, bool connected) 
{
    // Print a message when an input device is connected/disconnected.
    String connected_str = connected ? "connected" : "disconnected";
    UtilityFunctions::print("Device: {", device, "} has been ", connected_str, "!");

    // If this device is not contained in our list of input_devices, add it
    if (!input_devices.has(device) && connected == true) 
    {
        input_devices.append(device);

        // Add player to game with this device
        add_player(device);
    } else if (input_devices.has(device) && connected == false) {
        // If this device is already in our list of input devices and has been disconnected, remove it
        input_devices.erase(device);

        // Remove player corresponding to this device
        remove_player(device);
    }
    
    UtilityFunctions::print("Current number of devices: ", input_devices.size());

}

void GameManager::add_player(uint8_t player_index) 
{
    UtilityFunctions::print("Player ", player_index, " joined the game!");

    num_players++;
}

void GameManager::remove_player(uint8_t player_index) 
{
    UtilityFunctions::print("Player ", player_index, " left the game!");

    num_players--;
}

double GameManager::get_input_deadzone() const 
{
    return input_deadzone;
}

void GameManager::set_input_deadzone(const double p_deadzone) 
{
    input_deadzone = p_deadzone;
}
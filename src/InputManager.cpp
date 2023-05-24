#include "InputManager.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

void InputManager::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("get_input_deadzone"), &InputManager::get_input_deadzone);
    ClassDB::bind_method(D_METHOD("set_input_deadzone", "p_deadzone"), &InputManager::set_input_deadzone);
    ClassDB::add_property("InputManager", PropertyInfo(Variant::FLOAT, "input_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_input_deadzone", "get_input_deadzone");

    // Reminder here to always bind your methods if you need to reference them (in my case using the input->connect method)
    ClassDB::bind_method(D_METHOD("_on_joy_connection_changed", "device", "connected"), &InputManager::_on_joy_connection_changed);
}

InputManager::InputManager() 
{
    // Set defaults
    input_deadzone = 0.1;

    // Initialize input singleton pointer
    input = Input::get_singleton();
    input_map = InputMap::get_singleton();

    // For now, the keyboard is going to be considered the default device and will be associated with the ID 0.
    add_device({0, DeviceType::Keyboard, "Keyboard (default)"});

    // Get current number of joypads connected
    for (int device_id = 1; device_id <= input->get_connected_joypads().size(); device_id++) 
    {
        add_device({device_id, DeviceType::Joypad, "Joypad " + device_id});
    }

    // Connect 'joy_connection_changed' signal to _on_joy_connection_changed method
    int8_t _ret = input->connect("joy_connection_changed", Callable(this, "_on_joy_connection_changed"));
    if (_ret != 0) UtilityFunctions::print("Error {", _ret, "} connecting 'Input' signed '_on_joy_connection_changed.");

    // Print current number of devices
    if (Engine::get_singleton()->is_editor_hint()) return;
    UtilityFunctions::print("InputManager Initialized with ", input_devices.size(), " devices.");
}

InputManager::~InputManager() 
{
}

void InputManager::_on_joy_connection_changed(int device, bool connected) 
{   
    String connected_str = connected ? "connected" : "disconnected";
    int device_id = device + 1;
    InputDevice d_joypad = {device_id, DeviceType::Joypad, "Joypad " + device_id};

    if (connected == true && !is_device_connected(device_id)) {
        add_device(d_joypad);
    }

    if (connected == false && is_device_connected(device_id)) {
        remove_device(device_id);
    }

    UtilityFunctions::print("Joypad: {", device_id, "} has been ", connected_str, "!");
    UtilityFunctions::print("Current Device Count: ", input_devices.size());
}

bool InputManager::is_device_connected(int device_id) 
{
    for (auto device : input_devices) 
    {
        if (device.id == device_id) return true;
    }
    return false;
}

void InputManager::_input(Ref<InputEvent> event) 
{
    if (event->is_pressed()) {
        UtilityFunctions::print("InputMap Length: ", input_map->get_actions().size());
    }
}

void InputManager::add_device(InputDevice device) 
{
    // Add to input_devices list
    input_devices.append(device);

    // Input Mapping
    // TODO: Definetly refactor this
    input_map->add_action("move_right_"+device.id, input_deadzone);
    input_map->add_action("move_left_"+device.id, input_deadzone);
    input_map->add_action("move_up_"+device.id, input_deadzone);
    input_map->add_action("move_down_"+device.id, input_deadzone);
}

void InputManager::remove_device(int device_id) 
{
    // Remove device from input_devices list
    int i = 0;
    for (auto device : input_devices) 
    {
        if (device.id == device_id) 
        {
            input_devices.remove_at(i);
            return;
        }
        i++;
    }

    // Input Mapping
    // TODO: Definetly refactor this
    input_map->erase_action("move_right_"+device_id);
    input_map->erase_action("move_left_"+device_id);
    input_map->erase_action("move_up_"+device_id);
    input_map->erase_action("move_down_"+device_id);
}

double InputManager::get_input_deadzone() const
{
    return input_deadzone;
}

void InputManager::set_input_deadzone(const double p_deadzone) 
{
    input_deadzone = p_deadzone;
}
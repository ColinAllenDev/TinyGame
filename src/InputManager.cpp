#include "InputManager.h"
#include "GameManager.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

void InputManager::_bind_methods() 
{
    ClassDB::bind_method(D_METHOD("get_input_deadzone"), &InputManager::get_input_deadzone);
    ClassDB::bind_method(D_METHOD("set_input_deadzone", "p_deadzone"), &InputManager::set_input_deadzone);
    ClassDB::add_property("InputManager", PropertyInfo(Variant::FLOAT, "input_deadzone", PROPERTY_HINT_RANGE, "0.1, 1, 0.01"), "set_input_deadzone", "get_input_deadzone");

    // Reminder here to always bind your methods if you need to reference them (in my case using the input->connect method)
    ClassDB::bind_method(D_METHOD("_on_joy_connection_changed", "device", "connected"), &InputManager::_on_joy_connection_changed);
    
    // Signals
    ADD_SIGNAL(MethodInfo("input_request_player_join", PropertyInfo(Variant::INT, "p_id")));
}

InputManager::InputManager() 
{

} 

void InputManager::_ready() 
{
    /// RUNTIME LOGIC ///
    if (Engine::get_singleton()->is_editor_hint()) return;
    
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
    
    UtilityFunctions::print("InputManager Initialized with ", input_devices.size(), " devices.");

    game_manager = get_tree()->get_current_scene()->get_node<GameManager>("GameManager");
}

void InputManager::_input(Ref<InputEvent> event) 
{
    int p_id = event->get_device();
    if (event->get_class() == "InputEventJoypadButton") p_id++;

    if (event->is_action_pressed("start_"+String::num_int64(p_id))) 
    {
        if (game_manager->get_game_state() == GameState::AwaitPlayers) 
        {
            input_request_player_join(p_id);
        }
    }
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

void InputManager::add_device(InputDevice device) 
{
    // Add to input_devices list
    input_devices.append(device);

    // Input Mapping
    if (device.type == DeviceType::Joypad)
    {
        init_joypad_map(device);
    }
    else
    {
        init_keyboard_map(device);
    }
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
    UtilityFunctions::print("Removing device: ", device_id);
    input_map->erase_action("move_right_"+String::num_int64(device_id));
    input_map->erase_action("move_left_"+String::num_int64(device_id));
    input_map->erase_action("move_up_"+String::num_int64(device_id));
    input_map->erase_action("move_down_"+String::num_int64(device_id));
    input_map->erase_action("jump_"+String::num_int64(device_id));
}

HashMap<String, int> InputManager::init_joypad_map(InputDevice device) 
{
    HashMap<String, int> _map;
    int _id = device.id;

    // TODO : Refactor all this later
    auto* jpm_right = new InputEventJoypadMotion();
    _map.insert("move_right_"+String::num_int64(_id), {JOY_AXIS_LEFT_X});
    input_map->add_action("move_right_"+String::num_int64(_id));
    jpm_right->set_axis(JOY_AXIS_LEFT_X);
    jpm_right->set_axis_value(+1);
    input_map->action_add_event("move_right_"+String::num_int64(_id), jpm_right);
    
    auto* jpm_left = new InputEventJoypadMotion();
    _map.insert("move_left_"+String::num_int64(_id), {JOY_AXIS_LEFT_X});
    input_map->add_action("move_left_"+String::num_int64(_id));
    jpm_left->set_axis(JOY_AXIS_LEFT_X);
    jpm_left->set_axis_value(-1);
    input_map->action_add_event("move_left_"+String::num_int64(_id), jpm_left);
    
    auto* jpm_up = new InputEventJoypadMotion();
    _map.insert("move_up_"+String::num_int64(_id), {JOY_AXIS_LEFT_Y});
    input_map->add_action("move_up_"+String::num_int64(_id));
    jpm_up->set_axis(JOY_AXIS_LEFT_Y);
    jpm_up->set_axis_value(-1);
    input_map->action_add_event("move_up_"+String::num_int64(_id), jpm_up);
    
    auto* jpm_down = new InputEventJoypadMotion();
    _map.insert("move_down_"+String::num_int64(_id), {JOY_AXIS_LEFT_Y});
    input_map->add_action("move_down_"+String::num_int64(_id));
    jpm_down->set_axis(JOY_AXIS_LEFT_Y);
    jpm_down->set_axis_value(+1);
    input_map->action_add_event("move_down_"+String::num_int64(_id), jpm_down);
    
    auto* jpb_jump = new InputEventJoypadButton();
    _map.insert("jump_"+String::num_int64(_id), JOY_BUTTON_A);
    input_map->add_action("jump_"+String::num_int64(_id));
    jpb_jump->set_button_index(JOY_BUTTON_A);
    input_map->action_add_event("jump_"+String::num_int64(_id), jpb_jump);

    auto* jpb_start = new InputEventJoypadButton();
    _map.insert("start_"+String::num_int64(_id), JOY_BUTTON_START);
    input_map->add_action("start_"+String::num_int64(_id));
    jpb_start->set_button_index(JOY_BUTTON_START);
    input_map->action_add_event("start_"+String::num_int64(_id), jpb_start);

    auto* jpb_strike = new InputEventJoypadButton();
    _map.insert("strike_"+String::num_int64(_id), JOY_BUTTON_X);
    input_map->add_action("strike_"+String::num_int64(_id));
    jpb_strike->set_button_index(JOY_BUTTON_X);
    input_map->action_add_event("strike_"+String::num_int64(_id), jpb_strike);

    return _map;
}

HashMap<String, int> InputManager::init_keyboard_map(InputDevice device) 
{
    HashMap<String, int> _map;
    int _id = device.id;

    
    auto* kb_right = new InputEventKey();
    _map.insert("move_right_"+String::num_int64(_id), {KEY_D});
    input_map->add_action("move_right_"+String::num_int64(_id));
    kb_right->set_keycode(KEY_D);
    input_map->action_add_event("move_right_"+String::num_int64(_id), kb_right);
    
    auto* kb_left = new InputEventKey();
    _map.insert("move_left_"+String::num_int64(_id), {KEY_A});
    input_map->add_action("move_left_"+String::num_int64(_id));
    kb_left->set_keycode(KEY_A);
    input_map->action_add_event("move_left_"+String::num_int64(_id), kb_left);
    
    auto* kb_up = new InputEventKey();
    _map.insert("move_up_"+String::num_int64(_id), {KEY_W});
    input_map->add_action("move_up_"+String::num_int64(_id));
    kb_up->set_keycode(KEY_W);
    input_map->action_add_event("move_up_"+String::num_int64(_id), kb_up);
    
    auto* kb_down = new InputEventKey();
    _map.insert("move_down_"+String::num_int64(_id), {KEY_S});
    input_map->add_action("move_down_"+String::num_int64(_id));
    kb_down->set_keycode(KEY_S);
    input_map->action_add_event("move_down_"+String::num_int64(_id), kb_down);
    
    auto* kb_jump = new InputEventKey();
    _map.insert("jump_"+String::num_int64(_id), KEY_SPACE);
    input_map->add_action("jump_"+String::num_int64(_id));
    kb_jump->set_keycode(KEY_SPACE);
    input_map->action_add_event("jump_"+String::num_int64(_id), kb_jump);

    auto* kb_start = new InputEventKey();
    _map.insert("start_"+String::num_int64(_id), KEY_ENTER);
    input_map->add_action("start_"+String::num_int64(_id));
    kb_start->set_keycode(KEY_ENTER);
    input_map->action_add_event("start_"+String::num_int64(_id), kb_start);

    auto* kb_strike = new InputEventKey();
    _map.insert("strike_"+String::num_int64(_id), KEY_F);
    input_map->add_action("strike_"+String::num_int64(_id));
    kb_strike->set_keycode(KEY_F);
    input_map->action_add_event("strike_"+String::num_int64(_id), kb_strike);

    return _map;
}

void InputManager::input_request_player_join(int p_id) 
{
    emit_signal("input_request_player_join", p_id);
}

double InputManager::get_input_deadzone() const
{
    return input_deadzone;
}

void InputManager::set_input_deadzone(const double p_deadzone) 
{
    input_deadzone = p_deadzone;
}
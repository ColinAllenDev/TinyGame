#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot 
{
    class Input;
    class InputMap;
    class GameManager;

    class InputManager : public Node 
    {
        GDCLASS(InputManager, Node)
    private:
        // Components and Singletons
        Input* input;
        InputMap* input_map;
        GameManager* game_manager;

        // Input Device data structure
        enum DeviceType {Keyboard, Joypad};
        struct InputDevice 
        {
            int id;
            DeviceType type;
            String name;
        };
    
        // List of currently active input devices
        Vector<InputDevice> input_devices;
        
        // Input Properties
        double input_deadzone;
    protected:
        static void _bind_methods();
    public:
        InputManager() {}
        ~InputManager() {}

        void _ready();
        void _input(Ref<InputEvent> event);
        
        void _on_joy_connection_changed(int device, bool connected);
        bool is_device_connected(int device_id);

        void add_device(InputDevice device);
        void remove_device(int device_id);

        HashMap<String, int> init_joypad_map(InputDevice device);
        HashMap<String, int> init_keyboard_map(InputDevice device);

        // Signals
        void input_request_player_join(int p_id);

        // Getters and Setters
        double get_input_deadzone() const;
        void set_input_deadzone(const double p_deadzone);
    };
}

#endif
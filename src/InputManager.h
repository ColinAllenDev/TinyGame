#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot 
{
    class Input;
    class InputMap;

    class InputManager : public Node 
    {
        GDCLASS(InputManager, Node)
    private:
        // Singletons
        Input* input;
        InputMap* input_map;

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
        
        double input_deadzone;
        bool is_first_input;
    protected:
        static void _bind_methods();
    public:
        InputManager();
        ~InputManager();

        void _on_joy_connection_changed(int device, bool connected);
        bool is_device_connected(int device_id);

        void _input(Ref<InputEvent> event);

        void add_device(InputDevice device);
        void remove_device(int device_id);

        // Getters and Setters
        double get_input_deadzone() const;
        void set_input_deadzone(const double p_deadzone);
    };
}

#endif
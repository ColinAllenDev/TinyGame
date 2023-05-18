#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot 
{
    class Input;

    class GameManager : public Node3D
    {
        GDCLASS(GameManager, Node3D)
    private:
        // Player
        uint8_t num_players;

        // Input
        Input* input;
        Vector<int> input_devices;
        double input_deadzone;
        double movement_deadzone;
        Vector2 input_direction;
    protected:
        static void _bind_methods();
    public:
        GameManager();
        ~GameManager();

        void _process(double delta);
        void _process_input();
        void _input(const Ref<InputEvent> &event);

        void _on_joy_connection_changed(int device, bool connected);

        void add_player(uint8_t player_index);
        void remove_player(uint8_t player_index);        

        double get_input_deadzone() const;
        void set_input_deadzone(const double p_deadzone);
    };
}

#endif // GAMEMANAGER_H
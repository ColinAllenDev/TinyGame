#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "InputManager.h"
#include "GameManager.h"
#include "Player.h"
#include "PlayerController.h"
#include "CameraController.h"
#include "Ball.h"

using namespace godot;

void initialize_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;

    ClassDB::register_class<InputManager>();
    ClassDB::register_class<GameManager>();
    ClassDB::register_class<Player>();
    ClassDB::register_class<PlayerController>();
    ClassDB::register_class<CameraController>();
    ClassDB::register_class<Ball>();
}

void uninitialize_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C"
{
    GDExtensionBool GDE_EXPORT library_init(const GDExtensionInterface* p_interface,
        const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) 
    {
        godot::GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

        init_obj.register_initializer(initialize_module);
        init_obj.register_terminator(uninitialize_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
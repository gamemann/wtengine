/*!
 * wtengine | File:  engine.cpp
 * 
 * \author Matthew Evans
 * \version 0.7.2
 * \copyright See LICENSE.md for copyright information.
 * \date 2019-2022
 */

#include "wtengine/engine.hpp"

namespace wte {

commands engine::cmds;
ALLEGRO_TIMER* engine::main_timer = NULL;
ALLEGRO_EVENT_QUEUE* engine::main_event_queue = NULL;
std::vector<std::string> engine::file_locations;
bool engine::initialized = false;

/*
 *
 */
engine::engine(const int& argc, char** const& argv) {
    std::cout << "Starting wtengine... ";
    if(initialized == true) throw runtime_error(
        exception_item(display::window_title + " already running!", "Main engine", 1));
    initialized = true;
    std::cout << "OK!\n";

    std::cout << "Loading Allegro Game Library... ";
    //  Initialize Allegro.
    if(!al_init()) throw runtime_error(
        exception_item("Allegro failed to load!", "Main engine", 1));
    std::cout << "OK!\n";

    //  Initialize additional Allegro components.
    std::cout << "Loading Allegro add-ons... ";
    if(!al_init_image_addon()) throw runtime_error(
        exception_item("Failed to load Allegro image addon!", "Main engine", 1));
    if(!al_init_font_addon()) throw runtime_error(
        exception_item("Failed to load Allegro font addon!", "Main engine", 1));
    std::cout << "OK!\n";
    config::_flags::audio_installed = al_install_audio();
    //  Input detection.
    config::_flags::keyboard_detected = al_install_keyboard();
    config::_flags::mouse_detected = al_install_mouse();
    config::_flags::joystick_detected = al_install_joystick();
    config::_flags::touch_detected = al_install_touch_input();

    //  Configure PhysFS.
    std::cout << "Loading PhysicsFS... ";
    if(!PHYSFS_init(argv[0])) throw runtime_error(
        exception_item("Failed to load PhysicsFS!", "Main engine", 1));
    if(file_locations.empty()) throw runtime_error(
        exception_item("Need to configure locations for PhysFS!", "Main engine", 1));
    for(auto& it: file_locations) PHYSFS_mount(it.c_str(), NULL, 1);
    al_set_physfs_file_interface();
    std::cout << "OK!\n";

    //  Configure display.  Called from wte_display class.
    std::cout << "Configuring display... ";
    create_display();
    std::cout << "OK!\n";

    //  Disable pesky screensavers.
    al_inhibit_screensaver(true);

    std::cout << "Creating main timer and event queue... ";
    //  Configure main timer.
    main_timer = al_create_timer(1.0f / ticks_per_sec);
    if(!main_timer) throw runtime_error(
        exception_item("Failed to create timer!", "Main engine", 1));
    //  Configure main event queue.
    main_event_queue = al_create_event_queue();
    if(!main_event_queue) throw runtime_error(
        exception_item("Failed to create main event queue!", "Main engine", 1));
    std::cout << "OK!\n";

    //  Register event sources.
    al_register_event_source(main_event_queue, al_get_display_event_source(_display));
    al_register_event_source(main_event_queue, al_get_timer_event_source(main_timer));

    input::create_event_queue();

    //  commands
    cmds.add("exit", 0, [this](const msg_args& args) {
        if(config::flags::game_started) process_end_game();
        config::_flags::is_running = false;
    });
    cmds.add("new-game", 1, [this](const msg_args& args) {
        if(!config::flags::game_started) {
            process_new_game(args[0]);
        }
    });
    cmds.add("end-game", 0, [this](const msg_args& args) {
        if(config::flags::game_started) {
            process_end_game();
        }
    });
    cmds.add("fps-counter", 1, [this](const msg_args& args) {
        if(args[0] == "on") config::flags::draw_fps = true;
        if(args[0] == "off") config::flags::draw_fps = false;
    });
    cmds.add("load-script", 1, [this](const msg_args& args) {
        if(config::flags::game_started && args[0] != "") {
            try {
                if(!mgr::messages::load_script(args[0]))
                    throw exception(exception_item("Error loading script:  " + args[0], "engine", 2));
            } catch(const exception& e) { throw e; }
        }
    });

    if(build_options.debug_mode) logger::start();
}

/*
 *
 */
engine::~engine() {
    std::cout << "Stopping wtengine... ";
    PHYSFS_deinit();

    al_destroy_timer(main_timer);
    al_destroy_event_queue(main_event_queue);
    input::destroy_event_queue();
    destroy_display();
    al_inhibit_screensaver(false);
    al_uninstall_system();

    if(build_options.debug_mode) logger::stop();

    initialized = false;
    std::cout << "Done!\n\n";
}

/*
 *
 */
void engine::add_file_location(const std::string& flocation) {
    file_locations.push_back(flocation);
}

/*
 *
 */
void engine::wte_load(void) {
    //  Generate Allegro's default font and load into asset mgr.
    mgr::assets<al_font>::load<al_font>("wte_default_font", make_asset(al_font()));

    //  Initialize managers that require it.
    mgr::audio::initialize();
    mgr::gfx::renderer::initialize();
}

/*
 *
 */
void engine::wte_unload(void) {
    mgr::audio::de_init();
    mgr::gfx::renderer::de_init();
}

/*
 *
 */
void engine::process_new_game(const std::string& game_data) {
    std::srand(std::time(nullptr));  //  Seed random, using time.

    //  Make sure the menu isn't opened.
    config::_flags::menu_opened = false;

    //  Load a new message data file.
    if(!game_data.empty()) mgr::messages::load_file(game_data);

    //  Load systems and prevent further systems from being loaded.
    load_systems();
    mgr::systems::finalized = true;
    if(mgr::systems::empty()) throw runtime_error(
        exception_item("No systems have been loaded!", "Main Engine", 1));

    //  Stop audio manager from playing sounds.
    mgr::audio::music::a::stop();
    mgr::audio::music::b::stop();
    mgr::audio::ambiance::stop();
    mgr::audio::voice::stop();
    mgr::audio::sample::clear_instances();
    
    //  Clear world and load starting entities.
    mgr::world::clear();
    
    try { new_game(); } catch(exception& e) {
        //  Failed to create new game, abort.
        config::_flags::menu_opened = true;
        throw e;
        return;
    }

    //  Restart the timer at zero.
    al_stop_timer(main_timer);
    al_set_timer_count(main_timer, 0);
    engine_time::set(al_get_timer_count(main_timer));
    config::_flags::game_started = true;
    config::flags::input_enabled = true;
    al_start_timer(main_timer);
}

/*
 *
 */
void engine::process_end_game(void) {
    al_stop_timer(main_timer);
    config::_flags::game_started = false;
    config::flags::input_enabled = true;
    al_set_timer_count(main_timer, 0);
    engine_time::set(al_get_timer_count(main_timer));

    //  Stop audio manager from playing sounds.
    mgr::audio::music::a::stop();
    mgr::audio::music::b::stop();
    mgr::audio::ambiance::stop();
    mgr::audio::voice::stop();
    mgr::audio::sample::clear_instances();

    //  Call end game process.
    try { end_game(); } catch(const exception& e) { throw e; }
    //  Clear managers.
    mgr::world::clear();
    mgr::systems::clear();
    mgr::messages::clear();

    //  Open the menus.
    config::_flags::menu_opened = true;
}

/*
 *
 */
void engine::do_game(void) {
    wte_load();

    //  Set default states.
    config::_flags::is_running = true;
    config::_flags::game_started = false;
    config::_flags::menu_opened = true;

    while(config::flags::is_running) {
        /* *** START ENGINE LOOP ******************************************** */
        input::check_events();  //  Check for input.

        if(!config::flags::game_started) {       //  Game not running.
            al_stop_timer(main_timer);           //  Make sure the timer isn't.
            config::_flags::menu_opened = true;  //  And force menus
        }
        //  Pause / resume timer depending on if the game menu is opened.
        //  Also process the on_menu events.
        if(config::flags::menu_opened && al_get_timer_started(main_timer)) {
            al_stop_timer(main_timer);
            on_menu_open();
        }
        if(!config::flags::menu_opened && !al_get_timer_started(main_timer)) {
            on_menu_close();
            al_resume_timer(main_timer);
        }

        ALLEGRO_EVENT event;
        if(al_get_next_event(main_event_queue, &event)) {
            switch(event.type) {
            //  Call our game logic update on timer events.
            //  Timer is only running when the game is running.
            case ALLEGRO_EVENT_TIMER:
                //  Set the engine_time object to the current time.
                engine_time::set(al_get_timer_count(main_timer));
                //  Run all systems.
                mgr::systems::run();
                //  Process messages.
                mgr::messages::dispatch();
                //  Get any spawner messages and pass to handler.
                mgr::spawner::process_messages(mgr::messages::get("spawner"));
                break;
            //  Check if display looses focus.
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                out_of_focus();
                break;
            //  Check if display returns to focus.
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
                back_in_focus();
                break;
            //  Force quit if the game window is closed.
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                if(config::flags::game_started) process_end_game();
                config::_flags::is_running = false;
                break;
            //  Window has been resized.
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                //
                break;
            }
        }

        //  Get any system messages and pass to handler.
        cmds.process_messages(mgr::messages::get("system"));
        //  Send audio messages to the audio queue.
        mgr::audio::process_messages(mgr::messages::get("audio"));

        mgr::systems::run_untimed();   //  Run any untimed systems.
        mgr::gfx::renderer::render();  //  Render the screen.
        mgr::messages::prune();        //  Delete unprocessed messages.
        /* *** END ENGINE LOOP ********************************************** */
    }

    wte_unload();
}

}  //  end namespace wte

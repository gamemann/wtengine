/*
  WTEngine Demo
  By:  Matthew Evans
  File:  main.cpp

  See LICENSE.txt for copyright information

  Main file for WTEngine demo
*/

#include "include/wte/wte.hpp"

#include "include/main.hpp"
#include "include/game_loop.hpp"

const unsigned int wte::screen_width = 768;
const unsigned int wte::screen_height = 1024;

const float wte::TICKS_PER_SECOND = 60.0;

int main(int argc, char **argv) {
    //  Set some default values for the game flags
    wte::game_flag[wte::DRAW_HITBOX] = true;
    wte::game_flag[wte::DRAW_FPS] = true;

    //  Initialize game object
    game_loop main_game_loop;
    main_game_loop.wte_init();

    //  Run the game loop
    main_game_loop.do_game();

    //  Clean up
    main_game_loop.wte_unload();

    return 0; //  Exit program
}

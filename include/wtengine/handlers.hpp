/*!
 * WTEngine | File:  handlers.hpp
 * 
 * \author Matthew Evans
 * \version 0.71
 * \copyright See LICENSE.md for copyright information.
 * \date 2019-2021
 */

#ifndef WTE_HANDLERS_HPP
#define WTE_HANDLERS_HPP

#include <variant>
#include <functional>

#include <allegro5/allegro.h>

#include "wtengine/_globals/_defines.hpp"
#include "wtengine/_globals/alert.hpp"
#include "wtengine/_globals/engine_time.hpp"
#include "wtengine/_globals/handler.hpp"
#include "wtengine/_globals/wte_exception.hpp"
#include "wtengine/config.hpp"

namespace wte {

/*!
 * \class handlers
 * \brief Input handlers.
 */
class handlers {
    friend class input;

    public:
        virtual ~handlers() = default;             //!<  Default virtual destructor.
        handlers(const handlers&) = delete;        //!<  Delete copy constructor.
        void operator=(handlers const&) = delete;  //!<  Delete assignment operator.

        /*!
         * \brief Add handler
         */
        template <size_t S>
        inline static void add(const handler<S>& handle) {
            //
        };

    protected:
        handlers();

    private:
        static void run(const ALLEGRO_EVENT& event);

        template <size_t S>
        inline static void run_handlers(const ALLEGRO_EVENT& event) {
            switch(event.type) {
            //  Keyboard events
            case ALLEGRO_EVENT_KEY_DOWN:
                break;
            case ALLEGRO_EVENT_KEY_UP:
                break;

            //  Mouse events
            case ALLEGRO_EVENT_MOUSE_AXES:
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                break;
            case ALLEGRO_EVENT_MOUSE_WARPED:
                break;
            case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
                break;
            case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
                break;

            //  Joystick events
            case ALLEGRO_EVENT_JOYSTICK_AXIS:
                break;
            case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
                break;
            case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
                break;

            //  Touch events
            case ALLEGRO_EVENT_TOUCH_BEGIN:
                break;
            case ALLEGRO_EVENT_TOUCH_END:
                break;
            case ALLEGRO_EVENT_TOUCH_MOVE:
                break;
            case ALLEGRO_EVENT_TOUCH_CANCEL:
                break;
            }
        };

        static bool initialized;  //  Restrict to one instance.
};

}  //  end namespace wte

#endif

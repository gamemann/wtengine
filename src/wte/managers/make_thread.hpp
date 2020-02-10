/*
  WTEngine
  By:  Matthew Evans
  File:  make_thread.hpp

  See LICENSE.txt for copyright information

  Wrapper class for creating a threaded object
*/

#ifndef WTE_MAKE_THREAD_HPP
#define WTE_MAKE_THREAD_HPP

#include <thread>
#include <future>
#include <chrono>

namespace wte
{

namespace mgr
{

//!  Makes object a thread
/*!
  Extend this to create a threaded object.  Has a virtual run member that is defined
  in the inheriting class.  Calling start runs the "run" member in a detached thread.
  Inside the run member, is_running can be called to see if the thread is considered
  active.  Call the stop member to end the thread.
*/
class make_thread {
    public:
        //!  Calls stop member if started flag is true
        inline virtual ~make_thread() { if(started) stop(); };

        //!  Remove copy constructor
        make_thread(const make_thread&) = delete;
        //!  Remove assignment operator
        void operator=(make_thread const&) = delete;

        //!  Call to start execution of thread
        inline void start(void) {
            if(started == true) return;
            exit_state = exit_signal.get_future();
            std::thread th([&]() { run(); });
            th.detach();
            started = true;
        };

        //!  Call to end thread
        inline void stop(void) {
            exit_signal.set_value();
            started = false;
            exit_signal = std::promise<void>();
        };

    private:
        //  Call to trigger exit
        std::promise<void> exit_signal;
        //  Track exit signal state
        std::future<void> exit_state;

        //  Flag to track status of thread
        bool started;

    protected:
        //!  Set started flag to false
        inline make_thread() : started(false) {};

        //!  Call this within run() to check if the thread is running
        inline bool is_running(void) {
            if(exit_state.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
                return true;
            return false;
        };

        //!  Override this in extended class to implement a thread
        virtual void run() = 0;
};

} //  end namespace mgr

} //  end namespace wte

#endif

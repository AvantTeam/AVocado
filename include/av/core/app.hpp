#ifndef AV_CORE_APP_HPP
#define AV_CORE_APP_HPP

#include "glad/glad.h"
#include "util/task_queue.hpp"

#include <SDL.h>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace av {
    struct app_config {
        std::string title;
        int width = 800, height = 600;
        int gl_major = 3, gl_minor = 0;
        bool vsync = true;
        bool shown = true, fullscreen, resizable;
    };

    class app_listener {

    };
    
    /**
     * A non copy-constructible class defining an application. Should only be instantiated once. Holds an SDL window, an 
     * OpenGL context, and dynamic listeners.
     */
    class app {
        /** The SDL window this application holds. Initialized in `init(const app_config &)`. */
        SDL_Window *window;
        /** The OpenGL context this application holds. Initialized in `init(const app_config &)`. */
        SDL_GLContext context;
        /** The application listeners. */
        std::vector<app_listener *> listeners;
        /** Frame-delayed runnables submitted by `post(const function<void(app &)> &)`. */
        task_queue<void, app &> posts;
        /** Whether the main loop in `loop()` should end. */
        bool exitting;

        public:
        app(const app &) = delete; // Delete the copy-constructor.
        /**
         * Instantiates an empty application. Calls to `init(const app_config &)` and `loop()` respectively are required
         * to run the application. Add application listeners by invoking `add_listener(app_listener *const &)`.
         */
        app();
        /**
         * Destroys the application. The OpenGL context the SDL window are destroyed here. The requirement for the
         * application listeners to not be destructed are released.
         */
        ~app();

        /**
         * Adds an arbitrary application listener. Typically invoked before `init()`. Use with caution.
         * @param listener - The pointer to the application listener. This is all yours and won't be destructed in any of
         *                   the application class' codes, though you must make sure it is not destructed until `loop()`
         *                   returns.
         */
        inline void add_listener(app_listener *const &listener) {
            listeners.push_back(listener);
        }
        /**
         * Removes an application listener from the list. When called inside `loop()`, it will be removed in the next
         * frame. Note that this does not destroy the listener itself, it is your responsibility to do such.
         * @param listener - The pointer to the application listener.
         */
        inline void remove_listener(app_listener *const &listener) {
            post([=](app &) {
                listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
            });
        }

        /**
         * Initializes the application, creating an SDL window and an OpenGL context.
         * @param config - The application configuration.
         * 
         * @return `true` if succeeded and both the window and context are successfully created, `false` otherwise.
         */
        bool init(const app_config &config);
        /**
         * Initializes the application loop. `init()` must be successfully invoked prior to this function. This function
         * won't return until `exitting` is `true`.
         * @return `true` if the main loop ended purely due to the application reaching exit state, `false` if the loop
         *         encountered exception(s).
         */
        bool loop();
        /**
         * Submits a function that will run at the end of the frame in `loop()`.
         * @param function - The lambda `void` function accepting `app &` parameter.
         */
        inline void post(const std::function<void(app &)> &function) {
            posts.submit(function);
        }

        protected:
        /** Runs all functions submitted by `post(const function<void(app &)> &)` and clears the delayed run list. */
        inline void run_posts() {
            posts.run(*this);
        }
    };
}

#endif // !AV_CORE_APP_HPP

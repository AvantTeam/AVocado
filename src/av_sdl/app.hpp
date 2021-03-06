#ifndef AVSDL_APPLICATION_HPP
#define AVSDL_APPLICATION_HPP

#include "glad_impl.h"
#include "input.hpp"

#include <av/log.hpp>
#include <av/time.hpp>

#include <entt/signal/delegate.hpp>
#include <SDL2/SDL.h>

#include <stdexcept>
#include <vector>

namespace av {
    /**
     * @brief A non copy-constructible class defining an application. Should only be instantiated once. Holds an SDL
     * window, an OpenGL context, and dynamic listeners.
     */
    class sdl_app {
        public:
        /** @brief An application configuration, for creation of SDL windows. */
        struct config {
            /** @brief Window title. */
            const char *title = "";
            /** @brief Window dimension. */
            int width = 800, height = 600;
            /** @brief Whether to turn on VSync at startup or not. */
            bool vsync = true;
            /** @brief FPS cap. Set to `0` to disable. */
            int fps_cap = 0;
            /** @brief Window flags. */
            bool shown = true, fullscreen, resizable;
        };

        using listener_t = entt::delegate<void(sdl_app &)>;

        private:
        /** @brief Whether the main loop should end. */
        bool exitting;

        /** @brief The SDL window this application holds. **/
        SDL_Window *window;
        /** @brief The OpenGL context this application holds. **/
        SDL_GLContext gl_context;

        /** @brief The SDL input event manager of the application. */
        sdl_input input;
        /** @brief Global time manager of the application. */
        time_manager time;

        /** @brief The application update listeners. */
        std::vector<listener_t> update_listeners;
        /** @brief The application exit listeners. */
        std::vector<listener_t> exit_listeners;

        public:
        sdl_app(const sdl_app &from) = delete;

        /**
         * @brief Instantiates an application.
         *
         * @tparam T_init The initializer function type.
         * @param  init   The initial function to be invoked after creating an OpenGL context, , in a signature of
                         `void(sdl_app &)`.
         * @param  config The application configuration.
         */
        template<typename T_init>
        sdl_app(const T_init &init, const config &conf):
            exitting(false),

            window([&]() {
            if(SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error(std::string("Couldn't initialize SDL: ").append(SDL_GetError()));

            SDL_version ver;
            SDL_GetVersion(&ver);
            log::msg("Initialized SDL v%d.%d.%d.", ver.major, ver.minor, ver.patch);

            SDL_GL_LoadLibrary(nullptr);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

            unsigned int flags = SDL_WINDOW_OPENGL;
            if(conf.shown) flags |= SDL_WINDOW_SHOWN;
            if(conf.resizable) flags |= SDL_WINDOW_RESIZABLE;

            SDL_Window *window = SDL_CreateWindow(conf.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, conf.width, conf.height, flags);
            if(!window) throw std::runtime_error(std::string("Couldn't create SDL window: ").append(SDL_GetError()));

            return window;
        }()),

            gl_context([&]() {
            SDL_GLContext gl_context = SDL_GL_CreateContext(window);
            if(!gl_context) throw std::runtime_error(std::string("Couldn't create OpenGL context: ").append(SDL_GetError()));

            if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) throw std::runtime_error("Couldn't load OpenGL extension loader.");
            log::msg("Initialized OpenGL v%d.%d.", GLVersion.major, GLVersion.minor);

            if(conf.vsync) SDL_GL_SetSwapInterval(1);
            return gl_context;
        }()) {
            init(*this);

            while(!exitting) {
                SDL_Event e;
                while(SDL_PollEvent(&e)) {
                    if(e.type == SDL_QUIT) {
                        exitting = true;
                    } else {
                        input.read(e);
                    }
                }

                time.update({0, 1});
                input.update();

                for(const listener_t &listener : update_listeners) listener(*this);
                SDL_GL_SwapWindow(window);
            }

            for(const listener_t &listener : exit_listeners) listener(*this);
        }

        /** @brief Destroys the application. The OpenGL context the SDL window are destroyed here. */
        ~sdl_app() {
            if(gl_context) SDL_GL_DeleteContext(gl_context);
            if(window) SDL_DestroyWindow(window);
            SDL_Quit();
        }

        /**
         * @brief Hooks an update listener function.
         * @tparam T_func The function, in a signature of `void(sdl_app &)`.
         */
        template<auto T_func>
        inline void on_update() {
            listener_t &listener = update_listeners.emplace_back();
            listener.connect<T_func>();
        }
        /**
         * @brief Hooks an update listener member function.
         * @tparam T_func     The function, in a signature of `void(sdl_app &)`.
         * @tparam T_instance The instance type.
         * @param  instance   The instance that the member function is bound to.
         */
        template<auto T_func, typename T_instance>
        inline void on_update(T_instance &instance) {
            listener_t &listener = update_listeners.emplace_back();
            listener.connect<T_func>(instance);
        }

        /**
         * @brief Hooks a disposal listener function.
         * @tparam T_func The function, in a signature of `void(sdl_app &)`.
         */
        template<auto T_func>
        inline void on_exit() {
            listener_t &listener = exit_listeners.emplace_back();
            listener.connect<T_func>();
        }
        /**
         * @brief Hooks a disposal listener member function.
         * @tparam T_func     The function, in a signature of `void(sdl_app &)`.
         * @tparam T_instance The instance type.
         * @param  instance   The instance that the member function is bound to.
         */
        template<auto T_func, typename T_instance>
        inline void on_exit(T_instance &instance) {
            listener_t &listener = exit_listeners.emplace_back();
            listener.connect<T_func>(instance);
        }

        /** @brief Exits the application, breaking the main-loop. */
        inline void exit() { exitting = true; }

        /** @return The SDL window this application holds. */
        inline SDL_Window *get_window() const { return window; }
        /** @return The OpenGL context this application holds. */
        inline SDL_GLContext get_context() const { return gl_context; }

        /** @return The application's input manager. */
        inline sdl_input &get_input() { return input; }
        /** @return The (read-only) application's input manager. */
        inline const sdl_input &get_input() const { return input; }

        /** @return The application's time manager. */
        inline time_manager &get_time() { return time; }
        /** @return The (read-only) application's time manager. */
        inline const time_manager &get_time() const { return time; }
    };
}

#endif // !AVSDL_APPLICATION_HPP

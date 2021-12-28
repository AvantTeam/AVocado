#ifndef AVSDL_APPLICATION_HPP
#define AVSDL_APPLICATION_HPP

#include "glad_impl.h"
#include <av/log.hpp>

#include <SDL2/SDL.h>

#include <functional>
#include <stdexcept>
#include <vector>

namespace av {
    class sdl_app {
        public:
        struct config;

        using listener_t = std::function<void(sdl_app &)>;

        private:
        bool exitting;

        SDL_Window *window;
        SDL_GLContext gl_context;

        std::vector<listener_t> loop_listeners;
        std::vector<listener_t> exit_listeners;

        public:
        sdl_app(const listener_t &init, const config &conf):
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
                        // input.read(e)
                    }
                }

                for(const listener_t &listener : loop_listeners) listener(*this);

                SDL_GL_SwapWindow(window);
            }

            for(const listener_t &listener : exit_listeners) listener(*this);
        }

        ~sdl_app() {
            if(gl_context) SDL_GL_DeleteContext(gl_context);
            if(window) SDL_DestroyWindow(window);
            SDL_Quit();
        }

        inline void on_loop(const listener_t &listener) { loop_listeners.push_back(listener); }
        inline void on_exit(const listener_t &listener) { exit_listeners.push_back(listener); }

        inline void exit() { exitting = true; }
        
        struct config {
            const char *title = "";
            int width = 800, height = 600;
            bool vsync = true;
            int fps_cap = 0;
            bool shown = true, fullscreen, resizable;
        };
    };
}

#endif // !AVSDL_APPLICATION_HPP

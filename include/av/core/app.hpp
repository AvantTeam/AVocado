#ifndef AV_CORE_APP_HPP
#define AV_CORE_APP_HPP

#include <SDL.h>
#include <glad/glad.h>

#include <string>

namespace av {
    struct app_config {
        std::string title;
        int width = 800, height = 600;
        int gl_major = 3, gl_minor = 0;
        bool vsync = true;
        bool shown = true, fullscreen, resizable;
    };
    
    class app {
        private:
        SDL_Window *window;
        SDL_GLContext context;

        public:
        app(const app &copy) = delete;
        app(const app_config &config);
        ~app();
    };
}

#endif // !AV_CORE_APP_HPP

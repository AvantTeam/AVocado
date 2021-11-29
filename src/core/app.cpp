#include "av/core/app.hpp"

#include <string>
#include <stdexcept>
#include <map>

namespace av {
    app::app(const app_config &config):
    window([&]() -> SDL_Window * {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error(std::string("Couldn't initialize SDL: ").append(SDL_GetError()).c_str());

        SDL_version ver;
        SDL_GetVersion(&ver);
        SDL_Log("Initialized SDL v%d.%d.%d.", ver.major, ver.minor, ver.patch);

        SDL_GL_LoadLibrary(nullptr);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.gl_major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.gl_minor);

        unsigned int flags = SDL_WINDOW_OPENGL;
        if(config.shown) flags |= SDL_WINDOW_SHOWN;
        if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

        SDL_Window *window = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, flags);
        if(!window) throw std::runtime_error(std::string("Couldn't create SDL window: ").append(SDL_GetError()).c_str());

        return window;
    }()),
    context([&]() -> SDL_GLContext {
        SDL_GLContext context = SDL_GL_CreateContext(window);
        if(!context) throw std::runtime_error(std::string("Couldn't create OpenGL context: ").append(SDL_GetError()).c_str());

        gladLoadGLLoader(SDL_GL_GetProcAddress);
        SDL_Log("Initialized OpenGL v%s", glGetString(GL_VERSION));

        if(config.vsync) SDL_GL_SetSwapInterval(1);
        return context;
    }()) {}

    app::~app() {
        if(context) SDL_GL_DeleteContext(context);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

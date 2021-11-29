#include "av/core/app.hpp"

#include <string>
#include <stdexcept>
#include <map>

namespace av {
    app::app(const app_config &config):
        window(nullptr),
        context(nullptr),
        config(config) {}

    app::~app() {
        if(context) SDL_GL_DeleteContext(context);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool app::init() {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) goto error;

        SDL_version ver;
        SDL_GetVersion(&ver);
        SDL_Log("Initialized SDL v%d.%d.%d.", ver.major, ver.minor, ver.patch);

        SDL_GL_LoadLibrary(nullptr);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.gl_major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.gl_minor);

        {
            unsigned int flags = SDL_WINDOW_OPENGL;
            if(config.shown) flags |= SDL_WINDOW_SHOWN;
            if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

            window = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, flags);
            if(!window) goto error;
        }

        context = SDL_GL_CreateContext(window);
        if(!context) goto error;

        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) goto error;
        SDL_Log("Initialized OpenGL v%d.%d", GLVersion.major, GLVersion.minor);

        if(config.vsync) SDL_GL_SetSwapInterval(1);
        return true;
        
        error:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, std::string("Couldn't initialize SDL application: ").append(SDL_GetError()).c_str());
        return false;
    }
}

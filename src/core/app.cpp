#include "av/core/app.hpp"

namespace av {
    app::~app() {
        if(context) SDL_GL_DeleteContext(context);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool app::init(const app_config &config) {
        if(window || context) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot re-initialize an application.");
            return false;
        }

        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
            return false;
        }
        
        SDL_version ver;
        SDL_GetVersion(&ver);
        SDL_Log("Initialized SDL v%d.%d.%d.", ver.major, ver.minor, ver.patch);

        SDL_GL_LoadLibrary(nullptr);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        unsigned int flags = SDL_WINDOW_OPENGL;
        if(config.shown) flags |= SDL_WINDOW_SHOWN;
        if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

        window = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, flags);
        if(!window) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create SDL window: %s", SDL_GetError());
            return false;
        }

        context = SDL_GL_CreateContext(window);
        if(!context) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create OpenGL context: %s", SDL_GetError());
            return false;
        }

        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load OpenGL extension loader.");
            return false;
        }

        SDL_Log("Initialized OpenGL v%d.%d.", GLVersion.major, GLVersion.minor);
        
        if(config.vsync) SDL_GL_SetSwapInterval(1);
        return accept([](auto &listener, auto &app) { listener.init(app); });
    }

    bool app::loop() {
        if(!window || !context) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "`init()` must be successfully invoked before calling `loop()`.");
            return false;
        }
        
        run_posts();
        while(!exitting) {
            SDL_Event e;
            while(SDL_PollEvent(&e)) {
                if(e.type == SDL_QUIT) {
                    exitting = true;
                } else {
                    input.read(e);
                }
            }

            try {
                input.update();
            } catch(std::exception &e) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, e.what());
                return false;
            }

            if(!accept([](auto &listener, auto &app) { listener.update(app); })) return false;

            run_posts();
            SDL_GL_SwapWindow(window);
        }

        return true;
    }
}

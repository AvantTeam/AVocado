#include <av/core/app.hpp>

namespace av {
    app::app(const app_config &config):
        initialized(true),

        window([&]() -> SDL_Window * {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            log::msg<log_level::error>("Couldn't initialize SDL: %s", SDL_GetError());

            initialized = false;
            return nullptr;
        }

        SDL_version ver;
        SDL_GetVersion(&ver);
        log::msg("Initialized SDL v%d.%d.%d.", ver.major, ver.minor, ver.patch);

        SDL_GL_LoadLibrary(nullptr);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        unsigned int flags = SDL_WINDOW_OPENGL;
        if(config.shown) flags |= SDL_WINDOW_SHOWN;
        if(config.resizable) flags |= SDL_WINDOW_RESIZABLE;

        SDL_Window *window = SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.width, config.height, flags);
        if(!window) {
            log::msg<log_level::error>("Couldn't create SDL window: %s", SDL_GetError());

            initialized = false;
            return nullptr;
        }

        return window;
    }()),
        context([&]() -> SDL_GLContext {
        SDL_GLContext context = SDL_GL_CreateContext(window);
        if(!context) {
            log::msg<log_level::error>("Couldn't create OpenGL context: %s", SDL_GetError());

            initialized = false;
            return nullptr;
        }

        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) throw std::runtime_error("Couldn't load OpenGL extension loader.");
        log::msg("Initialized OpenGL v%d.%d.", GLVersion.major, GLVersion.minor);

        if(config.vsync) SDL_GL_SetSwapInterval(1);
        return context;
    }()),
        exitting(false),
        time({0.0f, 0.0f}) {}

    app::~app() {
        if(context) SDL_GL_DeleteContext(context);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();

        log::msg("Application disposed.");
    }

    bool app::loop() {
        if(!window || !context) {
            log::msg<log_level::error>("`Application didn't instantiate correctly. Check `has_initialized()`.");
            return false;
        }

        run_posts();
        if(!accept([](app_listener &listener, app &app) -> void { listener.init(app); })) return false;

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

            time.update({0, 1});
            try {
                input.update();
            } catch(std::exception &e) {
                log::msg<log_level::error>(e.what());
                return false;
            }

            if(!accept([](app_listener &listener, app &app) -> void { listener.update(app); })) return false;

            run_posts();
            SDL_GL_SwapWindow(window);
        }

        return accept([](app_listener &listener, app &app) -> void { listener.dispose(app); });
    }
}

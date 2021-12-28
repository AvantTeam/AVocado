#include <av_sdl/app.hpp>

using namespace av;

int main(int argc, char *argv[]) {
    try {
        sdl_app([](sdl_app &app) {

        }, {"AVocado"});

        return 0;
    } catch(std::exception &e) {
        log::msg<log_level::error>(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.what(), nullptr);

        return 1;
    }
}

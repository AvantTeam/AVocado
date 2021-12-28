#include <av_sdl/app.hpp>

using namespace av;

int main(int argc, char *argv[]) {
    try {
        sdl_app([](sdl_app &app) {
            log::level = log_level::debug;
            log::msg<log_level::debug>("Size of key_bind                           : %d", sizeof(key_bind));
            log::msg<log_level::debug>("Size of key_bind::key_callback             : %d", sizeof(key_bind::key_callback *));
            log::msg<log_level::debug>("Size of function<void(const input_value &)>: %d", sizeof(std::function<void(const input_value &)>));

            key_bind move;
            move.keyboard.set_keys<SDLK_w, SDLK_s, SDLK_a, SDLK_d>();
            move.callback = [](const input_value &value) {
                const glm::vec2 &move = value.read<glm::vec2>();
                log::msg("(%f, %f)", move.x, move.y);
            };

            app.get_input().bind<key_type::keyboard>("Movement", move);
        }, {"AVocado"});

        return 0;
    } catch(std::exception &e) {
        log::msg<log_level::error>(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error", e.what(), nullptr);

        return 1;
    }
}

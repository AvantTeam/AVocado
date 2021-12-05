#include <av/core/app.hpp>
#include <stdexcept>

#include <glm/vec2.hpp>

using namespace av; // I don't care.

int main(int argc, char *argv[]) {
    log::msg<log_level::info>("Start test");

    log::set_level<log_level::error>();
    log::msg<log_level::debug>("Can't see me.");
    log::msg<log_level::error>("Notice me!");

    log::set_level<log_level::debug>();
    log::msg<log_level::debug>("Size of input_manager: %d", sizeof(input_manager));
    log::msg<log_level::debug>("Size of input_value  : %d", sizeof(input_value));
    log::msg<log_level::debug>("Size of key_bind     : %d", sizeof(key_bind));

    class: public app_listener {
        public:
        void update(app &) override {
            //throw std::runtime_error("Open't sesame.");
        }
    } listener;

    app app;
    app.add_listener(&listener);

    key_bind key;
    key.keyboard.set_keys<SDLK_w, SDLK_s, SDLK_a, SDLK_d>();
    key.callback = [](const input_value &value) {
        const glm::vec2 &axis = value.read<glm::vec2>();
        // Do something with axis...
    };

    app.get_input().bind<key_type::keyboard>("WASD", key);
    if(!app.init()) return 1;

    bool success = app.loop();

    log::msg<log_level::info>("End test.");
    return success;
}

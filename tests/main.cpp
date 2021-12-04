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

    service::app::set();

    app &app = service::app::ref();
    app.add_listener(&listener);

    key_bind key;
    key.keyboard.type = key_bind::keyboard_bind::dimension::planar;
    key.keyboard.keys[0] = SDLK_w;
    key.keyboard.keys[1] = SDLK_s;
    key.keyboard.keys[2] = SDLK_a;
    key.keyboard.keys[3] = SDLK_d;
    key.callback = [](const input_value &value) {
        const glm::vec2 &axis = value.read<glm::vec2>();
        // Do something with axis...
    };

    app.get_input().bind<key_type::keyboard>("WASD", key);
    if(!app.init()) {
        service::app::reset();
        return 1;
    }
    
    bool success = app.loop();
    service::app::reset();

    log::msg<log_level::info>("End test");

    return success;
}

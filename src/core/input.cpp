#include <av/core/input.hpp>
#include <glm/vec2.hpp>

namespace av {
    void input_manager::read(const SDL_Event &e) {
        switch(e.type) {
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN: {
                const auto &mouse = e.button;
                const auto &button = mouse.button;

                if(mouse.state) {
                    mouse_down.emplace(button);
                } else {
                    mouse_down.erase(button);
                    mouse_up.emplace(button);
                }
            } break;

            case SDL_MOUSEWHEEL: {
                mouse_wheeled = true;

                const auto &wheel = e.wheel;
                if(wheel.direction) {
                    mouse_wheel[0] = wheel.x * -1;
                    mouse_wheel[1] = wheel.y * -1;
                } else {
                    mouse_wheel[0] = wheel.x;
                    mouse_wheel[1] = wheel.y;
                }
            } break;

            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                const auto &keyboard = e.key;
                const auto &symbol = keyboard.keysym.sym;

                if(keyboard.state) {
                    key_down.emplace(symbol);
                } else {
                    key_down.erase(symbol);
                    key_up.emplace(symbol);
                }
            } break;
        }
    }

    void input_manager::update() {
        if(!mouse_down.empty() || !mouse_up.empty()) {
            input_value value;

            const auto &map = binds[static_cast<int>(key_type::mouse_button)];
            for(const auto &[name, bind] : map) {
                const auto &button = bind.mouse_button.button;
                if(mouse_down.find(button) != mouse_down.end()) {
                    value.set(button);
                    value.performed = true;

                    bind.callback(value);
                } else if(mouse_up.find(button) != mouse_up.end()) {
                    value.set(button);
                    value.performed = false;

                    bind.callback(value);
                }
            }

            mouse_up.clear();
        }

        if(mouse_wheeled) {
            input_value value(mouse_wheel);

            const auto &map = binds[static_cast<int>(key_type::mouse_wheel)];
            for(const auto &[name, bind] : map) bind.callback(value);

            mouse_wheeled = false;
        }

        if(!key_down.empty() || !key_up.empty()) {
            using dimension = key_bind::keyboard_bind::dimension;
            input_value value;

            const auto &map = binds[static_cast<int>(key_type::keyboard)];
            for(const auto &[name, bind] : map) {
                const auto &keyboard = bind.keyboard;
                switch(keyboard.type) {
                    case dimension::single: {
                        const auto &key = keyboard.keys[0];
                        if(keyboard.is_continuous()) {
                            value.set(key);
                            value.performed = key_down.find(key) != key_down.end();

                            bind.callback(value);
                        } else {
                            if(key_down.find(key) != key_down.end()) {
                                value.set(key);
                                value.performed = true;

                                bind.callback(value);
                            } else if(key_up.find(key) != key_up.end()) {
                                value.set(key);
                                value.performed = false;

                                bind.callback(value);
                            }
                        }
                    } break;

                    case dimension::linear: {
                        bool
                            positive = key_down.find(keyboard.keys[0]) != key_down.end(),
                            negative = key_down.find(keyboard.keys[1]) != key_down.end();

                        if(positive || negative) {
                            value.set(
                                positive && negative ? 0.0f :
                                negative ? 1.0f : -1.0f
                            );
                            value.performed = true;

                            bind.callback(value);
                        } else {
                            value.set(0.0f);
                            value.performed = false;

                            bind.callback(value);
                        }
                    } break;

                    default: {
                        bool
                            up = key_down.find(keyboard.keys[0]) != key_down.end(),
                            down = key_down.find(keyboard.keys[1]) != key_down.end(),
                            left = key_down.find(keyboard.keys[2]) != key_down.end(),
                            right = key_down.find(keyboard.keys[3]) != key_down.end();

                        if(up || down || left || right) {
                            glm::vec2 axis(
                                left && right ? 0.0f :
                                left ? -1.0f : right ? 1.0f : 0.0f,

                                up && down ? 0.0f :
                                up ? 1.0f : down ? -1.0f : 0.0f
                            );

                            value.set(axis);
                            value.performed = true;

                            bind.callback(value);
                        } else {
                            value.set(glm::vec2(0.0f, 0.0f));
                            value.performed = false;

                            bind.callback(value);
                        }
                    }
                }
            }

            key_up.clear();
        } else {
            using dimension = key_bind::keyboard_bind::dimension;
            input_value value;
            value.performed = false;

            const auto &map = binds[static_cast<int>(key_type::keyboard)];
            for(const auto &[name, bind] : map) {
                const auto &keyboard = bind.keyboard;
                switch(keyboard.type) {
                    case dimension::single: if(keyboard.is_continuous()) {
                        value.set(keyboard.keys[0]);
                        bind.callback(value);
                    } break;

                    case dimension::linear: {
                        value.set(0.0f);
                        bind.callback(value);
                    } break;

                    default: {
                        value.set(glm::vec2(0.0f, 0.0f));
                        bind.callback(value);
                    };
                }
            }
        }
    }
}

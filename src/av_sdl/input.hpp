#ifndef AVSDL_INPUT_HPP
#define AVSDL_INPUT_HPP

#include <SDL2/SDL.h>

#include <av/input.hpp>

namespace av {
    /** @brief Implementation of the input manager. */
    class sdl_input: public input {
        public:
        /**
         * @brief Reads user input events and populates the input states to be processed in `update()`.
         *
         * @param e The SDL event gained typically from `SDL_PollEvent(SDL_Event *)`.
         */
        void read(const SDL_Event &e) override {
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
    };
}

#endif // !AVSDL_INPUT_HPP

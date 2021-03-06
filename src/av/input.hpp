#ifndef AV_INPUT_HPP
#define AV_INPUT_HPP

#include <entt/signal/delegate.hpp>
#include <SDL2/SDL.h>

#include <av/math.hpp>

#include <functional>
#include <stdexcept>
#include <string>
#include <map>
#include <unordered_set>

namespace av {
    /** @brief An input value passed to input callback functions, containing arbitrary value and a "performed" state. */
    struct input_value {
        /** @brief Raw pointer to the actual data. Use `read<T>()` to obtain the const reference to the data. */
        const void *data;
        /** @brief "Performed" as in mouse click or mouse release, key down or key up, etc. */
        bool performed;

        /**
         * @brief Constructs an input value with the given data.
         *
         * @tparam T The data type.
         * @param value The const reference to the data.
         */
        template<typename T>
        input_value(const T &value): data(&value), performed(true) {}
        /** @brief Constructs an empty input value. */
        input_value(): data(nullptr), performed(true) {};
        /** @brief Default destructor. Nothing special. */
        ~input_value() = default;

        /**
         * @brief Sets the data of this input value.
         *
         * @param value The const reference to the data.
         */
        template<typename T>
        inline void set(const T &value) {
            data = &value;
        }

        /**
         * @brief Reads the data as the given type parameter.
         *
         * @tparam T The data type to be read as.
         * @return The casted data of this input value.
         */
        template<typename T>
        inline const T &read() const {
            return *static_cast<const T *>(data);
        }
    };

    /** @brief Defines a type for key binds to bind to. */
    enum class key_type {
        /** @brief Binds to mouse button clicks. */
        mouse_button,
        /** @brief Binds to mouse wheel scrolling. */
        mouse_wheel,
        /** @brief Binds to keyboard key presses and releases. */
        keyboard,
        /** @brief Internally used value, used as this enum's member count. */
        member_count
    };

    /**
     * @brief Key binds are wrappers around key stroke listeners. Since it is a union, setup the fields accordingly to
     * the `key_type` it binds to, otherwise you'll get weird behaviors.
     */
    union key_bind {
        /** @brief Defines a key stroke callback, in a signature of `void(const input_value &)`. */
        using callback_t = entt::delegate<void(const input_value &)>;
        
        /** @brief The key bind's data, containing callback and type. */
        struct bind_data {
            /** @brief The key bind's callback function. Must be set up before registered. */
            callback_t callback;
            /** @brief The key bind's type. */
            key_type type;
        } data;

        /** @brief Specification for mouse button binding. */
        struct mouse_button_bind {
            /** @brief Derived field for memory offset. */
            bind_data callback;

            /** @brief The mouse button. Must be one of `SDL_BUTTON_LEFT`, `SDL_BUTTON_MIDDLE`, or `SDL_BUTTON_RIGHT`. */
            unsigned char button;
        } mouse_button;

        /** @brief Specification for keyboard binding. */
        struct keyboard_bind {
            /** @brief Derived field for memory offset. */
            bind_data callback;

            /** @brief Defines "key dimensions", that is to determine the input values' data based on key presses. */
            enum class dimension {
                /**
                 * @brief Holds to one key only. In case of a button press, `input_value::data` will be set to `keys[0]`,
                 * while `input_value::performed` will be set to whether the key was pressed or released.
                 */
                single,
                /**
                 * @brief Holds to 2 keys; the "additive" key and the "subtractive" key. `keys[0]` acts as the "additive"
                 * key while `keys[1]` acts as the "subtractive" key. When the "additive" or "subtractive" are pressed,
                 * they will yield `1.0f` and `-1.0f` respectively to `input_value::data`, while `input_value::performed`
                 * is set to whether the 2 keys are pressed at all.
                 */
                 linear,
                 /**
                  * @brief Holds to 4 keys, with the order of up, down, left, and right. The `input_value::data` is set to
                  * an un-normalized `glm::vec2` representing the axis, while `input_value::performed` is set to whether
                  * any keys are pressed at all.
                  */
                  planar
            } type;

            /**
             * @brief Container for keyboard keys. In case for `dimension::single`, only `keys[0]` is used. For
             * `dimension::linear`, `keys[0]` and `keys[1]` are used for "additive" and "subtractive" keys respectively.
             * For `dimension::planar`, all 4 keys are used as up, down, left, and right keys respectively.
             *
             * Exclusively for `dimension::single`, `keys[1]` can also be used to determine whether this key bind is
             * continuous or not. That is, whether to inform update every frame if the selected key is pressed or update
             * just when the key has just been pressed or released.
             */
            unsigned short keys[4];

            /**
             * @brief Convenience function to setup key bindings and dimension type.
             * @T_key0 First key. Implicitly sets `type` to `dimension::single`.
             * @T_key1 Second key. If supplied, then will set `type` to `dimension::single`.
             * @T_key2 Third key. If supplied, then T_key3 must be supplied as well, and will set `type` to `dimension::planar`.
             * @T_key3 Fourth key. If supplied, then T_key2 must be supplied as well, and will set `type` to `dimension::planar`.
             */
            template<int T_key0, int T_key1 = 0, int T_key2 = 0, int T_key3 = 0>
            void set_keys() {
                static constexpr bool
                    single = T_key0 && !T_key1 && !T_key2 && !T_key3,
                    linear = !single && T_key0 && T_key1 && !T_key2 && !T_key3,
                    planar = !single && !linear && T_key0 && T_key1 && T_key2 && T_key3;

                static_assert(single || linear || planar, "You must supply either 1, 2 or 4 keys to the template arguments.");
                if constexpr(single) {
                    type = dimension::single;
                    keys[0] = T_key0;
                } else if constexpr(linear) {
                    type = dimension::linear;
                    keys[0] = T_key0;
                    keys[1] = T_key1;
                } else {
                    type = dimension::planar;
                    keys[0] = T_key0;
                    keys[1] = T_key1;
                    keys[2] = T_key2;
                    keys[3] = T_key3;
                }
            }

            /**
             * @brief Sets the continuous state of this key bind. Only valid for `dimension::single`.
             *
             * @param continuous Whether the input callback should be called every frame on key down or single calls each
             * key down or key release.
             */
            inline void set_continuous(bool continuous) {
                if(type != dimension::single) throw std::runtime_error("Continuous keyboard bind is only valid on single binds.");
                keys[1] = continuous;
            }

            /** @return Whether this key bind is continuous. Only valid for `dimension::single`. */
            inline bool is_continuous() const {
                return keys[1];
            }
        } keyboard;

        /** @brief Default constructor, does nothing. */
        key_bind() {}
        /** @brief Default copy-constructor, copies the values accordingly. */
        key_bind(const key_bind &from) {
            data.type = from.data.type;
            switch(data.type) {
                case key_type::mouse_button: mouse_button = from.mouse_button; break;
                case key_type::mouse_wheel: break;
                case key_type::keyboard: keyboard = from.keyboard; break;
            }
        }
        /** @brief Default move-constructor, swaps the values. */
        key_bind(key_bind &&from) {
            from.swap(*this);
        }
        /** @brief Default destructor, destructs the callback. */
        ~key_bind() {
            data.callback.~callback_t();
        }

        /** @brief Default copy-assign operator, copies the values accordingly. */
        key_bind &operator =(const key_bind &from) {
            key_bind(from).swap(*this);
            return *this;
        }
        /** @brief Default move-assign operator, swaps the values. */
        key_bind &operator =(key_bind &&from) {
            from.swap(*this);
            return *this;
        }

        /** @brief Swaps this key bind with the other. Sets the other key bind's type to this one's. */
        void swap(key_bind &other) {
            other.data.type = data.type;
            std::swap(data.callback, other.data.callback);

            switch(data.type) {
                case key_type::mouse_button: std::swap(mouse_button, other.mouse_button); break;
                case key_type::mouse_wheel: break;
                case key_type::keyboard: std::swap(keyboard, other.keyboard); break;
            }
        }
    };

    /**
     * @brief An abstract input system manager. Manages input events and process them into eventually invoking key bind
     * callbacks. Key binds can be registered via allocating a `key_bind` on stack, then calling
     * `bind(const std::string &, const key_bind &)` to register a named key bind.
     *
     * This class is implemented in av-sdl's `input.hpp`. A call to `read(const SDL_Event &e)` is typically invoked in
     * the main loop to collect all key strokes, specifically at `SDL_PollEvent(SDL_Event *)`, then `update()` to
     * actually process them.
     */
    class input {
        protected:
        /** @brief Contains all named key binds. */
        std::map<std::string, key_bind> binds[static_cast<int>(key_type::member_count)];

        /** @brief Contains pressed mouse buttons. Populated in `read(const SDL_Event &e)`, cleared in `update()`. */
        std::unordered_set<unsigned char> mouse_down;
        /** @brief Contains released mouse buttons. Populated in `read(const SDL_Event &e)`, cleared in `update()`. */
        std::unordered_set<unsigned char> mouse_up;

        /** @brief Whether a mouse wheel event was detected in `read(const SDL_Event &e)`. Reset to false in `update()` */
        bool mouse_wheeled;
        /**
         * @brief Mouse wheel data, contains X and Y values respectively. Negative X values point to left and positive
         * X values point to right. Positive Y values point against the angle of the mouse to the user, positive Y
         * values point at the angle of the mouse to the user.
         */
        int mouse_wheel[2];

        /** @brief Contains pressed keys. Populated in `read(const SDL_Event &e)`, cleared in `update()`. */
        std::unordered_set<int> key_down;
        /** @brief Contains released keys. Populated in `read(const SDL_Event &e)`, cleared in `update()`. */
        std::unordered_set<int> key_up;

        public:
        /** @brief Default constructor. */
        input(): mouse_wheeled(false) {}
        /** @brief Default destructor. */
        ~input() = default;

        /**
         * @brief Reads user input events and populates the input states to be processed in `update()`. Implemented in
         * av-sdl's `input.hpp`.
         *
         * @param e The SDL event gained typically from `SDL_PollEvent(SDL_Event *)`.
         */
        virtual void read(const union SDL_Event &e) = 0;

        /**
         * @brief Processes all contained input states and invokes key bind callbacks accordingly. This function itself
         * does not read input events; use the implementation class instead.
         */
        void update() {
            if(!mouse_down.empty() || !mouse_up.empty()) {
                input_value value;

                const auto &map = binds[static_cast<int>(key_type::mouse_button)];
                for(const auto &[name, bind] : map) {
                    const auto &button = bind.mouse_button.button;
                    if(mouse_down.find(button) != mouse_down.end()) {
                        value.set(button);
                        value.performed = true;

                        bind.data.callback(value);
                    } else if(mouse_up.find(button) != mouse_up.end()) {
                        value.set(button);
                        value.performed = false;

                        bind.data.callback(value);
                    }
                }

                mouse_up.clear();
            }

            if(mouse_wheeled) {
                input_value value(mouse_wheel);

                const auto &map = binds[static_cast<int>(key_type::mouse_wheel)];
                for(const auto &[name, bind] : map) bind.data.callback(value);

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

                                bind.data.callback(value);
                            } else {
                                if(key_down.find(key) != key_down.end()) {
                                    value.set(key);
                                    value.performed = true;

                                    bind.data.callback(value);
                                } else if(key_up.find(key) != key_up.end()) {
                                    value.set(key);
                                    value.performed = false;

                                    bind.data.callback(value);
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

                                bind.data.callback(value);
                            } else {
                                value.set(0.0f);
                                value.performed = false;

                                bind.data.callback(value);
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

                                bind.data.callback(value);
                            } else {
                                value.set(glm::vec2(0.0f, 0.0f));
                                value.performed = false;

                                bind.data.callback(value);
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
                            bind.data.callback(value);
                        } break;

                        case dimension::linear: {
                            value.set(0.0f);
                            bind.data.callback(value);
                        } break;

                        default: {
                            value.set(glm::vec2(0.0f, 0.0f));
                            bind.data.callback(value);
                        };
                    }
                }
            }
        }

        /**
         * @brief Retrieves a key bind from its registered name.
         *
         * @tparam T_type The `key_type` of the named key bind.
         * @param name The registered key bind's name.
         * @return The key bind. Will throw an exception if not found.
         */
        template<key_type T_type>
        inline key_bind &get(const std::string &name) {
            static_assert(T_type != key_type::member_count, "`key_type::all` is not to be used externally.");

            auto &map = binds[static_cast<int>(T_type)];
            if(map.find(name) == map.end()) {
                throw std::runtime_error(std::string("No such key bind: '").append(name).append("'.").c_str());
            } else {
                return map.at(name);
            }
        }

        /**
         * @brief Retrieves a key bind from its registered name.
         *
         * @tparam T_type The `key_type` of the named key bind.
         * @param name The registered key bind's name.
         * @return The (read-only) key bind. Will throw an exception if not found.
         */
        template<key_type T_type>
        inline const key_bind &get(const std::string &name) const {
            static_assert(T_type != key_type::member_count, "`key_type::all` is not to be used externally.");

            const auto &map = binds[static_cast<int>(T_type)];
            if(map.find(name) == map.end()) {
                throw std::runtime_error(std::string("No such key bind: '").append(name).append("'.").c_str());
            } else {
                return map.at(name);
            }
        }

        /**
         * @brief Registers a key bind by it's name and type.
         *
         * @tparam T_type The `key_type` of the key bind.
         * @param  name The key bind name.
         * @param  bind The key bind. This will be copied, so it is safe to allocate it in stack and discard it after.
         * @return The reference to the (copy-constructed) key bind that has just been registered. Will throw an
         *         exception if another key bind with the same type and name is already bound.
         */
        key_bind &bind(const std::string &name, const key_bind &bind) {
            if(bind.data.type == key_type::member_count) throw std::runtime_error("`key_type::all` is not to be used externally.");
            if(!bind.data.callback) throw std::runtime_error("Key binds must have their callback functions setup.");

            auto &map = binds[static_cast<int>(bind.data.type)];
            if(!map.emplace(name, bind).second) {
                throw std::runtime_error(std::string("Key bind with identifier '").append(name).append("' already bound.").c_str());
            } else {
                return map.at(name);
            }
        }
    };
}

#endif // !AV_INPUT_HPP

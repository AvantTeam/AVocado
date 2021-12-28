#ifndef AV_MATH_HPP
#define AV_MATH_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <limits>

namespace av {
    template<typename T = float>
    constexpr inline T lerp(T from, T to, T frac) {
        return from + (to - from) * frac;
    }

    template<typename T = float>
    constexpr inline T max(T a, T b) {
        return a > b ? a : b;
    }

    template<typename T = float>
    constexpr inline T min(T a, T b) {
        return a < b ? a : b;
    }

    template<typename T = float>
    constexpr inline T abs(T value) {
        return value < static_cast<T>(0) ? -value : value;
    }

    template<typename T = float>
    constexpr inline T clamp(T a, T min = 0, T max = 1) {
        return a > max ? max : a < min ? min : a;
    }

    template<typename T = float>
    constexpr inline bool within(T a, T b, T epsilon = static_cast<T>(0.0001f)) {
        return abs<T>(a - b) <= epsilon;
    }

    template<typename T = float>
    struct rect_size {
        using dimension_type = T;

        T width;
        T height;

        rect_size(): width(static_cast<T>(0)), height(static_cast<T>(0)) {}
        rect_size(T size): width(size), height(size) {}
        rect_size(T width, T height): width(width), height(height) {}
    };

    template<typename T = float>
    struct rect {
        using dimension_type = T;

        T x = 0;
        T y = 0;
        T width = 0;
        T height = 0;

        bool contained_in(const rect &other) const {
            return
                x >= other.x && y >= other.y &&
                x + width <= other.x + other.width &&
                y + height <= other.y + other.height;
        }
    };
}

#endif // !AV_MATH_HPP

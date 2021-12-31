#ifndef AV_GRAPHICS_COLOR_HPP
#define AV_GRAPHICS_COLOR_HPP

#include "../math.hpp"

namespace av {
    /**
     * @brief General usage color structure. Contains 4 float values; red, green, blue, and alpha. Can be packed to
     * either integer or float bits, mainly for vertex attribute usage.
     *
     * All of the color channels must be clamped to [0..1], it is your responsibility to do such.
     */
    struct color {
        /** @brief Red. */
        float r;
        /** @brief Green. */
        float g;
        /** @brief Blue. */
        float b;
        /** @brief Alpha. */
        float a;

        /** @brief Default constructor, sets member_count channels to `0.0f`. */
        color(): r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
        /** @brief Constructs a color with given red, blue, and green values. Alpha will eb set to `1.0f`. */
        color(float r, float g, float b): r(r), g(g), b(b), a(1.0f) {}
        /** @brief Constructs a color with given red, blue, green, and alpha values. */
        color(float r, float g, float b, float a): r(r), g(g), b(b), a(a) {}

        /** @return An integer value with each byte from the upper-most containing alpha, blue, green, and red values. */
        inline int int_bits() const {
            return
                (static_cast<int>(a * 255.0f) << 24) |
                (static_cast<int>(b * 255.0f) << 16) |
                (static_cast<int>(g * 255.0f) << 8) |
                static_cast<int>(r * 255.0f);
        }
        /** @return A float value with each byte from the upper-most containing alpha, blue, green, and red values. */
        inline float float_bits() const {
            int bits = int_bits();
            return *reinterpret_cast<float *>(&bits);
        }

        /**
         * @brief Interpolates from one color to another.
         *
         * @param from The initial color.
         * @param to   The target color.
         * @param frac The progress of the interpolation, ranges from [0..1].
         * @return The interpolated color.
         */
        static color lerp(const color &from, const color &to, float frac) {
            return {
                av::lerp(from.r, to.r, frac),
                av::lerp(from.g, to.g, frac),
                av::lerp(from.b, to.b, frac),
                av::lerp(from.a, to.a, frac)
            };
        }
    };
}

#endif // !AV_GRAPHICS_COLOR_HPP

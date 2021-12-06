#ifndef AV_UTIL_GRAPHICS_COLOR_HPP
#define AV_UTIL_GRAPHICS_COLOR_HPP

namespace av {
    struct color {
        float r;
        float g;
        float b;
        float a;

        color(): r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
        color(float r, float g, float b): r(r), g(g), b(b), a(1.0f) {}
        color(float r, float g, float b, float a): r(r), g(g), b(b), a(a) {}

        inline int int_bits() const {
            return
                ((int)(a * 255.0f) << 24) |
                ((int)(b * 255.0f) << 16) |
                ((int)(g * 255.0f) << 8) |
                (int)(r * 255.0f);
        }

        inline float float_bits() const {
            int bits = int_bits();
            return *reinterpret_cast<float *>(&bits);
        }
    };
}

#endif // !AV_UTIL_GRAPHICS_COLOR_HPP

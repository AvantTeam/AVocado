#ifndef AV_GRAPHICS_COLOR_HPP
#define AV_GRAPHICS_COLOR_HPP

namespace av {
    struct Color32;
    
    struct Color {
        float r, g, b, a;
        
        operator Color32() const {
            return {
                static_cast<unsigned char>(r * 255.0f),
                static_cast<unsigned char>(g * 255.0f),
                static_cast<unsigned char>(b * 255.0f),
                static_cast<unsigned char>(a * 255.0f)
            };
        }
        
        operator unsigned int() const {
            return static_cast<Color32>(this);
        }
        
        operator float() const {
            return static_cast<Color32>(this);
        }
    };
    
    struct Color32 {
        unsigned char r, g, b, a;
        
        operator unsigned int() const {
            return *static_cast<unsigned int *>(&r);
        }
        
        operator float() const {
            return *static_cast<float *>(&r);
        }
    };
}

#endif

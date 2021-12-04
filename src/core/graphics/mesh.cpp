#include <av/core/graphics/mesh.hpp>
#include <stdexcept>

namespace av {
    int vert_attribute::count_size() const {
        switch(type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE: return sizeof(char) * components;
            case GL_SHORT:
            case GL_UNSIGNED_SHORT: return sizeof(short) * components;
            case GL_INT:
            case GL_UNSIGNED_INT: return sizeof(int) * components;
            case GL_FLOAT: return sizeof(float) * components;
            default: throw std::runtime_error("Invalid vertex attribute type.");
        }
    }
}

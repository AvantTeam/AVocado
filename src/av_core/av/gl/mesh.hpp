#ifndef AV_GL_MESH_HPP
#define AV_GL_MESH_HPP

#include <av/gl/buffer.hpp>

namespace av {
    template<typename T_VertexType>
    struct Mesh {
        VertexBuffer vertices;
        ElementBuffer elements;
    };
}

#endif

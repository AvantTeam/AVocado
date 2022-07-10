#ifndef AV_GLFW_GL_MESH_HPP
#define AV_GLFW_GL_MESH_HPP

#include <av/glfw/gl/buffers.hpp>

namespace av {
    struct Mesh {
        VertexBuffer vertices;
        ElementBuffer elements;
        
        Mesh() {}
        
        Mesh(const GL &gl): vbo(gl), ebo(gl) {}
    };
}

#endif

#ifndef AV_GLFW_GL_MESH_HPP
#define AV_GLFW_GL_MESH_HPP

#include <av/glfw/app.hpp>
#include <av/glfw/gl/buffers.hpp>
#include <av/glfw/gl/shader.hpp>

namespace av {
    struct Mesh {
        private:
        VertexBuffer vertices;
        ElementBuffer elements;
        
        public:
        Mesh() {}
        
        Mesh(const GL &gl = app_context().gl): vertices(gl), elements(gl) {}
        
        const VertexBuffer &get_vertices() {
            return vertices;
        }
        
        const ElementBuffer &get_elements() {
            return elements;
        }
        
        void init(const GL &gl = app_context().gl) {
            if(vertices || elements) throw std::runtime_error("init() can only be called once for default-constructed mesh.");
            vertices = std::move(VertexBuffer(gl));
            elements = std::move(ElementBuffer(gl));
        }
        
        void render(const GL &gl = app_context().gl) {
            //TODO
        }
    };
}

#endif

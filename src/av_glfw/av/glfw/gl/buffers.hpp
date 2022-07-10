#ifndef AV_GLFW_GL_BUFFERS_HPP
#define AV_GLFW_GL_BUFFERS_HPP

#include <type_traits>

#include <av/gl.hpp>

namespace av {
    template<int T_buffer_type>
    struct Buffer {
        GLuint bufID;
        
        Buffer() {}
        
        Buffer(const GL &gl) {
            gl.genBuffers(1, &bufID);
        }
        
        Buffer(const Buffer &) = delete;
        
        Buffer(Buffer &&from): bufID(from.bufID) {
            from.bufID = 0;
        }
        
        Buffer &operator =(const Buffer &) = delete;
        
        Buffer &operator =(Buffer &&from) {
            bufID = from.bufID;
            from.bufID = 0;
        }
        
        ~Buffer() {
            if(bufID) gl.deleteBuffers(1, &bufID);
        }
        
        void bind(const GL &gl) const {
            gl.bindBuffer(T_buffer_type, bufID);
        }
        
        void data(const GL &gl, void *data, size_t size, bool bind = true, int usage = GL_STATIC_DRAW) const {
            if(bind) bind(gl);
            gl.bufferData(T_buffer_type, size, data, usage);
        }
        
        void sub_data(const GL &gl, void *data, size_t size, size_t offset, bool bind = true) const {
            if(bind) bind(gl);
            gl.bufferSubData(T_buffer_type, offset, size, data);
        }
    };
    
    using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
    using ElementBuffer = Buffer<GL_ELEMENT_BUFFER>;
    using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;
}

#endif

#ifndef AV_GL_BUFFER_HPP
#define AV_GL_BUFFER_HPP

#include <av/gl.hpp>
#include <av/globals.hpp>

namespace av {
    template<GLenum T_buffer_type>
    struct Buffer {
        private:
        GLuint bufID;
        
        public:
        Buffer(): bufID([]() {
            GLuint id;
            AV_gl().genBuffers(1, &id);
            
            return id;
        }()) {}
        
        Buffer(const Buffer<T_buffer_type> &) = delete;
        
        Buffer(Buffer<T_buffer_type> &&from): bufID([](GLuint ret) {
            if(bufID) AV_gl().deleteBuffers(1, &bufID);
            return ret;
        }(from.bufID)) {
            from.bufID = 0;
        }
        
        ~Buffer() {
            if(bufID) AV_gl().deleteBuffers(1, &bufID);
        }
        
        void bind() const {
            AV_gl().bindBuffer(T_buffer_type, bufID);
        }
        
        void data(void *data, size_t size, int usage = GL_STATIC_DRAW, bool bind = true) const {
            if(bind) bind();
            AV_gl().bufferData(T_buffer_type, size, data, usage);
        }
        
        void sub_data(void *data, size_t size, size_t offset, bool bind = true) const {
            if(bind) bind();
            AV_gl().bufferSubData(T_buffer_type, offset, size, data);
        }
        
        operator bool() const {
            return bufID;
        }
    };
    
    using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
    using ElementBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
    using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;
}

#endif

#ifndef AV_GLFW_GL_BUFFERS_HPP
#define AV_GLFW_GL_BUFFERS_HPP

#include <string>

#include <av/gl.hpp>
#include <av/glfw/app.hpp>

namespace av {
    template<int T_buffer_type>
    struct Buffer {
        private:
        GLuint bufID;
        
        public:
        Buffer(): bufID(0) {}
        
        Buffer(const GL &gl): bufID(create_buffer(gl)) {}
        
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
            if(bufID) app_context().gl.deleteBuffers(1, &bufID);
        }
        
        void init(const GL &gl) {
            if(bufID) throw std::runtime_error("init() can only be called once for default-constructed buffer.");
            bufID = create_buffer(gl);
        }
        
        void bind(const GL &gl = app_context().gl) const {
            gl.bindBuffer(T_buffer_type, bufID);
        }
        
        void data(void *data, size_t size, bool bind = true, int usage = GL_STATIC_DRAW, const GL &gl = app_context().gl) const {
            if(bind) bind(gl);
            gl.bufferData(T_buffer_type, size, data, usage);
        }
        
        void sub_data(void *data, size_t size, size_t offset, bool bind = true, const GL &gl = app_context().gl) const {
            if(bind) bind(gl);
            gl.bufferSubData(T_buffer_type, offset, size, data);
        }
        
        operator bool() const {
            return bufID;
        }
        
        private:
        static GLuint create_buffer(const GL &gl) {
            int id;
            gl.genBuffers(1, &id);
            
            return id;
        }
    };
    
    using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
    using ElementBuffer = Buffer<GL_ELEMENT_BUFFER>;
    using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;
}

#endif

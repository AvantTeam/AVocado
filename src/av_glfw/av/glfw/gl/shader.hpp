#ifndef AV_GLFW_GL_SHADER_HPP
#define AV_GLFW_GL_SHADER_HPP

#include <stdexcept>
#include <string>

#include <av/gl.hpp>
#include <av/glfw/glfw.hpp>

namespace av {
    struct Shader {
        private:
        GLuint vertID, fragID, progID;
        
        public:
        Shader(): vertID(0), fragID(0), progID(0) {}
        
        Shader(const GLchar *vert_source, const GLchar *frag_source, const GL &gl = app_context().gl):
            vertID(create_shader<GL_VERTEX_SHADER>(vert_source, gl)),
            fragID(create_shader<GL_FRAGMENT_SHADER>(frag_source, gl)),
            progID(create_program(vertID, fragID, gl)) {}
        
        Shader(const Shader &) = delete;
        
        Shader(Shader &&from): vertID(from.vertID), fragID(from.fragID), progID(from.progID) {
            from.vertID = from.fragID = from.progID = 0;
        }
        
        Shader &operator =(const Shader &) = delete;
        
        Shader &operator =(const Shader &&from) {
            vertID = from.vertID;
            fragID = from.fragID;
            progID = from.progID;
            from.vertID = from.fragID = from.progID = 0;
        }
        
        ~Shader() {
            const GL &gl = app_context().gl;
            if(vertID) gl.deleteShader(vertID);
            if(fragID) gl.deleteShader(fragID);
            if(progID) gl.deleteProgram(progID);
        }
        
        void compile(const GLchar *vert_source, const GLchar *frag_source, const GL &gl = app_context().gl) {
            if(vertID || fragID || progID) throw std::runtime_error("compile() can only be called once for default-constructed shader.");
            vertID = create_shader<GL_VERTEX_SHADER>(vert_source, gl);
            fragID = create_shader<GL_FRAGMENT_SHADER>(frag_source, gl);
            progID = create_program(vertID, fragID, gl);
        }
        
        operator bool() const {
            return progID;
        }
        
        private:
        template<int T_type>
        static GLuint create_shader(const GLchar *source, const GL &gl = app_context().gl) {
            GLuint id = gl.createShader(GL_VERTEX_SHADER);
            gl.shaderSource(id, 1, &source, 0);
            gl.compileShader(id);
            
            GLint compiled;
            gl.getShaderiv(id, GL_COMPILE_STATUS, &compiled);
            if(!compiled) {
                GLint len = 0;
                gl.getShaderiv(id, GL_INFO_LOG_LENGTH, &len);
                
                std::string log(len, '\0');
                gl.getShaderInfoLog(id, len, &len, log.data());
                
                gl.deleteShader(id);
                throw std::runtime_error(std::string("Error while compiling shader:\n") + log);
            }
            
            return id;
        }
        
        static GLuint create_program(GLuint &vertID, GLuint &fragID, const GL &gl = app_context().gl) {
            GLuint id = gl.createProgram();
            gl.attachShader(id, vertID);
            gl.attachShader(id, vertID);
            gl.linkProgram(id);
            
            GLint linked;
            gl.getProgramiv(id, GL_LINK_STATUS, &linked);
            if(!linked) {
                GLint len = 0;
                gl.getProgramiv(id, GL_INFO_LOG_LENGTH, &len);
                
                std::string log(len, '\0');
                gl.getProgramInfoLog(id, len, &len, log.data());
                
                gl.deleteShader(vertID);
                gl.deleteShader(fragID);
                gl.deleteProgram(id);
                
                vertID = 0;
                fragID = 0;
                throw std::runtime_error(std::string("Error while linking program:\n") + log);
            }
            
            return id;
        }
    };
}

#endif

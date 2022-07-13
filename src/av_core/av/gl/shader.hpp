#ifndef AV_GL_SHADER_HPP
#define AV_GL_SHADER_HPP

#include <stdexcept>

#include <av/gl.hpp>
#include <av/globals.hpp>
#include <av/log.hpp>

namespace av {
    struct Shader {
        private:
        GLuint progID;
        
        public:
        Shader(): progID(0) {}
        
        Shader(const GLchar *vert_source, const GLchar *frag_source) {
            compile(vert_source, frag_source);
        }
        
        Shader(const Shader &) = delete;
        
        Shader(Shader &&from): progID([](GLuint ret) {
            if(progID) AV_gl().deleteProgram(progID);
            return ret;
        }(from.progID)) {}
        
        ~Shader() {
            if(progID) AV_gl().deleteBuffers(progID);
        }
        
        void compile(const GLchar *vert_source, const GLchar *frag_source) {
            const GL &gl = AV_gl();
            if(progID) gl.deleteProgram(progID);
            
            GLuint vertID = create_shader<GL_VERTEX_SHADER>(vert_source);
            GLuint fragID = create_shader<GL_FRAGMENT_SHADER>(frag_source);
            if(!vertID || !fragID) {
                if(vertID) gl.deleteShader(vertID);
                if(fragID) gl.deleteShader(fragID);
                progID = 0;
                
                throw std::runtime_error("Failed to compile shader; see above logs for details.");
            }
            
            progID = gl.createProgram();
            gl.attachShader(progID, vertID);
            gl.attachShader(progID, fragID);
            gl.linkProgram(progID);
            
            GLint linked = 0;
            gl.getProgramiv(progID, GL_LINK_STATUS, &linked);
            if(!linked) {
                GLint len = 0;
                gl.getProgramiv(progID, GL_INFO_LOG_LENGTH, &len);
                
                std::string msg(len, '\0');
                gl.getProgramInfoLog(progID, len, &len, msg.data());
                
                gl.deleteShader(vertID);
                gl.deleteShader(fragID);
                gl.deleteProgram(progID);
                
                progID = 0;
                throw std::runtime_error(std::string("Error while linking program:\n") + log);
            } else {
                gl.detachShader(progID, vertID);
                gl.detachShader(progID, fragID);
                gl.deleteShader(vertID);
                gl.deleteShader(fragID);
            }
        }
        
        private:
        template<GLenum T_shader_type>
        static GLuint create_shader(const GLchar *source) {
            const GL &gl = AV_gl();
            
            GLuint id = gl.createShader(T_shader_type);
            gl.shaderSource(id, 1, &source, 0);
            gl.compileShader(id);
            
            GLint compiled = 0;
            gl.getShaderiv(id, GL_COMPILE_STATUS, &compiled);
            if(!compiled) {
                GLint len = 0;
                gl.getShaderiv(id, GL_INFO_LOG_LENGTH, &len);
                
                std::string msg(len, '\0');
                gl.getShaderInfoLog(id, len, &len, msg.data());
                gl.deleteShader(id);
                
                log<LogLevel::error>(msg.c_str());
                return 0;
            }
            
            return id;
        }
    };
}

#endif

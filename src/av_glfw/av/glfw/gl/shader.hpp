#ifndef AV_GLFW_GL_SHADER_HPP
#define AV_GLFW_GL_SHADER_HPP

#include <av/gl.hpp>

namespace av {
    struct Shader {
        GLuint vertID, fragID, progID;
        
        Shader() {}
        
        Shader(const GL &gl, const char *vert_source, const char *frag_source):
            vertID(gl.createShader(GL_VERTEX_SHADER)),
            fragID(gl.createShader(GL_FRAGMENT_SHADER)),
            progID(gl.createProgram()) {
            gl.shaderSource(vertID, 1, &vert_source, NULL);
            gl.shaderSource(fragID, 1, &frag_source, NULL);
            
            gl.compileShader(vertID);
            gl.compileShader(fragID);
            
            gl.attachShader(progID, vertID);
            gl.attachShader(progID, fragID);
            gl.linkProgram(progID);
        }
        
        Shader(const Shader &) = delete;
        
        Shader(Shader &&from): vertID(from.vertID), fragID(from.fragID), progID(from.progID) {
            from.vertID = 0;
            from.fragID = 0;
            from.progID = 0;
        }
        
        Shader &operator =(const Shader &) = delete;
        
        Shader &operator =(const Shader &&from) {
            vertID = from.vertID;
            fragID = from.fragID;
            progID = from.progID;
            from.vertID = from.fragID = from.progID = 0;
        }
    };
}

#endif

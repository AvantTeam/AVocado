#ifndef AV_GL_TEXTURE_HPP
#define AV_GL_TEXTURE_HPP

#include <av/gl.hpp>
#include <av/globals.hpp>

namespace av {
    struct Texture2D {
        private:
        GLuint texID;
        
        public:
        Texture2D(): texID([]() {
            GLuint id;
            AV_gl().genTextures(1, &id);
            
            return id;
        }()) {}
        
        Texture2D(const Texture2D &) = delete;
        
        Texture2D(Texture2D &&from): texID([](GLuint ret) {
            if(texID) AV_gl().deleteTextures(1, &texID);
            return ret;
        }(from.texID)) {
            from.texID = 0;
        }
        
        ~Texture2D() {
            if(texID) AV_gl().deleteTextures(1, &texID);
        }
        
        void bind() const {
            AV_gl().bindTexture(GL_TEXTURE_2D, texID);
        }
        
        void data(size_t width, size_t height, void *data, GLenum pixel_format = GL_RGBA, GLenum pixel_type = GL_UNSIGNED_BYTE, bool bind = true) const {
            if(bind) bind();
            
            const GL &gl = AV_gl();
            gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, pixel_format, pixel_type, data);
            gl.generateMipmap(GL_TEXTURE_2D);
        }
    };
}

#endif

#ifndef AV_GLFW_GLFW_HPP
#define AV_GLFW_GLFW_HPP

#include <string>

#include <GLFW/glfw3.h>

#include <av/gl.hpp>

namespace av {
    struct GLFW_WindowParams {
        std::string title;

        int gl_major = 3,
            gl_minor = 2,
            width    = 640,
            height   = 480;
        
        bool
            resizable  : 1 = true,
            visible    : 1 = true,
            decorated  : 1 = true,
            focused    : 1 = true,
            topmost    : 1 = false,
            fullscreen : 1 = false;

        int
            red_bits     = 8,
            green_bits   = 8,
            blue_bits    = 8,
            alpha_bits   = 8,
            depth_bits   = 0,
            stencil_bits = 0;

        GLFWwindow *share;
    };
    
    struct GLFW_Context {
        std::string title;
        GLFWwindow *window;
        GL gl;
        
        GLFW_Context(): window(NULL) {}
        
        GLFW_Context(const std::string &title, GLFWwindow *window): title(title), window(window) {
            set_current();
            glfwSwapInterval(1);
            log("Created window %s", title.c_str());
            
            AV_OPENGL_ES ? gladLoadGLES2Context(gl, glfwGetProcAddress) : gladLoadGLContext(gl, glfwGetProcAddress);
            log("Initialized GL: %s", gl.getString(GL_VERSION));
        }
        
        GLFW_Context(const GLFW_Context &) = delete;
        
        GLFW_Context(GLFW_Context &&from): title(std::move(from.title)), window(from.window), gl(std::move(from.gl)) {
            from.window = NULL;
        }
        
        GLFW_Context &operator =(const GLFW_Context &) = delete;
        
        GLFW_Context &operator =(GLFW_Context &&from) {
            if(window) {
                glfwDestroyWindow(window);
                log("Destroyed window %s", title.c_str());
            }
            
            title = std::move(from.title);
            window = from.window;
            gl = std::move(from.gl);
            
            from.window = NULL;
            return *this;
        }
        
        ~GLFW_Context() {
            if(window) {
                glfwDestroyWindow(window);
                log("Destroyed window %s", title.c_str());
            }
        }
        
        void set_current() {
            if(window) glfwMakeContextCurrent(window);
        }
        
        void unset_current() {
            if(glfwGetCurrentContext() == window) glfwMakeContextCurrent(NULL);
        }
        
        static GLFW_Context create(const GLFW_WindowParams &param) {
            return GLFW_Context(
                param.title,
                create_window(param)
            );
        }
        
        static GLFWwindow *create_window(const GLFW_WindowParams &param) {
            glfwWindowHint(GLFW_CLIENT_API, AV_OPENGL_ES ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            
            #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
            #endif
            
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, param.gl_major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, param.gl_minor);
            
            glfwWindowHint(GLFW_DOUBLEBUFFER, true);
            glfwWindowHint(GLFW_RESIZABLE, param.resizable);
            glfwWindowHint(GLFW_VISIBLE, param.visible);
            glfwWindowHint(GLFW_DECORATED, param.decorated);
            glfwWindowHint(GLFW_FOCUSED, param.focused);
            glfwWindowHint(GLFW_FLOATING, param.topmost);
            glfwWindowHint(GLFW_MAXIMIZED, param.fullscreen);
            
            glfwWindowHint(GLFW_RED_BITS, param.red_bits);
            glfwWindowHint(GLFW_GREEN_BITS, param.green_bits);
            glfwWindowHint(GLFW_BLUE_BITS, param.blue_bits);
            glfwWindowHint(GLFW_ALPHA_BITS, param.alpha_bits);
            glfwWindowHint(GLFW_DEPTH_BITS, param.depth_bits);
            glfwWindowHint(GLFW_STENCIL_BITS, param.stencil_bits); 
            
            return glfwCreateWindow(param.width, param.height, param.title.c_str(), NULL, param.share);
        }
    };
}

#endif

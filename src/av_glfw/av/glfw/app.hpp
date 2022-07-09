#ifndef AV_GLFW_APP
#define AV_GLFW_APP

#ifndef AV_OPENGL_ES
#define AV_OPENGL_ES false
#endif

#include <stdexcept>
#include <string>
#include <type_traits>

#define GLFW_INCLUDE_NONE
#define GLAD_GL_IMPLEMENTATION

#include <av/gl/gl.hpp>
#include <GLFW/glfw3.h>

namespace av {
    class GLFW_App {
        public:
        struct GLFW_Context;
        
        using LogicCallback = std::add_pointer<void()>::type;
        using RenderCallback = std::add_pointer<void(const GLFW_Context &)>::type;
        
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
                depth_bits   = 0,
                stencil_bits = 0;

            GLFWwindow *share;
        };
        
        struct GLFW_AppParams {
            LogicCallback logic;
            RenderCallback render;
            
            GLFW_WindowParams window;
        };
        
        struct GLFW_Context {
            GLFWwindow *window;
            GL gl;
            
            GLFW_Context(): window(NULL) {}
            
            GLFW_Context(GLFWwindow *window): window(window) {
                glfwMakeContextCurrent(window);
                glfwSwapInterval(1);
                
                AV_OPENGL_ES ? gladLoadGLES2Context(&gl, glfwGetProcAddress) : gladLoadGLContext(&gl, glfwGetProcAddress);
            }
            
            GLFW_Context(const GLFW_Context &) = delete;
            
            GLFW_Context(GLFW_Context &&from): window(from.window), gl(std::move(from.gl)) {
                from.window = NULL;
            }
            
            GLFW_Context &operator =(const GLFW_Context &) = delete;
            
            GLFW_Context &operator =(GLFW_Context &&from) {
                if(window) glfwDestroyWindow(window);
                
                window = from.window;
                gl = std::move(from.gl);
                
                return *this;
            }
            
            ~GLFW_Context() {
                if(window) glfwDestroyWindow(window);
            }
        };
        
        private:
        bool initialized;
        
        public:
        GLFW_App(GLFW_AppParams params): initialized(glfwInit()) {
            if(!initialized) throw std::runtime_error("GLFW failed to initialize.");
            
            glfwSetErrorCallback([](int error, const char *description) {
                throw std::runtime_error(std::string(std::to_string(error)) + std::string(": ") + description);
            });
            
            GLFW_Context root_context = std::move(create_context(params.window));
            while(!glfwWindowShouldClose(root_context.window)) {
                glfwSwapBuffers(root_context.window);
                glfwPollEvents();
            }
        }
        
        GLFW_App(const GLFW_App &) = delete;
        GLFW_App(GLFW_App &&) = delete;
        
        ~GLFW_App() {
            if(initialized) glfwTerminate();
        }
        
        GLFW_Context create_context(GLFW_WindowParams params) {
            glfwWindowHint(GLFW_CLIENT_API, AV_OPENGL_ES ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, params.gl_major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, params.gl_minor);
            
            glfwWindowHint(GLFW_RESIZABLE, params.resizable);
            glfwWindowHint(GLFW_VISIBLE, params.visible);
            glfwWindowHint(GLFW_DECORATED, params.decorated);
            glfwWindowHint(GLFW_FOCUSED, params.focused);
            glfwWindowHint(GLFW_FLOATING, params.topmost);
            glfwWindowHint(GLFW_MAXIMIZED, params.fullscreen);
            
            glfwWindowHint(GLFW_RED_BITS, params.red_bits);
            glfwWindowHint(GLFW_GREEN_BITS, params.green_bits);
            glfwWindowHint(GLFW_BLUE_BITS, params.blue_bits);
            glfwWindowHint(GLFW_DEPTH_BITS, params.depth_bits);
            glfwWindowHint(GLFW_STENCIL_BITS, params.stencil_bits); 
            
            return GLFW_Context(glfwCreateWindow(params.width, params.height, params.title.c_str(), NULL, params.share));
        }
    };
}

#endif

#ifndef AV_GLFW_APP_HPP
#define AV_GLFW_APP_HPP

#ifndef AV_OPENGL_ES
#define AV_OPENGL_ES false
#endif

#define GLFW_INCLUDE_NONE
#define GLAD_GL_IMPLEMENTATION

#include <cstring>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <GLFW/glfw3.h>

#include <av/gl.hpp>
#include <av/log.hpp>

namespace av {
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
            glfwMakeContextCurrent(window);
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
        
        static GLFW_Context create(GLFW_WindowParams param) {
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
            
            return GLFW_Context(param.title, glfwCreateWindow(param.width, param.height, param.title.c_str(), NULL, param.share));
        }
    };
    
    struct GLFW_AppParams {
        RenderCallback
            init, dispose,
            render;
        
        GLFW_WindowParams window;
    };
    
    namespace {
        struct Post {
            using CallbackType = std::add_pointer<void(void *)>::type;
            
            void *data;
            CallbackType callback;
        };
        
        inline std::deque<Post> posts;
        inline std::recursive_mutex post_lock;
    }
    
    bool GLFW_Run(const GLFW_AppParams &param) {
        glfwSetErrorCallback([](int error, const char *description) {
            throw std::runtime_error(std::string(std::to_string(error)) + std::string(": ") + description);
        });
        
        try {
            glfwInit();
            
            int major, minor, rev;
            glfwGetVersion(&major, &minor, &rev);
            log("Initialized GLFW v%d.%d.%d", major, minor, rev);
            
            GLFW_Context root_context = std::move(GLFW_Context::create(param.window));
            if(param.init) param.init(root_context);
            
            while(!glfwWindowShouldClose(root_context.window)) {
                {
                    std::lock_guard lock(post_lock);
                    while(!posts.empty()) {
                        Post post = std::move(posts.front());
                        posts.pop_front();
                        
                        post.callback(post.data);
                    }
                }
                
                if(param.render) param.render(root_context);
                
                glfwSwapBuffers(root_context.window);
                glfwPollEvents();
            }
            
            if(param.dispose) param.dispose(root_context);
            return false;
        } catch(std::exception &e) {
            log<LogLevel::error>("Exception thrown in the main thread: %s", e.what());
            return true;
        }
    }
    
    void GLFW_Post(void *data, Post::CallbackType callback) {
        if(!callback) return;
        
        std::lock_guard lock(post_lock);
        posts.push_back(std::move(Post{data, callback}));
    }
    
    void GLFW_Err(const std::string &msg) {
        char *str = new char[msg.length() + 1];
        std::strcpy(str, msg.c_str());
        
        GLFW_Post(str, [](void *str) {
            const char *msg = static_cast<const char *>(str);
            
            std::runtime_error err(msg);
            delete msg;
            
            throw err;
        });
    }
}

#endif

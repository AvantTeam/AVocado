#ifndef AV_GLFW_APP_HPP
#define AV_GLFW_APP_HPP

#ifndef AV_OPENGL_ES
#define AV_OPENGL_ES false
#endif

#define GLFW_INCLUDE_NONE
#define GLAD_GL_IMPLEMENTATION

#include <csignal>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <av/log.hpp>
#include <av/glfw/glfw.hpp>

namespace av {
    using PostCallback = std::add_pointer<void(void *)>::type;
    
    void app_post(void *data, PostCallback callback);
    
    void app_err(const std::string &msg);
}

#include <av/glfw/assets.hpp>

namespace av {
    struct GLFW_AppParams {
        using AppCallback = std::add_pointer<void()>::type;
        
        AppCallback
            init, dispose,
            render;
        
        GLFW_WindowParams window;
        Assets::AssetsParam assets;
    };
    
    namespace {
        struct Post {
            void *data;
            PostCallback callback;
        };
        
        inline std::deque<Post> posts;
        inline std::recursive_mutex post_lock;
        
        inline bool exit = false;
        inline bool running = false;
        
        inline GLFW_Context context;
        inline std::unique_ptr<Assets> assets;
    }
    
    void app_post(void *data, PostCallback callback) {
        if(!callback) return;
        
        std::lock_guard lock(post_lock);
        posts.push_back(std::move(Post{data, callback}));
    }
    
    void app_err(const std::string &msg) {
        char *str = new char[msg.length() + 1];
        std::strcpy(str, msg.c_str());
        
        app_post(str, [](void *str) {
            const char *msg = static_cast<const char *>(str);
            
            std::runtime_error err(msg);
            delete[] msg;
            
            throw err;
        });
    }
    
    void app_exit() {
        if(!running) throw std::runtime_error("There is no running application.");
        exit = true;
    }
    
    bool app_exiting() {
        if(!running) throw std::runtime_error("There is no running application.");
        return exit;
    }
    
    GLFW_Context &app_context() {
        if(!running) throw std::runtime_error("There is no running application.");
        return context;
    }
    
    Assets &app_assets() {
        if(!running) throw std::runtime_error("There is no running application.");
        return *assets;
    }
    
    bool app_create(const GLFW_AppParams &param) {
        if(running) throw std::runtime_error("There already is a running application.");
        glfwSetErrorCallback([](int error, const char *description) {
            throw std::runtime_error(std::string(std::to_string(error)) + std::string(": ") + description);
        });
        
        signal(SIGINT, [](int sig_num) {
            log<LogLevel::warn>("Interrupt signal raised; trying to exit...");
            if(running) app_exit();
        });
        
        try {
            running = true;
            exit = false;
            
            glfwInit();
            
            int major, minor, rev;
            glfwGetVersion(&major, &minor, &rev);
            log("Initialized GLFW v%d.%d.%d", major, minor, rev);
            
            context = std::move(GLFW_Context::create(param.window));
            
            Assets::AssetsParam assets_param(std::move(param.assets));
            assets_param.window = context.window;
            
            assets.reset(new Assets(assets_param));
            
            if(param.init) param.init();
            while(!(exit |= glfwWindowShouldClose(context.window))) {
                {
                    std::lock_guard lock(post_lock);
                    while(!posts.empty()) {
                        Post post = std::move(posts.front());
                        posts.pop_front();
                        
                        post.callback(post.data);
                    }
                }
                
                if(param.render) param.render();
                
                glfwSwapBuffers(context.window);
                glfwPollEvents();
            }
            
            if(param.dispose) param.dispose();
            assets.reset();
            
            running = false;
            return false;
        } catch(std::exception &e) {
            log<LogLevel::error>("Exception thrown in the main thread: %s", e.what());
            
            running = false;
            return true;
        }
    }
}

#endif

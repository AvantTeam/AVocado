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

#include <av/callback.hpp>
#include <av/log.hpp>
#include <av/glfw/glfw.hpp>
#include <av/glfw/assets.hpp>
#include <av/glfw/globals_glfw.hpp>

namespace av {
    struct GLFW_AppParams {
        Callback<void>
            init, dispose,
            render;
        
        GLFW_WindowParams window;
        AssetsParam assets;
    };
    
    namespace {
        inline std::deque<Callback<void>> posts;
        inline std::mutex post_lock;
        
        inline bool exit = false;
        inline bool running = false;
    }
    
    void AV_post(const Callback<void> &callback) {
        if(!callback) return;
        
        std::lock_guard<std::mutex> lock(post_lock);
        posts.push_back(std::move(callback));
    }
    
    void AV_err(const std::string &msg) {
        char *str = new char[msg.length() + 1];
        std::strcpy(str, msg.c_str());
        
        AV_post({ [](void *str) {
            const char *msg = static_cast<const char *>(str);
            
            std::runtime_error err(msg);
            delete[] msg;
            
            throw err;
        }, str });
    }
    
    void AV_exit() {
        exit = true;
    }
    
    bool AV_exiting() {
        return exit;
    }
    
    bool AV_run(GLFW_AppParams param) {
        if(running) throw std::runtime_error("There already is a running application.");
        glfwSetErrorCallback([](int error, const char *description) {
            throw std::runtime_error(std::string(std::to_string(error)) + std::string(": ") + description);
        });
        
        signal(SIGINT, [](int sig_num) {
            log<LogLevel::warn>("Interrupt signal raised; trying to exit...");
            if(running) AV_exit();
        });
        
        bool errored = false;
        try {
            running = true;
            exit = false;
            
            glfwInit();
            
            int major, minor, rev;
            glfwGetVersion(&major, &minor, &rev);
            log("Initialized GLFW v%d.%d.%d", major, minor, rev);
            
            AV_set_root_context(GLFW_Context::create(param.window));
            AV_init_assets(param.assets);
            
            GLFW_Context &context = AV_get_root_context();
            
            if(param.init) param.init();
            while(!(exit |= glfwWindowShouldClose(context.window))) {
                {
                    Callback<void> post;
                    while([&post]() {
                        std::lock_guard<std::mutex> lock(post_lock);
                        if(posts.empty()) {
                            return false;
                        } else {
                            post = std::move(posts.front());
                            posts.pop_front();
                            
                            return true;
                        }
                    }()) post();
                }
                
                if(param.render) param.render();
                glfwSwapBuffers(context.window);
                glfwPollEvents();
            }
            
            running = false;
            errored = false;
        } catch(std::exception &e) {
            log<LogLevel::error>("Exception thrown in the main thread: %s", e.what());
            
            running = false;
            errored = true;
        }
        
        if(param.dispose) param.dispose();
        AV_dispose_assets();
        AV_reset_root_context();
        
        return errored;
    }
}

#endif

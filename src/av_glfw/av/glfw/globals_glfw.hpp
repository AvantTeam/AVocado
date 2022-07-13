#ifndef AV_GLFW_GLOBALS_GLFW_HPP
#define AV_GLFW_GLOBALS_GLFW_HPP

#include <stdexcept>
#include <utility>

#include <av/globals.hpp>
#include <av/glfw/glfw.hpp>

namespace av {
    namespace {
        inline std::optional<GLFW_Context> root_context;
        inline thread_local std::optional<GLFW_Context> context {};
    }
    
    void AV_set_root_context(GLFW_Context &&ctx) {
        root_context = std::move(ctx);
    }
    
    void AV_reset_root_context() {
        root_context.reset();
    }
    
    GLFW_Context &AV_get_root_context() {
        return root_context ? *root_context : (throw std::runtime_error("Root GLFW context not defined yet."));
    }
    
    void AV_set_context(GLFW_Context &&ctx) {
        context = std::move(ctx);
    }
    
    void AV_reset_context() {
        context.reset();
    }
    
    GLFW_Context &AV_get_context() {
        return context ? *context : AV_get_root_context();
    }
    
    const GL &AV_gl() {
        return AV_get_context().gl;
    }
    
    void *AV_window() {
        return AV_get_root_context().window;
    }
}

#endif

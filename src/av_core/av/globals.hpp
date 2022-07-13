#ifndef AV_GLOBALS_HPP
#define AV_GLOBALS_HPP

#include <string>

#include <av/callback.hpp>
#include <av/gl.hpp>

namespace av {
    void AV_post(const Callback<void> &);
    
    void AV_err(const std::string &);
    
    const GL &AV_gl();
    
    void *AV_window();
}

#endif

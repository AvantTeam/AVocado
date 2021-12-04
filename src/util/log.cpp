#include "av/util/log.hpp"

namespace av {
    log_level log::level = log_level::error;
    int log::errors = 0;
    int log::warns = 0;

    #ifdef _WIN32
    HANDLE log::win_console = GetStdHandle(STD_OUTPUT_HANDLE);
    #endif
}

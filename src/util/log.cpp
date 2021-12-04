#include "av/util/log.hpp"

namespace av {
    log_level log::level = log_level::error;
    int log::errors = 0;
    int log::warns = 0;

    #ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    HANDLE log::win_console = console;
    WORD log::win_def_color = [](HANDLE console) -> WORD {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(console, &info);
        
        return info.wAttributes;
    }(console);
    #endif
}

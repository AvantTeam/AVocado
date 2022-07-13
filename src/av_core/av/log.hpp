#ifndef AV_LOG_HPP
#define AV_LOG_HPP

#include <cstdio>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace av {
    enum class LogLevel {
        none,
        info,
        warn,
        error,
        debug
    };
    
    inline LogLevel log_level = LogLevel::error;
    
    struct Prefix {
        const char *ansi, *pref;
        int windows;

        constexpr Prefix(const char *a, int w, const char *p): ansi(a), pref(p), windows(w) {}
    };
    
    static constexpr Prefix prefixes[4] = {
        {"34", 9, "[I] "},
        {"33", 14, "[W] "},
        {"31", 12, "[E] "},
        {"30;1", 8, "[D] "}
    };
    
    #ifdef _WIN32
    inline HANDLE log_win_console = GetStdHandle(STD_OUTPUT_HANDLE);
    inline WORD log_win_def_color = [](HANDLE console) -> WORD {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(console, &info);

        return info.wAttributes;
    }(log_win_console);
    #endif
    
    template<LogLevel T_level = LogLevel::info, typename... T_ArgTypes>
    static inline void log(const char *str, T_ArgTypes &&... args) {
        if(log_level == LogLevel::none || T_level > log_level) return;
        static constexpr const Prefix &prev = prefixes[static_cast<int>(T_level) - 1];

        #ifdef _WIN32
        SetConsoleTextAttribute(log_win_console, prev.windows);
        printf(prev.pref);
        SetConsoleTextAttribute(log_win_console, log_win_def_color);
        #else
        printf("\u001B[%sm%s\u001B[0m", prev.ansi, prev.pref);
        #endif

        printf(str, std::forward<T_ArgTypes>(args)...);
        printf("\n");
    }
}

#endif

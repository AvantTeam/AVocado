#ifndef AV_LOG_HPP
#define AV_LOG_HPP

#include <cstdio>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace av {
    /** @brief Defines a log level; see `log`'s documentation for details. */
    enum class log_level {
        /** @brief Message prefixed with `[I]`. */
        info,
        /** @brief Message prefixed with `[W]`. */
        warn,
        /** @brief Message prefixed with `[E]`. */
        error,
        /** @brief Message prefixed with `[D]`. */
        debug
    };

    struct prefix {
        const char *ansi, *pref;
        int windows;

        constexpr prefix(const char *a, int w, const char *p): ansi(a), pref(p), windows(w) {}
    };

    /** @brief Utility logger class. */
    class log {
        /** @brief Total error logs. */
        static int errors;
        /** @brief Total warn logs. */
        static int warns;
        /** @brief Log level prefixes and colors in the format of `{ANSI, Windows, Prefix}`. */
        static constexpr prefix prefixes[4] = {
            {"34", 9, "[I] "},
            {"33", 14, "[W] "},
            {"31", 12, "[E] "},
            {"30;1", 8, "[D] "}
        };

        #ifdef _WIN32
        static HANDLE win_console;
        static WORD win_def_color;
        #endif

        log() = delete;
        ~log() = delete;

        public:
        /** @brief Highest log level that will be shown. */
        static log_level level;

        /**
         * @brief Utility function to output a formatted message to the console, prefixed by the log level's initials.
         *
         * @tparam T_level The log level. The message will be discarded if this preceeds `level`.
         * @tparam T_args  Implicit format argument types.
         * @param  str     The message format string.
         * @param  args    Arguments to be formatted to the message.
         */
        template<log_level T_level = log_level::info, typename... T_args>
        static inline void msg(const char *str, T_args &&... args) {
            if(T_level > level) return;
            static constexpr const prefix &prev = prefixes[static_cast<int>(T_level)];

            #ifdef _WIN32
            SetConsoleTextAttribute(win_console, prev.windows);
            printf(prev.pref);
            SetConsoleTextAttribute(win_console, win_def_color);
            #else
            printf("\u001B[%sm%s\u001B[0m", prev.ansi, prev.pref);
            #endif

            printf(str, std::forward<T_args>(args)...);
            printf("\n");
        }
    };

    log_level log::level = log_level::error;
    int log::errors = 0;
    int log::warns = 0;

    #ifdef _WIN32
    HANDLE log::win_console = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD log::win_def_color = [](HANDLE console) -> WORD {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(console, &info);

        return info.wAttributes;
    }(log::win_console);
    #endif
}

#endif // !AV_LOG_HPP

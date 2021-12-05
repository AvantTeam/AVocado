#ifndef AV_UTIL_LOG_HPP
#define AV_UTIL_LOG_HPP

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
        const char* ansi, *pref;
        int windows;
        constexpr prefix(const char* a, int w, const char* p) : ansi(a), windows(w), pref(p) {}
    };

    /** @brief Utility logger class. */
    class log {
        /** @brief Highest log level that will be shown. */
        static log_level level;
        /** @brief Total error logs. */
        static int errors;
        /** @brief Total warn logs. */
        static int warns;
        /** @brief Log level prefixes and colors in the format of {ANSI, Windows, Prefix}*/
        static prefix constexpr prefixes[4] = {
            {"34", 9, "[I] "},
            {"33", 14, "[W] "},
            {"31", 12, "[E] "}, 
            {"34", 9, "[D] "}
        };

        #ifdef _WIN32
        static HANDLE win_console;
        static WORD win_def_color;
        #endif

        log() = delete;
        ~log() = delete;
        
        public:
        /**
         * @brief Utility function to output a formatted message to the console, prefixed by the log level's initials.
         *
         * @tparam T_level The log level. The message will be discarded if this preceeds `level`.
         * @tparam T_args Implicit format argument types.
         * @param str The message format string.
         * @param args Arguments to be formatted to the message.
         */
        template<log_level T_level = log_level::info, typename... T_args>
        static inline void msg(const char *str, T_args &&... args) {
            if(T_level > level) return;

            #ifdef _WIN32
                SetConsoleTextAttribute(win_console, prefixes[(int)level].windows);
                printf(prefixes[(int)level].pref);
                SetConsoleTextAttribute(win_console, win_def_color);
            #else
                printf("\u001B[%cm%c\u001B[0m", prefixes[(int)level].ansi, prefixes[(int)level].pref);

            #endif

            printf(str, std::forward<T_args>(args)...);
            printf("\n");
        }

        /**
         * @brief Sets the logger's log level limit.
         * @tparam T_level The log level; use `log_level::debug` to show all logs.
         */
        template<log_level T_level>
        static inline void set_level() {
            level = T_level;
        }

        /**
         * @brief Sets the logger's log level limit.
         * @param level The log level; use `log_level::debug` to show all logs.
         */
        static inline void set_level(log_level level) {
            log::level = level;
        }
    };
}

#endif // !AV_UTIL_LOG_HPP

#ifndef AV_CORE_UTIL_LOG_HPP
#define AV_CORE_UTIL_LOG_HPP

#include <cstdio>
#include <utility>

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

    /** @brief Utility logger class. */
    class log {
        /** @brief Highest log level that will be shown. */
        static log_level level;
        /** @brief Total error logs. */
        static int errors;
        /** @brief Total warn logs. */
        static int warns;
        /** @brief Total debug logs. */
        static int debugs;

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
            if constexpr(T_level == log_level::info) {
                printf("[I] ");
            } else if constexpr(T_level == log_level::warn) {
                warns++;
                printf("[W] ");
            } else if constexpr(T_level == log_level::error) {
                errors++;
                printf("[E] ");
            } else if constexpr(T_level == log_level::debug) {
                debugs++;
                printf("[D] ");
            }

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

#endif // !AV_CORE_UTIL_LOG_HPP

#ifndef AV_CORE_UTIL_LOG_HPP
#define AV_CORE_UTIL_LOG_HPP

namespace av {
    enum class log_level {
        info,
        warn,
        error
    };

    struct log {
        /** @brief Total error logs */
        int errors = 0;
        /** @brief Total warn logs */
        int warns = 0;

        /**
         * @brief Utility function to output a formatted message to the console, prefixed by the log level's initials.
         * 
         * @tparam level The log level
         * @param str The message to be logged
         * @param args Arguments to be formatted to the message
         */
        template<log_level level, typename... Args>
        static void message(const char *str, Args &&... args) {
            if constexpr(level == log_level::info){
                printf("[I] ");
            }else if constexpr(level == log_level::warn){
                warns++;
                printf("[W] ");
            }else if constexpr(level == log_level::error){
                errors++;
                printf("[E] ");
            };

            printf(str, std::forward<Args>(args)...);
            printf("\n");
        }
    };
}

#endif // !AV_CORE_UTIL_LOG_HPP

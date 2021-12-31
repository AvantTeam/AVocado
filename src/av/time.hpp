#ifndef AV_TIME_HPP
#define AV_TIME_HPP

#include <chrono>
#include <initializer_list>
#include <vector>

namespace av {
    /** @brief Multi-values time manager. Typically updated in main-loop. */
    class time_manager {
        using clock = std::chrono::high_resolution_clock;

        /** @brief The delta time since last update. */
        float delta_time;
        /** @brief The time point since last update. */
        clock::time_point last_time;

        /** @brief The time values. */
        std::vector<float> times;

        public:
        /**
         * @brief Constructs a time manager with initial time values.
         * @tparam T_list The time list type, defaults to `initializer_list<float>`.
         * @param  init   The list of the initial time values.
         */
        template<typename T_list = std::initializer_list<float>>
        time_manager(const T_list &init = {0.0f}):
            delta_time(0.0f),
            last_time(clock::now()),
            times(init) {}

        ~time_manager() = default;

        /**
         * @brief Updates specific time entries and recalculates the delta time.
         * @tparam T_list  The time list type, defaults to `initializer_list<int>`.
         * @param  indices The time entry indices.
         */
        template<typename T_list = std::initializer_list<int>>
        inline void update(const T_list &indices = {0}) {
            const clock::time_point &now = clock::now();
            delta_time = (now - last_time).count() / 1000000000.0f;

            for(int index : indices) times[index] += delta_time;

            last_time = std::move(now);
        }

        /**
         * @brief Resets specific time entries.
         * @tparam T_list  The time list type, defaults to `initializer_list<int>`.
         * @param  indices The time entry indices.
         */
        template<typename T_list = std::initializer_list<int>>
        inline void reset(const T_list &indices = {0}) {
            for(int index : indices) times[index] = 0.0f;
        }

        /**
         * @param index The time entry index. Must not be greater than the total size.
         * @return The time value.
         */
        inline float get(int index = 0) const { return times[index]; }
        /** @return The delta time. */
        inline float delta() const { return delta_time; }

        /** @return The total size of the time entries. */
        inline size_t size() { return times.size(); }
    };
}

#endif // !AV_TIME_HPP

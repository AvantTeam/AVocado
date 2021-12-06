#ifndef AV_UTIL_TIME_HPP
#define AV_UTIL_TIME_HPP

#include <chrono>

namespace av {
    class time {
        static float total_time, delta_time;

        private:
        static std::chrono::high_resolution_clock::time_point new_time;

        public:
        static void update(){
            std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

            delta_time = (now - new_time).count();
            total_time += delta_time;

            new_time = now;
        }
    };
}

#endif // !AV_UTIL_TIME_HPP
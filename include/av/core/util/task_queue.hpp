#ifndef AV_CORE_UTIL_TASK_QUEUE_HPP
#define AV_CORE_UTIL_TASK_QUEUE_HPP

#include <functional>
#include <mutex>
#include <vector>

namespace av {
    template<typename T_ret_type = void, typename... T_args>
    class task_queue {
        typedef std::function<T_ret_type(T_args...)> T_func;
        static constexpr bool is_void = std::is_void<T_ret_type>::value;

        std::vector<T_func> queue;
        std::recursive_mutex lock;

        public:
        inline void submit(const T_func &function) {
            lock.lock();
            queue.push_back(function);
            lock.unlock();
        }

        template<typename T = void>
        void run(T_args... args, const std::function<T(std::conditional_t<is_void, int, T_ret_type>)> &listener = {}) {
            lock.lock();
            if(queue.empty()) {
                lock.unlock();
                return;
            }

            std::vector<T_func> runs = queue;
            queue.clear();

            lock.unlock();
            if constexpr(!is_void) {
                for(const T_func &func : runs) {
                    T_ret_type &result = func(args...);
                    listener(result);
                }
            } else {
                for(const T_func &func : runs) func(args...);
            }
        }
    };
}

#endif

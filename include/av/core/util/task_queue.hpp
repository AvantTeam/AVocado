#ifndef AV_CORE_UTIL_TASKQUEUE_HPP
#define AV_CORE_UTIL_TASKQUEUE_HPP

#include <functional>
#include <mutex>
#include <vector>

namespace av {
    template<typename T_ret_type = void, typename... T_args>
    class task_queue {
        typedef std::function<T_ret_type(T_args...)> T_func;

        std::vector<T_func> queue;
        std::recursive_mutex lock;

        public:
        inline void submit(const T_func &function) {
            lock.lock();
            queue.push_back(function);
            lock.unlock();
        }

        template<auto T_listener = 0>
        void run(const T_args &... args) {
            lock.lock();
            if(queue.empty()) {
                lock.unlock();
                return;
            }

            std::vector<T_func> runs = queue;
            queue.clear();

            lock.unlock();
            if constexpr(!T_listener) {
                for(const T_func &func : runs) func(args...);
            } else {
                //TODO non-static member function
                for(const T_func &func : runs) T_listener(func(args...));
            }
        }
    };
}

#endif // !AV_CORE_UTIL_TASKQUEUE_HPP

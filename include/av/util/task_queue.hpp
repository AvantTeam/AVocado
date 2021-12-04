#ifndef AV_UTIL_TASKQUEUE_HPP
#define AV_UTIL_TASKQUEUE_HPP

#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace av {
    /** @brief A thread-safe container of functions. */
    template<typename T_ret_type = void, typename... T_args>
    class task_queue {
        public:
        /** @brief The function type. */
        using T_func = std::function<T_ret_type(T_args...)>;

        private:
        /** @brief All the queued functions. Cleared up every invocations to `run(T_args &&...)`. */
        std::vector<T_func> queue;
        /** @brief The thread lock to lock access to `queue`. */
        std::recursive_mutex thread_lock;

        public:
        /**
         * @brief Submits a function to the queue.
         * 
         * @param function The callable object to be added to, either a method reference or a lambda expression.
         */
        inline void submit(const T_func &function) {
            thread_lock.lock();
            queue.push_back(std::move(function));
            thread_lock.unlock();
        }

        /**
         * @brief Invokes all functions in the queue.
         * 
         * @param args The function arguments.
         */
        void run(T_args &&... args) {
            thread_lock.lock();
            if(queue.empty()) {
                thread_lock.unlock();
                return;
            }
            
            std::vector<T_func> runs = std::move(queue);
            queue.clear();

            thread_lock.unlock();
            for(const T_func &func : runs) func(std::forward<T_args>(args)...);
        }

        /**
         * @brief Invokes all functions in the queue with a listener.
         * 
         * @param listener The listening function, in a signature of `T(T_ret_type &)`.
         * @param args The function arguments.
         */
        template<typename T = void, typename T_sub_ret = T_ret_type, typename = typename std::enable_if_t<!std::is_void_v<T_sub_ret>>>
        void run(const std::function<T(T_sub_ret &)> &listener, T_args &&... args) {
            thread_lock.lock();
            if(queue.empty()) {
                thread_lock.unlock();
                return;
            }

            std::vector<T_func> runs = std::move(queue);
            queue.clear();

            thread_lock.unlock();
            for(const T_func &func : runs) listener(func(std::forward<T_args>(args)...));
        }
    };
}

#endif // !AV_UTIL_TASKQUEUE_HPP

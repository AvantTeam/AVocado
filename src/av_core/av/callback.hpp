#ifndef AV_CALLBACK_HPP
#define AV_CALLBACK_HPP

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace av {
    template<typename T_ReturnType, typename... T_ArgTypes>
    struct Callback {
        using FunctionType = std::add_pointer<T_ReturnType(void *, T_ArgTypes...)>::type;
        
        FunctionType func;
        void *data;
        
        T_ReturnType operator ()(T_ArgTypes... args) const {
            if(!func) throw std::runtime_error("Function pointer is null.");
            if constexpr(std::is_same<T_ReturnType, void>::value) {
                func(data, args...);
            } else {
                return func(data, args...);
            }
        }
        
        operator bool() const {
            return func;
        }
    };
}

#endif

#ifndef AV_UTIL_EXPRTRAITS_HPP
#define AV_UTIL_EXPRTRAITS_HPP

#include <type_traits>

namespace av {
    constexpr bool or_f(bool left, bool right) noexcept {
        return left || right;
    }

    template<bool T_left, bool T_right>
    struct or: std::bool_constant<or_f(T_left, T_right)> {};

    template<bool T_left, bool T_right>
    constexpr bool or_v = or<T_left, T_right>::value;

    template<typename T_left, typename T_right,
        typename = typename std::enable_if_t<or<std::is_integral_v<T_left>, std::is_floating_point_v<T_right>>>
    > constexpr bool more_than_f(T_left left, T_right right) {
        return left > right;
    }

    template<auto T_left, auto T_right>
    struct more_than: std::bool_constant<more_than_f<decltype(T_left, decltype(T_right))>(T_left, T_right)> {};

    template<auto T_left, auto T_right>
    constexpr bool more_than_v = more_than<T_left, T_right>::value;
}

#endif // !AV_UTIL_EXPRTRAITS_HPP

#ifndef AV_CORE_SERVICE_HPP
#define AV_CORE_SERVICE_HPP

#include <entt/locator/locator.hpp>

namespace av {
    class app;

    /**
     * @brief Encapsulates global accessible modules. Most of the service locators here are are modified internally,
     * and it is your responsibility not to modify nor reset them.
     */
    struct service {
        using app = entt::service_locator<app>;

        service() = delete;
        ~service() = delete;
    };
}

#endif // !AV_CORE_SERVICE_HPP

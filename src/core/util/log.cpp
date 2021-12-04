#include "av/core/util/log.hpp"

namespace av {
    log_level log::level = log_level::error;
    int log::errors = 0;
    int log::warns = 0;
    int log::debugs = 0;
}

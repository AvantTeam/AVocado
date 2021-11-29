find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

set(CORE_HEADERS
    ../include/av/core/app.hpp
)

set(CORE_SOURCES
    core/app.cpp
)

add_library(avcore ${CORE_HEADERS} ${CORE_SOURCES})
add_library(AvCompage::avcore ALIAS avcore)

target_include_directories(avcore
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(avcore
    PUBLIC
        glad::glad glm::glm
        SDL2::SDL2main SDL2::SDL2
)

install(
    FILES ${CORE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/av
)

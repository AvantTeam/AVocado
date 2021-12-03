find_package(glm REQUIRED)
find_package(EnTT REQUIRED)
find_package(SDL2 REQUIRED)

set(CORE_HEADERS
    ../include/glad/glad.h
    ../include/KHR/khrplatform.h
    ../include/av/core/app.hpp
    ../include/av/core/input.hpp
    ../include/av/core/service.hpp
    ../include/av/core/util/log.hpp
    ../include/av/core/util/task_queue.hpp
)

set(CORE_SOURCES
    glad.c
    core/app.cpp
    core/input.cpp
)

add_library(avcore ${CORE_HEADERS} ${CORE_SOURCES})
add_library(AVocado::avcore ALIAS avcore)

add_executable(packer packer/packer.cpp)

target_compile_features(avcore PUBLIC cxx_std_17)
target_compile_features(packer PUBLIC cxx_std_17)

target_include_directories(avcore
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(avcore
    PUBLIC
        glm::glm EnTT::EnTT
        SDL2::SDL2main SDL2::SDL2
)

foreach(HEADER ${CORE_HEADERS})
    string(REGEX REPLACE "[^\\/]+$" "" HEADER_DIR ${HEADER})
    install(FILES ${HEADER}
        DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${HEADER_DIR}
    )
endforeach()

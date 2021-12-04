find_package(glm REQUIRED)
find_package(EnTT REQUIRED)
find_package(SDL2 REQUIRED)

set(avbackend_HEADERS
    ../include/glad/glad.h
    ../include/KHR/khrplatform.h
    ../include/av/backend/app.hpp
    ../include/av/backend/input.hpp
)

set(avbackend_SOURCES
    backend/app.cpp
    backend/input.cpp
)

set(avcore_HEADERS
    ../include/av/core/service.hpp
    ../include/av/core/util/log.hpp
    ../include/av/core/util/command_parser.hpp
    ../include/av/core/util/task_queue.hpp
)

set(avcore_SOURCES
    glad.c
    core/util/command_parser.cpp
    core/util/log.cpp
)

foreach(SUBMODULE avcore avbackend)
    add_library(${SUBMODULE} ${${SUBMODULE}_HEADERS} ${${SUBMODULE}_SOURCES})
    add_library(AVocado::${SUBMODULE} ALIAS ${SUBMODULE})

    target_compile_features(${SUBMODULE} PUBLIC cxx_std_17)
    target_include_directories(${SUBMODULE}
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    foreach(HEADER ${${SUBMODULE}_HEADERS})
        string(REGEX REPLACE "[^\\/]+$" "" HEADER_DIR ${HEADER})
        install(FILES ${HEADER}
            DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${HEADER_DIR}
        )
    endforeach()
endforeach()

target_link_libraries(avcore
    PUBLIC
        glm::glm EnTT::EnTT
)

target_link_libraries(avbackend
    PUBLIC
        avcore
        SDL2::SDL2main SDL2::SDL2
)

add_executable(packer packer/packer.cpp)
target_compile_features(packer PUBLIC cxx_std_17)

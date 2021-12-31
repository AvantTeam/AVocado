cmake_minimum_required(VERSION 3.13)
project(AVocado
    VERSION 0.1.0
    LANGUAGES C CXX
)

option(BUILD_TESTS "Build library test units." OFF)
option(BUILD_PACKER "Build sprite packer." OFF)

add_subdirectory(src)
if(${BUILD_TESTS})
    add_subdirectory(tests)
endif()
if(${BUILD_PACKER})
    add_subdirectory(packer)
endif()

project(Tests LANGUAGES CXX)
add_executable(Tests
    main.cpp
)

target_compile_features(Tests PRIVATE cxx_std_17)
add_compile_options(-Wall -Wextra)

find_package(AVocado REQUIRED)

if(MINGW)
    target_link_libraries(Tests PRIVATE mingw32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(Tests PRIVATE -mconsole)
    endif()
endif()

target_link_libraries(Tests PRIVATE AVocado::avocado AVocado::avocado-sdl)

project(Tests LANGUAGES CXX)
add_executable(Tests
    main.cpp
)

target_compile_features(Tests PRIVATE cxx_std_17)

find_package(AVocado REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(Tests PRIVATE mingw32)
endif()
target_link_libraries(Tests PRIVATE AVocado::avbackend)

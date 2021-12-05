project(Tests LANGUAGES CXX)
add_executable(Tests
    main.cpp
)

target_compile_features(Tests PRIVATE cxx_std_17)

find_package(AVocado REQUIRED)

if(MINGW)
    target_link_libraries(Tests PRIVATE mingw32)
    target_link_options(Tests PRIVATE -mconsole)
endif()
target_link_libraries(Tests PRIVATE AVocado::avcore)

add_executable(Tests
    main.cpp
)

find_package(AVocado REQUIRED)
target_link_libraries(Tests PRIVATE
    AVocado::av_glfw AVocado::av_core
)

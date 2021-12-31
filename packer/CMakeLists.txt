add_executable(packer
    packer.cpp
)

target_compile_features(packer PRIVATE cxx_std_17)
add_compile_options(-Wall -Wextra)

find_package(AVocado REQUIRED)
find_package(cxxopts REQUIRED)

target_link_libraries(packer PRIVATE
    AVocado::avocado cxxopts::cxxopts
)

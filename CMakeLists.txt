cmake_minimum_required(VERSION 3.9)
project(AVocado
    VERSION 0.0.1
    LANGUAGES C CXX
)

option(BUILD_SHARED_LIBS "Build shared library." OFF)
option(BUILD_TESTS "Build library test units." OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(src)

foreach(SUBMODULE avcore avutil)
    set(PROJECT ${SUBMODULE})
    set(VERSION ${PROJECT_VERSION})

    set(PREFIX ${CMAKE_INSTALL_PREFIX})
    set(EXEC_PREFIX ${CMAKE_INSTALL_FULL_BINDIR})
    set(LIB_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
    set(INCLUDE_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
    
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/in/${SUBMODULE}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${SUBMODULE}.pc
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${SUBMODULE}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endforeach()

install(TARGETS avcore avutil
    EXPORT AVocadoTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

export(EXPORT AVocadoTargets
    FILE AVocadoTargets.cmake
    NAMESPACE AVocado::
)

install(EXPORT AVocadoTargets
    FILE AVocadoTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
    NAMESPACE AVocado::
)

write_basic_package_version_file(
    AVocadoConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/in/AVocadoConfig.cmake.in
    AVocadoConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

install(FILES ${PROJECT_BINARY_DIR}/AVocadoConfig.cmake ${PROJECT_BINARY_DIR}/AVocadoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

export(PACKAGE AVocado)

if(${BUILD_TESTS})
    add_subdirectory(tests)
endif()

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

foreach(SUBMODULE avcore)
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

    install(TARGETS ${SUBMODULE}
        EXPORT ${SUBMODULE}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    write_basic_package_version_file(
        ${SUBMODULE}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/in/${SUBMODULE}-config.cmake.in
        ${SUBMODULE}-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${SUBMODULE}
    )

    export(EXPORT ${SUBMODULE}-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/${SUBMODULE}-targets.cmake
        NAMESPACE AVocado::
    )

    install(EXPORT ${SUBMODULE}-targets
        FILE ${SUBMODULE}-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${SUBMODULE}
        NAMESPACE AVocado::
    )

    install(
        FILES
            ${PROJECT_BINARY_DIR}/${SUBMODULE}-config.cmake
            ${PROJECT_BINARY_DIR}/${SUBMODULE}-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${SUBMODULE}
    )
endforeach()

install(TARGETS avcore
    EXPORT AVocado-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

export(EXPORT AVocado-targets
    NAMESPACE AVocado::
    FILE AVocado-targets.cmake
)

install(EXPORT AVocado-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
    NAMESPACE AVocado::
)

write_basic_package_version_file(
    AVocado-config-version.cmake
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/in/AVocado-config.cmake.in
    AVocado-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

install(FILES ${PROJECT_BINARY_DIR}/AVocado-config.cmake ${PROJECT_BINARY_DIR}/AVocado-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

if(${BUILD_TESTS})
    add_subdirectory(tests)
endif()

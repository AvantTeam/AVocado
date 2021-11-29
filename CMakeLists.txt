cmake_minimum_required(VERSION 3.9)

project(AvCompage
    VERSION 0.0.1
    DESCRIPTION ...
    LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "Build shared library" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(src)

include(Metas.cmake)
foreach(SUBMODULE avcore)
    set(PROJECT ${SUBMODULE})
    set(DESCRIPTION ${${SUBMODULE}_DESCRIPTION})
    set(VERSION ${${SUBMODULE}_VERSION})

    set(PREFIX ${CMAKE_INSTALL_PREFIX})
    set(EXEC_PREFIX ${CMAKE_INSTALL_FULL_BINDIR})
    set(LIB_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
    set(INCLUDE_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})

    set(VERSION ${PROJECT_VERSION})
    set(CONFIG_RESULT ${CMAKE_CURRENT_BINARY_DIR}/${SUBMODULE}.pc)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/in/${SUBMODULE}.pc.in
        ${CONFIG_RESULT}
    )

    install(
        FILES ${CONFIG_RESULT}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )

    install(
        TARGETS ${SUBMODULE}
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
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${SUBMODULE}/cmake
    )

    export(
        EXPORT ${SUBMODULE}-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/${SUBMODULE}-targets.cmake
        NAMESPACE AvCompage::
    )

    install(
        EXPORT ${SUBMODULE}-targets
        FILE ${SUBMODULE}-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${SUBMODULE}/cmake
        NAMESPACE AvCompage::
    )

    install(
        FILES
            ${PROJECT_BINARY_DIR}/${SUBMODULE}-config.cmake
            ${PROJECT_BINARY_DIR}/${SUBMODULE}-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${SUBMODULE}/cmake
    )
endforeach()

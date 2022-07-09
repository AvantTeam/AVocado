find_package(glfw3 3.3 REQUIRED)

set(COMPS av_core av_glfw)

set(av_core_SOURCES
    av/gl/gl.hpp
)

set(av_glfw_SOURCES
    av/glfw/app.hpp
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

foreach(COMP ${COMPS})
    add_library(${COMP} INTERFACE)
    add_library(AVocado::${COMP} ALIAS ${COMP})

    target_compile_features(${COMP} INTERFACE cxx_std_20)
    target_include_directories(${COMP} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/${COMP}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    foreach(HEADER ${${COMP}_HEADERS})
        target_sources(${COMP} INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/${COMP}/${HEADER}>
        )
    endforeach()
endforeach()

target_link_libraries(av_glfw INTERFACE
    av_core glfw
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/in/AVocadoConfig.cmake.in
    ${PROJECT_BINARY_DIR}/AVocadoConfig.cmake
    INSTALL_DESTINATION lib/cmake/AVocado
)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/AVocadoConfigVersion.cmake
    VERSION ${VER}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS ${COMPS}
    EXPORT AVocadoTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

export(EXPORT AVocadoTargets
    FILE ${PROJECT_BINARY_DIR}/AVocadoTargets.cmake
    NAMESPACE AVocado::
)

install(EXPORT AVocadoTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
    NAMESPACE AVocado::
)

install(FILES
    ${PROJECT_BINARY_DIR}/AVocadoConfigVersion.cmake
    ${PROJECT_BINARY_DIR}/AVocadoConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

foreach(COMP ${COMPS})
    foreach(SOURCE ${${COMP}_SOURCES})
        string(REGEX REPLACE "[^\\/]+$" "" SOURCE_DIR ${SOURCE})
        install(FILES ${COMP}/${SOURCE}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${SOURCE_DIR}
        )
    endforeach()
endforeach()

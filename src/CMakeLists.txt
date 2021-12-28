find_package(glm REQUIRED)
find_package(EnTT REQUIRED)
find_package(SDL2 REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(avocado_HEADERS
    av/glad.h
    av/khrplatform.h

    av/input.hpp
    av/log.hpp
    av/math.hpp
    av/time.hpp
)

set(avocado-sdl_HEADERS
    av_sdl/glad_impl.h

    av_sdl/app.hpp
    av_sdl/input.hpp
)

foreach(COMP avocado avocado-sdl)
    add_library(${COMP} INTERFACE ${${COMP}_HEADERS})
    add_library(AVocado::${COMP} ALIAS ${COMP})

    target_compile_features(${COMP} INTERFACE cxx_std_17)
    target_include_directories(${COMP} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    foreach(HEADER ${${COMP}_HEADERS})
        string(REGEX REPLACE "[^\\/]+$" "" HEADER_DIR ${HEADER})
        install(FILES ${HEADER}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${HEADER_DIR}
        )
    endforeach()

    set(VERSION ${PROJECT_VERSION})
    set(PREFIX ${CMAKE_INSTALL_PREFIX})
    set(EXEC_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
    set(LIB_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    set(INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})

    configure_file(
        ${PROJECT_SOURCE_DIR}/cmake/in/${COMP}.pc.in
        ${PROJECT_BINARY_DIR}/${COMP}.pc
    )

    install(
        FILES ${PROJECT_BINARY_DIR}/${COMP}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endforeach()

target_link_libraries(avocado INTERFACE
    glm::glm EnTT::EnTT
)

target_link_libraries(avocado-sdl INTERFACE
    avocado
    SDL2::SDL2main
)
if(NOT TARGET SDL2::SDL2-static)
    target_link_libraries(avocado-sdl INTERFACE SDL2::SDL2)
else()
    target_link_libraries(avocado-sdl INTERFACE SDL2::SDL2-static)
endif()

install(TARGETS avocado avocado-sdl
    EXPORT AVocadoTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

export(EXPORT AVocadoTargets
    FILE ${PROJECT_BINARY_DIR}/AVocadoTargets.cmake
    NAMESPACE AVocado::
)

install(EXPORT AVocadoTargets
    FILE AVocadoTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
    NAMESPACE AVocado::
)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/AVocadoConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/in/AVocadoConfig.cmake.in
    ${PROJECT_BINARY_DIR}/AVocadoConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

install(FILES ${PROJECT_BINARY_DIR}/AVocadoConfig.cmake ${PROJECT_BINARY_DIR}/AVocadoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AVocado
)

export(PACKAGE AVocado)

add_library(libyad process.cpp pipe.cpp)
add_library(yad::libyad ALIAS libyad)

set_target_properties(
    libyad
    PROPERTIES OUTPUT_NAME yad    
)

target_compile_features(libyad PUBLIC cxx_std_17)

target_include_directories(
    libyad
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include
)

include(GNUInstallDirs)

install(TARGETS libyad
    EXPORT yad-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT yad-targets
    FILE yad-config.cmake
    NAMESPACE yad::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yad
)



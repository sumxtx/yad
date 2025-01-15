add_library(libyad libyad.cpp)
add_library(yad::libyad ALIAS libyad)

set_target_properties(
    libyad
    PROPERTIES OUTPUT_NAME yad    
)

target_compile_features(libyad PUBLIC cxx_std_17)

target_include_directories(
    libyad
    PRIVATE include
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

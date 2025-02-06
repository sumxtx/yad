cmake_minimum_required(VERSION 3.28)

project("yad" LANGUAGES CXX)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")

# Find the edit library (libedit)
find_library(EDIT_LIBRARY NAMES edit)

# Check if the library is found
if (EDIT_LIBRARY)
    target_link_libraries("yad" PRIVATE ${EDIT_LIBRARY})
else()
    message(FATAL_ERROR "libedit not found")
endif()

if(BUILD_TESTING)
    add_subdirectory("test")
endif()



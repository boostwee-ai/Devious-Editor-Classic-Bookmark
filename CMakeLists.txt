cmake_minimum_required(VERSION 3.29)

# Direct inclusion for CI environments where the package registry might be missing
if(DEFINED ENV{GEODE_SDK})
    message(STATUS "Using GEODE_SDK from environment: $ENV{GEODE_SDK}")
    set(Geode_DIR "$ENV{GEODE_SDK}/cmake")
    include("$ENV{GEODE_SDK}/cmake/GeodeConfig.cmake")
else()
    find_package(Geode REQUIRED)
endif()

project(devious-editor-classic-bookmark VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
	add_compile_definitions(_WIN32)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_geode_mod(
    NAME "Devious Editor Classic Bookmark"
    ID "bishop.devious-editor-classic-bookmark"
    VERSION "1.0.0"
    SOURCES ${SOURCES}
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

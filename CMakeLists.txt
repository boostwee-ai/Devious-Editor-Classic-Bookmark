cmake_minimum_required(VERSION 3.29)

# Finding Geode before project() can sometimes help with toolchains
if(DEFINED ENV{GEODE_SDK})
    set(Geode_DIR "$ENV{GEODE_SDK}/cmake")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{GEODE_SDK}")
endif()
find_package(Geode REQUIRED)

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

cmake_minimum_required(VERSION 3.29)

project(devious-editor-classic-bookmark VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Geode REQUIRED)

# Need sockets for mac/windows
if(WIN32)
	add_compile_definitions(_WIN32)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_geode_mod(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

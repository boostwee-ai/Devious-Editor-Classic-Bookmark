cmake_minimum_required(VERSION 3.29)

project(devious-editor-classic-bookmark VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Geode REQUIRED HINTS "$ENV{GEODE_SDK}")

# Need sockets for mac/windows
if(WIN32)
	add_compile_definitions(_WIN32)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

setup_geode_mod(${PROJECT_NAME} SOURCES ${SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

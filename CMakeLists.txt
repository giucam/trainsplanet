cmake_minimum_required(VERSION 2.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -g -std=c++0x")

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/3dparty/noisepp/core ${CMAKE_SOURCE_DIR}/3dparty/noisepp/threadpp)
LINK_DIRECTORIES(${CMAKE_BUILD_DIR})

find_package(Qt5Core)
find_package(Qt5Gui)

set(CMAKE_AUTOMOC ON)

add_subdirectory(3dparty/noisepp)

set(SOURCES
    src/main.cpp
    src/window.cpp
    src/miscutils.cpp
    src/gl/glprogram.cpp
    src/terrain/terrain.cpp
    src/terrain/datafetcher.cpp
    src/terrain/heightmap.cpp
    src/terrain/quadtree.cpp)

add_executable(trainsplanet ${SOURCES})
qt5_use_modules(trainsplanet Gui Quick)
target_link_libraries(trainsplanet GL noisepp)


set(CMAKE_CXX_FLAGS "-g -std=c++0x")

set(SOURCES
    utils/NoiseBuilders.cpp
    utils/NoiseInStream.cpp
    utils/NoiseWriter.cpp
    utils/NoiseGradientRenderer.cpp
    utils/NoiseReader.cpp
    utils/NoiseModules.cpp
    utils/NoiseSystem.cpp
    utils/NoiseColourValue.cpp
    utils/NoiseJobQueue.cpp
    utils/NoiseEndianUtils.cpp
    utils/NoiseOutStream.cpp
    utils/NoiseImage.cpp)

add_library(noisepp SHARED ${SOURCES})

cmake_minimum_required(VERSION 2.8)

project (ConfigFile CXX)
set(CMAKE_BUILD_TYPE Release)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(ConfigFile STATIC ConfigFile.cpp)

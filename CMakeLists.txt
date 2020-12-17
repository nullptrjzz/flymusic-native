cmake_minimum_required(VERSION 3.17)
project(flyaudio)

set(CMAKE_CXX_STANDARD 14)
link_directories(./)

add_library(flyaudio SHARED FlyAudio.h FlyAudio.cpp)
target_link_libraries(flyaudio libbass.so)
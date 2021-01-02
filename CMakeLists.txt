cmake_minimum_required(VERSION 3.17)
project(flyaudio)

set(CMAKE_CXX_STANDARD 14)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
include_directories (${GTK3_INCLUDE_DIRS})
link_directories (./ ${GTK3_LIBRARY_DIRS})

add_library(flyaudio SHARED FlyAudio.h FlyAudio.cpp md5.cpp md5.h json.hpp kb_lin.hpp push.hpp)
add_definitions (${GTK3_CFLAGS_OTHER})
target_link_libraries(flyaudio libbass.so libbassflac.so libtag.so libtag_c.so X11 Xext Xi ${GTK3_LIBRARIES} libuiohook.so pthread)
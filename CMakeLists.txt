cmake_minimum_required(VERSION 3.5)

project (PayloadSDK)

set(CMAKE_BUILD_TYPE Debug)

if (VIO)
set(VIO $ENV{VIO})
message("-- VIO: ${VIO}")
add_definitions(-DVIO=${VIO})
endif(VIO)

find_package(PkgConfig REQUIRED)
#Use Pkg-config to configure GStreamer
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GST REQUIRED
                    gstreamer-1.0
                    gstreamer-video-1.0
                    gstreamer-app-1.0
                    )

add_subdirectory(libs)
add_subdirectory(bindings)
# examples
add_subdirectory(examples)

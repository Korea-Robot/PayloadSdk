cmake_minimum_required(VERSION 3.5)

set(PAYLOADSDKLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libs)

find_package( Threads )

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
find_package(CURL REQUIRED)
find_package(spdlog REQUIRED)

#include
include_directories(${PAYLOADSDKLIB_DIR}
                    ${PAYLOADSDKLIB_DIR}/third-party/mavlink/include
                    ${PAYLOADSDKLIB_DIR}/gSDK/src/mavlink/include/mavlink/v2.0
                    ${PAYLOADSDKLIB_DIR}/gSDK/src
                    ${GST_INCLUDE_DIRS}
                    ${CURL_INCLUDE_DIR}
                    )

# Add a Python extension module using pybind11
find_package(pybind11 REQUIRED)
pybind11_add_module(pypayload pysdk.cpp)
target_link_libraries(pypayload
    PRIVATE
        PayloadSDK
        ${CMAKE_THREAD_LIBS_INIT}
        ${GST_LIBRARIES}
        ${JSONCPP_LIBRARIES}
        ${CURL_LIBRARIES}
        stdc++fs
)

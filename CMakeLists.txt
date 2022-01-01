cmake_minimum_required(VERSION 3.0.0)
project(deepstream_cxx VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories("/opt/nvidia/deepstream/deepstream/sources/includes")
link_directories("/opt/nvidia/deepstream/deepstream-6.0/lib")

include_directories("/usr/include/gstreamer-1.0")
include_directories("/usr/include/glib-2.0")
include_directories("/usr/lib/aarch64-linux-gnu/glib-2.0/include")

add_executable(deepstream_cxx main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(${PROJECT_NAME} cudart nvds_meta nvdsgst_meta gstreamer-1.0 gobject-2.0 glib-2.0)

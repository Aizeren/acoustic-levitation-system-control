cmake_minimum_required(VERSION 3.8)

project(phase-monitoring VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE main.cpp functions.cpp)
set(HEADER functions.h)
set(OpenCV_DIR C:/opencv/build)

find_package(OpenCV REQUIRED)
add_executable(phase-monitoring ${SOURCE} ${HEADER})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT phase-monitoring)

target_link_libraries( phase-monitoring ${OpenCV_LIBS})
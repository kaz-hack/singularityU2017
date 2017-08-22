#faceDetection
project(faceDetection)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ../bin)

find_package(OpenCV 3 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${OpenCV_INCLUDE_DIRS})

set(LIBRARIES /usr/local/lib)

add_executable(faceDetection faceDetection.cpp)

target_link_libraries(faceDetection ${OpenCV_LIBS})
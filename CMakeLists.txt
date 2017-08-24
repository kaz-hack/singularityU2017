cmake_minimum_required(VERSION 2.8.4)

PROJECT(myApplications)

set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

#dlibの解凍先に合わせて適宜変更
include(dlib/cmake)

macro(add_example name)
   add_executable(${name} ${name}.cpp)
   target_link_libraries(${name} dlib::dlib ${OpenCV_LIBS})
endmacro()

macro(add_gui_example name)
   if (DLIB_NO_GUI_SUPPORT)
      message("No GUI support, so we won't build the ${name} example.")
  else()
      add_example(${name})
   endif()
endmacro()

if (DLIB_NO_GUI_SUPPORT)
   message("No GUI support, so we won't build the webcam_face_pose_ex example.")
else()
   find_package(OpenCV QUIET)
   if (OpenCV_FOUND)
      include_directories(${OpenCV_INCLUDE_DIRS})

   else()
      message("OpenCV not found, so we won't build the webcam_face_pose_ex example.")
   endif()
endif()

add_example(face_detection)

# ===================================================================================
#  The dlib CMake configuration file
#
#             ** File generated automatically, do not modify **
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    FIND_PACKAGE(dlib REQUIRED)
#    TARGET_LINK_LIBRARIES(MY_TARGET_NAME ${dlib_LIBRARIES})
#
#    This file will define the following variables:
#      - dlib_LIBRARIES                : The list of all imported targets for dlib modules.
#      - dlib_INCLUDE_DIRS             : The dlib include directories.
#      - dlib_VERSION                  : The version of this dlib build.
#      - dlib_VERSION_MAJOR            : Major version part of this dlib revision.
#      - dlib_VERSION_MINOR            : Minor version part of this dlib revision.
#
# ===================================================================================



 
# Our library dependencies (contains definitions for IMPORTED targets)
if(NOT TARGET dlib-shared AND NOT dlib_BINARY_DIR)
   # Compute paths
   get_filename_component(dlib_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
   include("${dlib_CMAKE_DIR}/dlib.cmake")
endif()

find_library(dlib_LIBRARIES dlib HINTS "/usr/local/lib")
set(dlib_LIBRARIES ${dlib_LIBRARIES} "/opt/X11/lib/libSM.dylib;/opt/X11/lib/libICE.dylib;/opt/X11/lib/libX11.dylib;/opt/X11/lib/libXext.dylib;/usr/local/lib/libpng.dylib;/usr/lib/libcblas.dylib;/usr/lib/liblapack.dylib;/usr/lib/libsqlite3.dylib")
set(dlib_LIBS      ${dlib_LIBRARIES} "/opt/X11/lib/libSM.dylib;/opt/X11/lib/libICE.dylib;/opt/X11/lib/libX11.dylib;/opt/X11/lib/libXext.dylib;/usr/local/lib/libpng.dylib;/usr/lib/libcblas.dylib;/usr/lib/liblapack.dylib;/usr/lib/libsqlite3.dylib")
set(dlib_INCLUDE_DIRS "/usr/local/include" "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/usr")

include(/usr/local/include/dlib/cmake_utils/use_cpp_11.cmake)

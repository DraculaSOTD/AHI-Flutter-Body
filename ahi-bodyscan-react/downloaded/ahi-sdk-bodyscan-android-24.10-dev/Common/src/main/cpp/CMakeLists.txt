cmake_minimum_required(VERSION 3.18.1)
project(common)
set(CMAKE_CXX_STANDARD 17)

include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

add_library(common
        SHARED
        Common.cpp)

# Add logging library
find_library(log-lib
        log)

# Add include directories from 3rd party c++ dependencies.
target_include_directories(
        common
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Link common library with 3rd party dependency libraries
target_link_libraries(common
        lib_opencv
        -ljnigraphics
        ${log-lib})

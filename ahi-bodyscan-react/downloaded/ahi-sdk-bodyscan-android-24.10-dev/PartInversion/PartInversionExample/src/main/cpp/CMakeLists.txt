cmake_minimum_required(VERSION 3.18.1)
set(CMAKE_CXX_STANDARD 17)
project("example")

include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

add_library(example
        SHARED
        example.cpp)

find_library(log-lib
        log)

target_link_libraries(example
        lib_opencv
        ${log-lib})

cmake_minimum_required(VERSION 3.18.1)
project(poseInspection)
set(CMAKE_CXX_STANDARD 17)

include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

# Get all external source files
file(GLOB EXTERNAL_SOURCES
        ../../../../Common/src/main/cpp/Common.cpp)

include_directories(../../../../Common/src/main/cpp)

# Create Pose Inspection library,
add_library(poseInspection
        SHARED
        PoseInspectionJNI.cpp
        PoseInspection.cpp
        ${EXTERNAL_SOURCES})

find_library(log-lib
        log)

# Add include directories from the poseInspection library and 3rd party c++ dependencies.
target_include_directories(poseInspection
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include)

# Link poseInspection library with common libraries and 3rd party dependency libraries
target_link_libraries(
        poseInspection
        lib_opencv
        ${log-lib}
        -ljnigraphics
)

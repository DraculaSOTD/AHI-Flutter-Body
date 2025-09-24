# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.18.1)
project(ProjectName)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-experimental-isel")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -llog")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=sha1")

set(TFLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/prebuilt-tensorflow-lite")
set(TFLITE_LIBS_DIR "${TFLITE_DIR}/libs/${ANDROID_ABI}")

# Get all external source files
file(GLOB_RECURSE EXTERNAL_SOURCES ../../../../Common/src/main/cpp/*.cpp)
include_directories(../../../../Common/src/main/cpp)

# Add openCV
include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

# Add AHICommon
if (DEFINED LOCAL_COMMON_DIR)
    add_subdirectory(${LOCAL_COMMON_DIR} ${LOCAL_COMMON_DIR}/build)
else ()
    # This will find all of the libraries from the common module, using prefab
    find_package(ahi-sdk-common-android REQUIRED CONFIG)
endif ()

# Add jsoncpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/jsoncpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/jsoncpp/include)

# Add TFLite
file(GLOB TFLITE_LIBS "${TFLITE_LIBS_DIR}/lib*.*")
include_directories(
        ${TFLITE_DIR}/include
)
add_library(tensorflow-lite STATIC IMPORTED)
set_target_properties(tensorflow-lite PROPERTIES IMPORTED_LOCATION ${TFLITE_LIBS_DIR}/libtensorflow-lite.a)
add_library(tensorflow-lite-lib INTERFACE)
target_link_libraries(tensorflow-lite-lib
        INTERFACE
        ${TFLITE_LIBS}
        tensorflow-lite)

# Get all the source files for the bodyscan util library
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Get all external source files
file(GLOB EXTERNAL_SOURCES
        ../../../../Common/src/main/cpp/Common.cpp
        ../../../../Common/src/main/cpp/jnihelper/*.cpp)

include_directories(../../../../Common/src/main/cpp)

file(GLOB SEGMENTATION_UTIL_SOURCES
        *.cpp
        )

# Create the segmentation library
add_library(
        segmentation
        SHARED
        SegmentationJNI.cpp
        ${SEGMENTATION_UTIL_SOURCES}
        ${EXTERNAL_SOURCES}
)

if (DEFINED LOCAL_COMMON_DIR)
    target_include_directories(
            segmentation
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            tensorflow-lite-lib
            PUBLIC
            ${LOCAL_COMMON_DIR}/src/main/cpp/util/include
    )

    # Link segmentation library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            segmentation
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            tensorflow-lite-lib
            ${LOCAL_COMMON_DIR}/build/intermediates/library_jni/debug/jni/${CMAKE_ANDROID_ARCH_ABI}/libahi_common_util.so
    )
else ()
    # Link segmentation library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            segmentation
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            tensorflow-lite-lib
            ahi-sdk-common-android::ahi_common_util
    )
endif ()


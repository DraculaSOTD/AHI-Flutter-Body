cmake_minimum_required(VERSION 3.18.1)
project(contourGenerator)
set(CMAKE_CXX_STANDARD 17)

include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

# Get all external source files
file(GLOB EXTERNAL_SOURCES
        ../../../../Common/src/main/cpp/*.cpp
        ../../../../Common/src/main/cpp/jnihelper/*.cpp
        ../../../../AHIBodyScan/src/main/cpp/util/tiny-AES-c/*.cpp
        ../../../../AHIBodyScan/src/main/cpp/util/tiny-AES-c/*.c)

include_directories(../../../../Common/src/main/cpp)
include_directories(../../../../AHIBodyScan/src/main/cpp/util/tiny-AES-c)

# Add AHICommon
if (DEFINED LOCAL_COMMON_DIR)
    add_subdirectory(${LOCAL_COMMON_DIR} ${LOCAL_COMMON_DIR}/build)
else ()
    # This will find all of the libraries from the common module, using prefab
    find_package(ahi-sdk-common-android REQUIRED CONFIG)
endif ()

# Add cereal
include_directories(../../../../Common/src/main/cpp/libs/cereal/include)

# Get all Contour source files
file(GLOB_RECURSE CONTOUR_SOURCES
        *.cpp)

# Create the contour library
add_library( # Sets the name of the library.
        contourGenerator
        SHARED
        ${CONTOUR_SOURCES}
        ${EXTERNAL_SOURCES})

# Add logging library
find_library(log-lib
        log)

# Add include directories from 3rd party c++ dependencies.
target_include_directories(
        contourGenerator
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Get all 3rd party c++ libraries.
file(GLOB CPP_DEPS_LIBS "${CPP_UTILS_PATH}/${CMAKE_ANDROID_ARCH_ABI}/lib*.*")


if (DEFINED LOCAL_COMMON_DIR)
    target_include_directories(
            contourGenerator
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            PUBLIC
            ${LOCAL_COMMON_DIR}/src/main/cpp/util/include
    )

    # Link poseInspection library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            contourGenerator
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            ${LOCAL_COMMON_DIR}/build/intermediates/library_jni/debug/jni/${CMAKE_ANDROID_ARCH_ABI}/libahi_common_util.so
    )
else ()
    # Link poseInspection library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            contourGenerator
            ${CPP_DEPS_LIBS}
            lib_opencv
            ${log-lib}
            -ljnigraphics
            ahi-sdk-common-android::ahi_common_util
    )
endif ()
cmake_minimum_required(VERSION 3.18.1)
project(inversion)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-experimental-isel")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=sha1")

# Add openCV
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

# Add cereal
include_directories(../../../../Common/src/main/cpp/libs/cereal/include)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(/include)

file(GLOB SEGMENTATION_UTIL_SOURCES
        *.cpp)

add_library( # Sets the name of the library.
        inversion
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        InversionJNI.cpp
        ${SEGMENTATION_UTIL_SOURCES}
        ${EXTERNAL_SOURCES})

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Add include directories from the multiscan library and 3rd party c++ dependencies.
# Note that we don't need to add the common module include directory, it is done for us by prefab.
target_include_directories(
        inversion
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Link poseInspection library with common libraries and 3rd party dependency libraries
target_link_libraries(
        inversion
        ${CPP_DEPS_LIBS}
        lib_opencv
        ${log-lib}
        -ljnigraphics
)

#Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.18.1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-experimental-isel")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -llog")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=sha1")

include_directories(../../../../Common/src/main/cpp/libs/cereal/include)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

# Get all the source files for the resources util library
file(GLOB_RECURSE RESOURCES_UTIL_SOURCES
        util/**/*.cpp
        util/**/*.c)

# Create the resources library
add_library(
        resources
        SHARED
        ResourcesJNI.cpp
        ${RESOURCES_UTIL_SOURCES}
)
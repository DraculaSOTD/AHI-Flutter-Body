cmake_minimum_required(VERSION 3.18.1)
project(bodyscan)
set(CMAKE_CXX_STANDARD 17)

# OpenCV
include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

if (DEFINED LOCAL_COMMON_DIR)
    add_subdirectory(${LOCAL_COMMON_DIR} ${LOCAL_COMMON_DIR}/build)
else ()
    # This will find all of the libraries from the common module, using prefab
    find_package(ahi-sdk-common-android REQUIRED CONFIG)
endif ()

# Get all bodyscan source files
file(GLOB BODYSCAN_SOURCES *.cpp)

# Create the bodyscan library
add_library(
        bodyscan
        SHARED
        ${BODYSCAN_SOURCES}
)

# Add include directories from bodysan library and 3rd party c++ dependencies.
# Note that we don't need to add the common module include directory, it is done for us by prefab.
target_include_directories(
        bodyscan
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CPP_UTILS_PATH}/include
        ${CPP_UTILS_PATH}/${CMAKE_ANDROID_ARCH_ABI}/include
)
# Get all 3rd party c++ libraries.
file(GLOB CPP_DEPS_LIBS "${CPP_UTILS_PATH}/${CMAKE_ANDROID_ARCH_ABI}/lib*.*")

message("cpp deps libs: ${CMAKE_ANDROID_ARCH_ABI}")

if (DEFINED LOCAL_COMMON_DIR)
    target_include_directories(
            bodyscan
            PUBLIC
            ${LOCAL_COMMON_DIR}/src/main/cpp/util/include
    )

    # Link bodyscan library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            bodyscan
            lib_opencv
            ${LOCAL_COMMON_DIR}/build/intermediates/library_jni/debug/jni/${CMAKE_ANDROID_ARCH_ABI}/libahi_common_util.so
            ${CPP_DEPS_LIBS}
    )
else ()
    # Link bodyscan library with common libraries and 3rd party dependency libraries
    target_link_libraries(
            bodyscan
            lib_opencv
            ahi-sdk-common-android::ahi_common_util
            ${CPP_DEPS_LIBS}
    )
endif ()
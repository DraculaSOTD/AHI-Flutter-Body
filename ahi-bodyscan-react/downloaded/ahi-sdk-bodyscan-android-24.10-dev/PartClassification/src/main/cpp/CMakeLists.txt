cmake_minimum_required(VERSION 3.18.1)
project(ProjectName)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-experimental-isel")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -llog")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=sha1")

set(TFLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../PartSegmentation/src/main/cpp/libs/prebuilt-tensorflow-lite")
set(TFLITE_LIBS_DIR "${TFLITE_DIR}/libs/${ANDROID_ABI}")

# Add openCV
include_directories(${OpenCV_DIR}/jni/include)
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

# Add jsoncpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/jsoncpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/jsoncpp/include)

# Add TFLite
file(GLOB TFLITE_LIBS "${TFLITE_LIBS_DIR}/lib*.*")
include_directories(${TFLITE_DIR}/include)
add_library(tensorflow-lite STATIC IMPORTED)
set_target_properties(tensorflow-lite PROPERTIES IMPORTED_LOCATION ${TFLITE_LIBS_DIR}/libtensorflow-lite.a)
add_library(tensorflow-lite-lib INTERFACE)
target_link_libraries(tensorflow-lite-lib
        INTERFACE
        ${TFLITE_LIBS}
        tensorflow-lite)

# Add AHICommon
if (DEFINED LOCAL_COMMON_DIR)
    add_subdirectory(${LOCAL_COMMON_DIR} ${LOCAL_COMMON_DIR}/build)
else ()
    # This will find all of the libraries from the common module, using prefab
    find_package(ahi-sdk-common-android REQUIRED CONFIG)
endif ()

# Add cereal
include_directories(../../../../Common/src/main/cpp/libs/cereal/include)

# Get all classification includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Get all external source files
file(GLOB EXTERNAL_SOURCES
        ../../../../Common/src/main/cpp/*.cpp
        ../../../../Common/src/main/cpp/jnihelper/*.cpp)

include_directories(../../../../Common/src/main/cpp)

# Get all classification source files
file(GLOB CLASSIFICATION_SOURCES
        /include/*.cpp
        *.cpp)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create the classification library
add_library(
        classification
        SHARED
        ${CLASSIFICATION_SOURCES}
        ${EXTERNAL_SOURCES}
)

# Add logging library
find_library(log-lib
        log)

if (DEFINED LOCAL_COMMON_DIR)
    target_include_directories(
            classification
            lib_opencv
            ${log-lib}
            -ljnigraphics
            jsoncpp
            tensorflow-lite-lib
            PUBLIC
            ${LOCAL_COMMON_DIR}/src/main/cpp/util/include
    )

    # Link classification library with dependency libraries
    target_link_libraries(
            classification
            lib_opencv
            ${log-lib}
            -ljnigraphics
            jsoncpp
            tensorflow-lite-lib
            ${LOCAL_COMMON_DIR}/build/intermediates/library_jni/debug/jni/${CMAKE_ANDROID_ARCH_ABI}/libahi_common_util.so
    )
else ()
    # Link classification library with dependency libraries
    target_link_libraries(
            classification
            lib_opencv
            ${log-lib}
            -ljnigraphics
            jsoncpp
            tensorflow-lite-lib
            ahi-sdk-common-android::ahi_common_util
    )
endif ()


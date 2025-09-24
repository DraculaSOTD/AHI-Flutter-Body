
# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

PROJECT(jsoncpp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DJSON_USE_EXCEPTION=0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# General source files
file(GLOB SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/lib_json/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/lib_json/json_valueiterator.inl
	${CMAKE_CURRENT_SOURCE_DIR}/src/lib_json/version.h.in
	)

add_library( # Sets the name of the library.
             jsoncpp

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${SOURCE_FILES}
            )

target_link_libraries( # Specifies the target library.
                       jsoncpp

                        -Wl,-x
                        -Wl,-z,relro
                        -Wl,-z,now

                       ${android}
                     )

# export(PACKAGE jsoncpp)

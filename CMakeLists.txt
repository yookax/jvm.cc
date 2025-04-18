cmake_minimum_required(VERSION 3.30)
project(jvmcc)

#set(CMAKE_CXX_FLAGS "-std=c++23 -m64 -pedantic -Wfatal-errors\
#        -Wall -Wextra -Wcomment -Wshadow -Wwrite-strings -Wpointer-arith \
#        -Wunreachable-code -Wreturn-type -Wcast-qual \
#        -Wswitch-default -Wswitch-enum -Wuninitialized -Winit-self \
#        -Wparentheses -Wunused-macros")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

message(STATUS "C++: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Compilation flags: " ${CMAKE_CXX_FLAGS})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8 /execution-charset:utf-8 /experimental:module ") # /W4
endif()

find_package(ZLIB)
if(ZLIB_FOUND)
    add_definitions(-DENABLE_ZLIB)
#    target_include_directories(clib PRIVATE ${CURL_INCLUDE_DIR})
#    target_link_libraries(curltest ${CURL_LIBRARY})
endif()

file(GLOB_RECURSE
        SOURCE_CXX_FILES
        CONFIGURE_DEPENDS
        src/*.cpp
)

file(GLOB_RECURSE
        SOURCE_MODULE_FILES
        CONFIGURE_DEPENDS
        src/*.ixx
)

# 生成可运行的虚拟机
add_executable(jvmcc ${SOURCE_CXX_FILES})

# 创建CXX_MODULES类型的FILE_SET
target_sources(jvmcc
        PUBLIC
        FILE_SET file_set TYPE CXX_MODULES
        FILES ${SOURCE_MODULE_FILES}
)
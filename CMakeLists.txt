cmake_minimum_required(VERSION 3.30)
project(jvmcc)

#set(CMAKE_CXX_FLAGS "-std=c++23 -m64 -pedantic -Wfatal-errors\
#        -Wall -Wextra -Wcomment -Wshadow -Wwrite-strings -Wpointer-arith \
#        -Wunreachable-code -Wreturn-type -Wcast-qual \
#        -Wswitch-default -Wswitch-enum -Wuninitialized -Winit-self \
#        -Wparentheses -Wunused-macros")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(jvm PRIVATE -fmodules-ts)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /experimental:module")
endif()

set (SOURCE_FILES src/slot.cpp src/jvm.cpp src/jni.cpp src/native/init_native.cpp src/native/unsafe.cpp
        src/jmm.cpp src/interpreter.cpp src/classfile/descriptor.cpp src/classfile/module.cpp
        src/encoding.cpp src/classfile/poly.cpp src/reference.cpp
        src/runtime/thread.cpp src/runtime/heap.cpp src/dll.cpp src/classfile/method.cpp
        src/classfile/field.cpp src/classfile/constant_pool.cpp
        src/classfile/class_loader.cpp src/primitive.cpp src/init.cpp
        src/object/object.cpp src/object/array.cpp src/object/string.cpp src/object/box.cpp
        src/object/reflect.cpp src/classfile/class.cpp src/classfile/array_class.cpp src/exception.cpp
        src/classfile/invoke.cpp src/convert.cpp src/native/method_handle_natives.cpp
        src/native/scoped_memory_access.cpp src/native/var_handle.cpp src/native/method_handle.cpp
        src/object/allocator.cpp src/sysinfo.cpp
)

if (MSVC)
    add_compile_options("/utf-8")
endif()

# 这里生成 jvm.dll
# jdk/bin/zip.dll 和 jdk/bin/java.dll 均依赖此dll
# 如果加载以上两个dll失败，可以用‘Dependency Walker’之类的工具查看下这两个dll所依赖的函数在jvm.dll中是否实现。
# 如果有函数没有实现，可以在jvm.cpp中实现之。
# 如果有Debug加载zip.dll和java.dll成功，而Release版失败的情况，可以考虑下是不是Release版是不是把jvm.cpp中没有使用的函数优化掉了。
# 总之一句话，无论Debug版还是Release版jvm.dll中导出所有zip.dll和java.dll中所依赖的函数。
add_library(jvm SHARED ${SOURCE_FILES})

SET_TARGET_PROPERTIES(jvm PROPERTIES OUTPUT_NAME "jvm" PREFIX "")

# 生成可运行的虚拟机
add_executable(jvmcc src/vm_client.cpp)
target_link_libraries(jvmcc jvm)

# 创建CXX_MODULES类型的FILE_SET
target_sources(jvm
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS src
        FILES
        src/classfile/classfile.ixx src/runtime/runtime.ixx
        src/object/object.ixx src/vmstd.ixx src/constants.ixx
        src/classfile/bytecode_reader.ixx src/classfile/invoke.ixx
        src/classfile/class_loader.ixx src/classfile/poly.ixx
        src/runtime/heap.ixx src/reference.ixx src/dll.ixx
)

#target_sources(jvmcc
#        PUBLIC
#        FILE_SET CXX_MODULES
#        BASE_DIRS src
#        FILES
#        src/classfile/classfile.ixx src/runtime/runtime.ixx
#        src/object/object.ixx src/vmstd.ixx src/classfile/constant_pool.ixx
#)

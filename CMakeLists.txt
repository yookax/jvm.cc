cmake_minimum_required(VERSION 3.30)
project(jvmcc)

#set(CMAKE_CXX_FLAGS "-std=c++23 -m64 -pedantic -Wfatal-errors\
#        -Wall -Wextra -Wcomment -Wshadow -Wwrite-strings -Wpointer-arith \
#        -Wunreachable-code -Wreturn-type -Wcast-qual \
#        -Wswitch-default -Wswitch-enum -Wuninitialized -Winit-self \
#        -Wparentheses -Wunused-macros")

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_SCAN_FOR_MODULES ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8 /experimental:module ") # /W4
endif()

# 添加 zlib 子目录，引入 zlib 的 CMakeLists.txt
# add_subdirectory(lib/zlib-1.3.1)

# 包含 minizip 头文件目录
# target_include_directories(jvmcc PRIVATE lib/zlib-1.3.1/minizip)

add_subdirectory(lib/minizip)

set (SOURCE_FILES ${MINIZIP_FILES} src/jvm.cpp src/jni.cpp src/native/unsafe.cpp
        src/jmm.cpp src/interpreter.cpp src/classfile/descriptor.cpp src/classfile/module.cpp
        src/encoding.cpp src/classfile/poly.cpp src/reference.cpp
        src/runtime/thread.cpp src/runtime/heap.cpp src/classfile/method.cpp
        src/classfile/field.cpp src/classfile/constant_pool.cpp
        src/classfile/class_loader.cpp src/init.cpp
        src/object/object.cpp src/object/array.cpp src/object/string.cpp src/object/box.cpp
        src/object/reflect.cpp src/classfile/class.cpp src/classfile/array_class.cpp src/exception.cpp
        src/classfile/invoke.cpp src/native/method_handle_natives.cpp
        src/native/scoped_memory_access.cpp src/native/var_handle.cpp src/native/method_handle.cpp
        src/object/allocator.cpp
        src/native/java/lang/Thread.cpp
        src/native/native.cpp
        src/native/java/lang/Class.cpp
        src/native/java/lang/Object.cpp
        src/native/java/lang/reflect/Array.cpp
        src/native/java/lang/System.cpp
        src/native/java/lang/Runtime.cpp
        src/native/java/lang/Float.cpp
        src/native/java/lang/Double.cpp
        src/native/jdk/internal/misc/Unsafe.cpp
        src/native/jdk/internal/misc/CDS.cpp
        src/native/java/lang/ClassLoader.cpp
        src/native/java/lang/Module.cpp
        src/native/java/lang/Throwable.cpp
        src/native/java/lang/String.cpp
        src/native/jdk/internal/reflect/Reflection.cpp
        src/native/java/io/FileInputStream.cpp
        src/native/java/io/FileOutputStream.cpp
        src/native/java/io/FileDescriptor.cpp
        src/native/java/lang/ref/Reference.cpp
        src/native/java/lang/invoke/MethodHandleNatives.cpp
        src/native/jdk/internal/misc/Signal.cpp
        src/native/java/lang/invoke/MethodHandle.cpp
        src/native/java/io/WinNTFileSystem.cpp
        src/native/sun/nio/ch/IOUtil.cpp
        src/native/jdk/internal/util/SystemProps-Raw.cpp
        src/native/jdk/internal/misc/VM.cpp
        src/native/sun/io/Win32ErrorMode.cpp
        src/native/jdk/internal/loader/BootLoader.cpp
        src/native/jdk/internal/loader/NativeLibraries.cpp
)

set (TEST_SOURCE_FILES
        test-cpp/test.cpp test-cpp/convert_test.cpp test-cpp/slot_test.cpp
        test-cpp/descriptor_test.cpp test-cpp/heap_test.cpp test-cpp/class_loader_test.cpp
        test-cpp/object_test.cpp test-cpp/class_test.cpp test-cpp/foo_test.cpp
        test-cpp/encoding_test.cpp
)

# 这里生成 jvm.dll
# jdk/bin/zip.dll 和 jdk/bin/java.dll 均依赖此dll
# 如果加载以上两个dll失败，可以用‘Dependency Walker’之类的工具查看下这两个dll所依赖的函数在jvm.dll中是否实现。
# 如果有函数没有实现，可以在jvm.cpp中实现之。
# 如果有Debug加载zip.dll和java.dll成功，而Release版失败的情况，可以考虑下是不是Release版是不是把jvm.cpp中没有使用的函数优化掉了。
# 总之一句话，无论Debug版还是Release版jvm.dll中导出所有zip.dll和java.dll中所依赖的函数。
add_library(jvm SHARED ${SOURCE_FILES} ${TEST_SOURCE_FILES})
SET_TARGET_PROPERTIES(jvm PROPERTIES OUTPUT_NAME "jvm" PREFIX "")

# 生成可运行的虚拟机
add_executable(jvmcc src/vm_client.cpp)
target_link_libraries(jvmcc jvm)

add_executable(jvmcc-test test-cpp/test.cpp)
target_link_libraries(jvmcc-test jvm)

# 创建CXX_MODULES类型的FILE_SET
target_sources(jvm
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS src
        FILES
        src/classfile/classfile.ixx src/runtime/runtime.ixx src/slot.ixx
        src/object/object.ixx src/constants.ixx src/util/primitive.ixx
        src/classfile/bytecode_reader.ixx src/classfile/invoke.ixx
        src/classfile/class_loader.ixx src/classfile/poly.ixx src/util/sysinfo.ixx
        src/runtime/heap.ixx src/reference.ixx src/dll.ixx src/util/convert.ixx
        src/interpreter.ixx src/encoding.ixx src/exception.ixx src/util/access_flags.ixx
        src/native/native.ixx
)

#target_sources(jvmcc
#        PUBLIC
#        FILE_SET CXX_MODULES
#        BASE_DIRS src
#        FILES
#        src/classfile/classfile.ixx src/runtime/runtime.ixx
#        src/object/object.ixx src/vmstd.ixx src/classfile/constant_pool.ixx
#)

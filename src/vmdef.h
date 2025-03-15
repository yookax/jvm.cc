#ifndef VMDEF_H
#define VMDEF_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cassert>
#include <string>
#include <vector>

#ifndef PATH_MAX
    #ifdef MAX_PATH
        #define PATH_MAX MAX_PATH
    #else
        #define PATH_MAX 260
    #endif
#endif
 
#define VM_VERSION "1.0" // version of this jvm, a string.
#define JAVA_COMPAT_VERSION "1.8.0_162" // todo

// jvm 最大支持的classfile版本
#define JVM_MAX_CLASSFILE_MAJOR_VERSION 60
#define JVM_MAX_CLASSFILE_MINOR_VERSION 65535

// size of heap
#define VM_HEAP_SIZE (512*1024*1024) // 512Mb

// every thread has a vm stack
#define VM_STACK_SIZE (16*1024*1024) // 16Mb

// jni 局部引用表默认最大容量
#define JNI_LOCAL_REFERENCE_TABLE_MAX_CAPACITY 512

/*
 * Java虚拟机中的整型类型的取值范围如下：
 * 1. 对于byte类型， 取值范围[-2e7, 2e7 - 1]。
 * 2. 对于short类型，取值范围[-2e15, 2e15 - 1]。
 * 3. 对于int类型，  取值范围[-2e31, 2e31 - 1]。
 * 4. 对于long类型， 取值范围[-2e63, 2e63 - 1]。
 * 5. 对于char类型， 取值范围[0, 65535]。
 */
using jbyte    = int8_t;
using jboolean = jbyte; // 本虚拟机实现，byte 和 boolean 用同一类型
using jbool    = jboolean;
using jchar    = uint16_t;
using jshort   = int16_t;
using jint     = int32_t;
using jlong    = int64_t;
using jfloat   = float;
using jdouble  = double;

#define jtrue  1
#define jfalse 0

using jsize = jint;

#define JINT_TO_JBOOL(_i)  ((_i) != 0 ? true : false)
#define JINT_TO_JBYTE(_i)  ((jbyte)((_i) & 0xff))
#define JINT_TO_JCHAR(_i)  ((jchar)((_i) & 0xffff))
#define JINT_TO_JSHORT(_i) ((jshort)((_i) & 0xffff))

// s: signed
using s1 = int8_t;
using s2 = int16_t;
using s4 = int32_t;

// u: unsigned
using u1 = uint8_t;
using u2 = uint16_t;
using u4 = uint32_t;
using u8 = uint64_t;

class Object;
class Class;
class ArrayClass;
class Method;
class Field;
class Annotation;

using jref       = Object*; // JVM 中的引用类型
using jstrRef    = jref;    // java.lang.String 的引用
using jarrRef    = jref;    // Array 的引用
using jobjArrRef = jref;    // java.lang.Object Array 的引用
using jclsRef    = jref;    // java.lang.Class 的引用

typedef char utf8_t;
typedef jchar unicode_t;

class Heap;
extern Heap *g_heap;

extern std::string g_java_home;

extern u2 g_classfile_major_version;
extern u2 g_classfile_manor_version;

// The system Thread group.
extern Object *g_sys_thread_group;

#define BOOT_CLASS_LOADER nullptr
extern Object *g_app_class_loader;
extern Object *g_platform_class_loader;

extern bool g_vm_initing;

template <typename T> concept
JavaValueType = std::is_same_v<T, jint>
                || std::is_same_v<T, jbyte> || std::is_same_v<T, jbool>
                || std::is_same_v<T, jchar> || std::is_same_v<T, jshort>
                || std::is_same_v<T, jfloat> || std::is_same_v<T, jlong>
                || std::is_same_v<T, jdouble> || std::is_same_v<T, jref>;

struct Property {
    const utf8_t *name;
    const utf8_t *value;
    Property(const utf8_t *name0, const utf8_t *value0): name(name0), value(value0) {
        assert(name != nullptr);
        assert(value != nullptr);
    }
};

extern std::vector<Property> g_properties;

struct InitArgs {
    bool asyncgc = false;
    bool verbosegc = false;
    bool verbosedll = false;
    bool verboseclass = false;

    // Whether compaction has been given on the command line, and the value if it has
    bool compact_specified = false;
    int do_compact = false;
    bool trace_jni_sigs = false;

    char *classpath = nullptr;

    char *bootpath = nullptr;
    char *bootpath_a = nullptr;
    char *bootpath_p = nullptr;
    char *bootpath_c = nullptr;
    char *bootpath_v = nullptr;

    int java_stack = VM_STACK_SIZE;
    unsigned long min_heap = VM_HEAP_SIZE;
    unsigned long max_heap = VM_HEAP_SIZE;

    Property *commandline_props;
    int props_count = 0;

    void *main_stack_base;

    /* JNI invocation API hooks */

    int (* vfprintf)(FILE *stream, const char *fmt, va_list ap) = std::vfprintf;
    void (* exit)(int status) = std::exit;
    void (* abort)() = std::abort;
};

/* This number, mandated by the JVM spec as 255,
 * is the maximum number of slots
 * that any Java method can receive in its argument list.
 * It limits both JVM signatures and method type objects.
 * The longest possible invocation will look like
 * staticMethod(arg1, arg2, ..., arg255) or
 * x.virtualMethod(arg1, arg2, ..., arg254).
 *
 * jvms规定函数最多有255个参数，this也算，long和double占两个长度
 */
#define MAX_JVM_ARITY 255

// jvms数组最大维度为255
#define ARRAY_MAX_DIMENSIONS 255

#define MAIN_THREAD_NAME "main" // name of main thread
#define GC_THREAD_NAME "gc"     // name of gc thread

#define FILE_LINE_STR (__FILE__ + std::string(": ") + std::to_string(__LINE__))

#define printvm(...) \
do { \
    printf("%s: %d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); \
} while(false)

// #define println(...) do { printvm(__VA_ARGS__); printf("\n"); } while(false)

/* --------------------- 配置日志 -------------------- */

#define LOG_LEVEL_ERR     0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_DEBUG   2
#define LOG_LEVEL_TRACE   3
#define LOG_LEVEL_VERBOSE 4 

// 日志级别，默认为 LOG_LEVEL_ERR
#define LOG_LEVEL LOG_LEVEL_ERR 

#define ERR     printvm
#define WARN    printvm
#define DEBUG   printvm
#define TRACE   printvm
#define VERBOSE printvm

#if (LOG_LEVEL < LOG_LEVEL_VERBOSE)
#undef VERBOSE
#define VERBOSE(...) ((void) 0)
#endif

#if (LOG_LEVEL < LOG_LEVEL_TRACE)
#undef TRACE
#define TRACE(...) ((void) 0)
#endif

#if (LOG_LEVEL < LOG_LEVEL_DEBUG)
#undef DEBUG
#define DEBUG(...) ((void) 0)
#endif

#if (LOG_LEVEL < LOG_LEVEL_WARNING)
#undef WARN
#define WARN(...) ((void) 0)
#endif

// -------------------------------------------------

enum ExitCode {
    EXIT_CODE_SUCCESS = 0,
    EXIT_CODE_UNCAUGHT_JAVA_EXCEPTION = -1,
    EXIT_CODE_UNREACHABLE = -2,
    EXIT_CODE_UNIMPLEMENTED = -3,
    EXIT_CODE_UNKNOWN_ERROR = -4,
};

// 出现异常，退出jvm
#define panic(...) \
do { \
    printvm("JVM panic! "); \
    printf(__VA_ARGS__); \
    exit(EXIT_CODE_UNKNOWN_ERROR); \
} while(false)

// 代码出现致命错误，
// 打印信息，退出JVM
#define UNREACHABLE(...) \
do { \
    printvm("Unreachable.\n"); \
    printf(__VA_ARGS__); \
    exit(EXIT_CODE_UNREACHABLE); \
} while(false)

// 未实现的接口、功能等
// 打印位置，退出JVM
#define unimplemented \
{ \
    printf("Unimplemented. %s: %d. %s\n", __FILE__, __LINE__, __func__); \
    exit(EXIT_CODE_UNIMPLEMENTED); \
}

#endif //VMDEF_H

#include <cassert>
#include <cstdio>
#include <ctime>
#include "vmdef.h"
#include "jni.h"
#include "jmm.h"

import std.core;
import std.threading;
import slot;
import sysinfo;
import runtime;
import object;
import heap;
import classfile;
import class_loader;
import reference;
import dll;
import constants;
import encoding;
import interpreter;

using namespace std;
using namespace slot;
using namespace utf8;
using namespace java_lang_String;


// 需要实现的函数参考 openjdk\src\hotspot\share\include\jvm.h

#define JVM_MIRROR(_jclass) ((jclsRef) _jclass)->jvm_mirror

/*
 * This file contains additional functions exported from the VM.
 * These functions are complementary to the standard JNI support.
 * There are three parts to this file:
 *
 * First, this file contains the VM-related functions needed by native
 * libraries in the standard Java API. For example, the java.lang.Object
 * class needs VM-level functions that wait for and notify monitors.
 *
 * Second, this file contains the functions and constant definitions
 * needed by the byte code verifier and class file format checker.
 * These functions allow the verifier and format checker to be written
 * in a VM-independent way.
 *
 * Third, this file contains various I/O and nerwork operations needed
 * by the standard Java I/O and network APIs. A part of these APIs,
 * namely the jio_xxxprintf functions, are included from jvm_io.h.
 */

/*
 * Bump the version number when either of the following happens:
 *
 * 1. There is a change in JVM_* functions.
 *
 * 2. There is a change in the contract between VM and Java classes.
 *    For example, if the VM relies on a new private field in Thread
 *    class.
 */

#define JVM_INTERFACE_VERSION 6

JNIEXPORT jint JNICALL
JVM_GetInterfaceVersion(void) {
    TRACE("JVM_GetInterfaceVersion()");
    return JVM_INTERFACE_VERSION;
}

/*************************************************************************
 PART 1: Functions for Native Libraries
 ************************************************************************/
/*
 * java.lang.Object
 */
JNIEXPORT jint JNICALL
JVM_IHashCode(JNIEnv *env, jobject obj) {
    TRACE("JVM_IHashCode(env=%p, obj=%p)", env, obj);
    return (jint)(intptr_t)obj; // todo 实现错误。改成当前的时间如何。
}

JNIEXPORT void JNICALL
JVM_MonitorWait(JNIEnv *env, jobject obj, jlong ms) {
    TRACE("JVM_MonitorWait(env=%p, obj=%p, ms=%ld)", env, obj, ms);
    ((jref)obj)->wait(ms);
}

JNIEXPORT void JNICALL
JVM_MonitorNotify(JNIEnv *env, jobject obj) {
    TRACE("JVM_MonitorNotify(env=%p, obj=%p)", env, obj);
    ((jref)obj)->notify();
}

JNIEXPORT void JNICALL
JVM_MonitorNotifyAll(JNIEnv *env, jobject obj) {
    TRACE("JVM_MonitorNotifyAll(env=%p, obj=%p)", env, obj);
    ((jref)obj)->notify_all();
}

JNIEXPORT jobject JNICALL
JVM_Clone(JNIEnv *env, jobject _obj) {
    TRACE("JVM_Clone(env=%p, obj=%p)", env, _obj);
    jref obj = (jref) _obj;
    if (!obj->clazz->is_subclass_of(load_boot_class("java/lang/Cloneable"))) {
        JNI_THROW(env, "java/lang/CloneNotSupportedException", nullptr); // todo msg
        return nullptr;
    }
    return (jobject) obj->clone();
}

/*
 * java.lang.String
 */
JNIEXPORT jstring JNICALL 
JVM_InternString(JNIEnv *env, jstring str) {
    TRACE("JVM_InternString(env=%p, str=%p)", env, str);
    assert(g_string_class == ((jstrRef) str)->clazz);
    return (jstring) intern((jstrRef) str);
}

/*
 * java.lang.System
 * The current time as UTC milliseconds from the epoch(1970-1-1-00:00:00 UTC).
 */
JNIEXPORT jlong JNICALL
JVM_CurrentTimeMillis(JNIEnv *env, jclass ignored) {
    TRACE("JVM_CurrentTimeMillis(env=%p, ignored=%p)", env, ignored);
    // Get the current time point
    auto now = std::chrono::system_clock::now();
    // Calculate the duration from the epoch to the current time point
    auto duration = now.time_since_epoch();
    // Convert the duration to milliseconds
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliseconds;
}

/*
 * 返回最准确的可用系统计时器的当前值，以毫微秒为单位。
 * 此方法只能用于测量已过的时间，与系统或钟表时间的其他任何时间概念无关。
 * 返回值表示从某一固定但任意的时间算起的毫微秒数（或许从以后算起，所以该值可能为负）。
 * 此方法提供毫微秒的精度，但不是必要的毫微秒的准确度。它对于值的更改频率没有作出保证。
 * 在取值范围大于约 292 年（263 毫微秒）的连续调用的不同点在于：由于数字溢出，将无法准确计算已过的时间。
 */
JNIEXPORT jlong JNICALL
JVM_NanoTime(JNIEnv *env, jclass ignored) {
    TRACE("JVM_NanoTime(env=%p)", env);
    // todo

//    timespec ts{};
//    clock_gettime(CLOCK_REALTIME, &ts);  //获取相对于1970到现在的秒数
//    return (jlong) ts.tv_sec * 1000000000 + ts.tv_nsec;

    auto now = std::chrono::high_resolution_clock::now();
    return now.time_since_epoch().count();
}

JNIEXPORT jlong JNICALL
JVM_GetNanoTimeAdjustment(JNIEnv *env, jclass ignored, jlong offset_secs) {
    TRACE("JVM_GetNanoTimeAdjustment(env=%p, offset_secs=%lld)", env, offset_secs);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_ArrayCopy(JNIEnv *env, jclass ignored, jobject _src, jint src_pos,
              jobject _dst, jint dst_pos, jint length) {
    TRACE("JVM_ArrayCopy(env=%p, ignored=%p, src=%p, src_pos=%d, dst=%p, dst_pos=%d, length=%d)",
                    env, ignored, _src, src_pos, _dst, dst_pos, length);

    auto dst = (jarrRef) _dst;
    auto src = (jarrRef) _src;
    assert(dst->is_array_object());
    assert(src->is_array_object());
    array_copy(dst, dst_pos, src, src_pos, length);
}

/*
 * Gather the VM and command line properties and return as a String[].
 * The array indices are alternating key/value pairs
 * supplied by the VM including those defined on the command line
 * using -Dkey=value that may override the platform defined value.
 *
 * Note: The platform encoding must have been set.
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetProperties(JNIEnv *env) {
    TRACE("JVM_GetProperties(env=%p)", env);

    jarrRef prop_array = Allocator::string_array(g_properties.size()*2);
    int i = 0;
    for (Property &p : g_properties) {
        prop_array->setRefElt(i++, Allocator::string(p.name));
        prop_array->setRefElt(i++, Allocator::string(p.value));
    }
    return (jobjectArray) prop_array;
}

// JNIEXPORT jobject JNICALL
// JVM_InitProperties(JNIEnv *env, jobject props)
// {
//     TRACE("JVM_InitProperties(env=%p, props=%p)", env, props);
//     // todo init
//     Method *setProperty = lookup_inst_method(((jref) props)->clazz,
//             "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");

//     for (int i = 0; i < g_properties_count; i++) {
//         assert(g_properties[i].name != nullptr && g_properties[i].value!= nullptr);
//         exec_java_func3(setProperty, props, allocString(g_properties[i].name), allocString(g_properties[i].value));
//     }

//     return props;
// }

// /*
//  * java.io.File
//  */
// JNIEXPORT void JNICALL
// JVM_OnExit(void (*func)(void))
// {
//     TRACE("JVM_OnExit(func=%p)", func);
//     UNIMPLEMENTED
// }

// /*
//  * java.lang.Runtime
//  */
// JNIEXPORT void JNICALL
// JVM_Exit(jint code)
// {
//     TRACE("JVM_Exit(code=%d)", code);
//     unimplemented
// }

/*
 * java.lang.Runtime
 */

/*
 * Notify the VM that it's time to halt.
 *
 * static native void beforeHalt();
 */
JNIEXPORT void JNICALL
JVM_BeforeHalt() {
    TRACE("JVM_BeforeHalt");
    unimplemented
}

/*
 * The halt method is synchronized on the halt lock
 * to avoid corruption of the delete-on-shutdown file list.
 * It invokes the true native halt method.
 *
 * static native void halt0(int status);
 */
JNIEXPORT void JNICALL
JVM_Halt(jint code) {
    TRACE("JVM_Halt(code=%d)", code);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_GC(void) {
    TRACE("JVM_GC()");
    unimplemented
}

/* Returns the number of real-time milliseconds that have elapsed since the
 * least-recently-inspected heap object was last inspected by the garbage
 * collector.
 *
 * For simple stop-the-world collectors this value is just the time
 * since the most recent collection.  For generational collectors it is the
 * time since the oldest generation was most recently collected.  Other
 * collectors are free to return a pessimistic estimate of the elapsed time, or
 * simply the time since the last full collection was performed.
 *
 * Note that in the presence of reference objects, a given object that is no
 * longer strongly reachable may have to be inspected multiple times before it
 * can be reclaimed.
 */
JNIEXPORT jlong JNICALL
JVM_MaxObjectInspectionAge(void) {
    TRACE("JVM_MaxObjectInspectionAge()");
    unimplemented 
}

// JNIEXPORT void JNICALL
// JVM_TraceInstructions(jboolean on)
// {
//     TRACE("JVM_TraceInstructions(on=%d)", on);
//     UNIMPLEMENTED
// }

// JNIEXPORT void JNICALL
// JVM_TraceMethodCalls(jboolean on)
// {
//     TRACE("JVM_TraceMethodCalls(on=%d)", on);
//     UNIMPLEMENTED
// }

/* todo
 * totalMemory()这个方法返回的是java虚拟机现在已经从操作系统那里挖过来的内存大小，
 * 也就是java虚拟机这个进程当时所占用的所有内存。如果在运行java的时候没有添加-Xms参数，那么，在java程序运行的过程的，
 * 内存总是慢慢的从操作系统那里挖的，基本上是用多少挖多少，直 挖到maxMemory()为止，所以totalMemory()是慢慢增大的。
 * 如果用了-Xms参数，程序在启动的时候就会无条件的从操作系统中挖- Xms后面定义的内存数，然后在这些内存用的差不多的时候，再去挖。
 */
JNIEXPORT jlong JNICALL
JVM_TotalMemory(void) {
    TRACE("JVM_TotalMemory()");
    return g_heap->get_size();
}

JNIEXPORT jlong JNICALL
JVM_FreeMemory(void) {
    TRACE("JVM_FreeMemory()");
    return g_heap->count_free_memory();
}

/* todo
 * maxMemory()这个方法返回的是java虚拟机（这个进程）能构从操作系统那里挖到的最大的内存，以字节为单位，如果在运行java程序的时 候，没有添加-Xmx参数，那么就是64兆，也就是说maxMemory()返回的大约是64*1024*1024字节，这是java虚拟机默认情况下能 从操作系统那里挖到的最大的内存。如果添加了-Xmx参数，将以这个参数后面的值为准，例如java -cp ClassPath -Xmx512m ClassName，那么最大内存就是512*1024*0124字节。
 */
JNIEXPORT jlong JNICALL
JVM_MaxMemory(void) {
    TRACE("JVM_MaxMemory()");
    return VM_HEAP_SIZE; // todo
}

JNIEXPORT jint JNICALL
JVM_ActiveProcessorCount(void) {
    TRACE("JVM_ActiveProcessorCount()");
    return processor_number();
}

JNIEXPORT jboolean JNICALL
JVM_IsUseContainerSupport(void)
{
    TRACE("JVM_IsUseContainerSupport");
    unimplemented
}

JNIEXPORT void * JNICALL
JVM_LoadZipLibrary() {
    unimplemented
}

JNIEXPORT void * JNICALL
JVM_LoadLibrary(const char *name) {
    TRACE("JVM_LoadLibrary(name=%s)", name);
    void *handle = open_library(name);
    if (handle == nullptr)
        ERR("load library(%s) failed\n", name);
    return handle;
}

JNIEXPORT void JNICALL
JVM_UnloadLibrary(void *handle) {
    TRACE("JVM_UnloadLibrary(handle=%p)", handle);
    close_library(handle);
}

JNIEXPORT void * JNICALL
JVM_FindLibraryEntry(void *handle, const char *name) {
    TRACE("JVM_FindLibraryEntry(handle=%p, name=%s)", handle, name);
    void *p = find_library_entry(handle, name);
    return p;
}

JNIEXPORT jboolean JNICALL
JVM_IsSupportedJNIVersion(jint version) {
    TRACE("JVM_IsSupportedJNIVersion(version=%d)", version);
    bool b = isSupportedJNIVersion(version);
    return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jobjectArray JNICALL
JVM_GetVmArguments(JNIEnv *env) {
    TRACE("JVM_GetVmArguments(env=%p)", env);
    unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsPreviewEnabled(void) {
    unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsContinuationsSupported(void) {
    unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsForeignLinkerSupported(void) {
    unimplemented
}

/*
 * Initialize archived static fields in the given Class using archived
 * values from CDS dump time. Also initialize the classes of objects in
 * the archived graph referenced by those fields.
 *
 * Those static fields remain as uninitialized if there is no mapped CDS
 * java heap data or there is any error during initialization of the
 * object class in the archived graph.
 *
 * jdk\internal\misc\CDS.java
 * CDS: Class Data Sharing
 */
JNIEXPORT void JNICALL
JVM_InitializeFromArchive(JNIEnv* env, jclass cls) {
    TRACE("JVM_InitializeFromArchive(env=%p, cls=%p)", env, cls);
    // unimplemented

    // todo 这个函数是干嘛的？？？？？

    Class *c = JVM_MIRROR(cls);
    // todo
}

JNIEXPORT void JNICALL
JVM_RegisterLambdaProxyClassForArchiving(JNIEnv* env, jclass caller,
                                         jstring invokedName,
                                         jobject invokedType,
                                         jobject methodType,
                                         jobject implMethodMember,
                                         jobject instantiatedMethodType,
                                         jclass lambdaProxyClass) {
    TRACE("JVM_RegisterLambdaProxyClassForArchiving(env=%p)", env); // todo
    unimplemented
}

JNIEXPORT jclass JNICALL
JVM_LookupLambdaProxyClassFromArchive(JNIEnv* env, jclass caller,
                                      jstring invokedName,
                                      jobject invokedType,
                                      jobject methodType,
                                      jobject implMethodMember,
                                      jobject instantiatedMethodType,
                                      jboolean initialize) {
    TRACE("JVM_LookupLambdaProxyClassFromArchive(env=%p)", env); // todo
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_GetCDSConfigStatus() {
    unimplemented
}

JNIEXPORT void JNICALL
JVM_LogLambdaFormInvoker(JNIEnv* env, jstring line) {
    unimplemented
}

JNIEXPORT void JNICALL
JVM_DumpClassListToFile(JNIEnv* env, jstring fileName) {
    unimplemented
}

JNIEXPORT void JNICALL
JVM_DumpDynamicArchive(JNIEnv* env, jstring archiveName) {
    unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsCDSDumpingEnabled(JNIEnv* env) {
    TRACE("JVM_IsCDSDumpingEnabled(env=%p)", env);
    // unimplemented
    return false; // todo 这个函数是干嘛的？？？？？
}

JNIEXPORT jboolean JNICALL
JVM_IsDynamicDumpingEnabled(JNIEnv* env) {
    TRACE("JVM_IsDynamicDumpingEnabled(env=%p)", env);
    unimplemented
}

// defined in jdk\internal\misc\CDS.java
JNIEXPORT jboolean JNICALL
JVM_IsSharingEnabled(JNIEnv* env) {
    TRACE("JVM_IsSharingEnabled(env=%p)", env);
    // unimplemented
    return false; // todo 这个函数是干嘛的？？？？？
}

// defined in jdk\internal\misc\CDS.java
// JNIEXPORT void *JNICALL
// JVM_LogLambdaFormInvoker(JNIEnv* env, jstring line) {
//     TRACE("JVM_LogLambdaFormInvoker()");
//     unimplemented
// }

JNIEXPORT jboolean JNICALL
JVM_IsDumpingClassList(JNIEnv* env) {
    TRACE("JVM_IsDumpingClassList(env=%p)", env);

    // unimplemented
    return false; // todo 这个函数是干嘛的？？？？？
}

/*
 * Returns a predictable "random" seed derived from the VM's build ID and version,
 * to be used by java.util.ImmutableCollections to ensure that archived
 * ImmutableCollections are always sorted the same order for the same VM build.
 *
 * jdk\internal\misc\CDS.java
 */
JNIEXPORT jlong JNICALL
JVM_GetRandomSeedForDumping() {
    TRACE("JVM_GetRandomSeedForDumping()");
    return static_cast<unsigned int>(std::time(nullptr));
}

// /*
//  * java.lang.Float and java.lang.Double
//  */
// JNIEXPORT jboolean JNICALL
// JVM_IsNaN(jdouble d)
// {
//     TRACE("JVM_IsNaN(d=%f)", d);
//     unimplemented
// }

/*
 * java.lang.Throwable
 */
JNIEXPORT void JNICALL
JVM_FillInStackTrace(JNIEnv *env, jobject _throwable) {
    TRACE("JVM_FillInStackTrace(env=%p, throwable=%p)", env, _throwable);

    jref throwable = (jref) _throwable;
    Thread *thread = get_current_thread();

    Frame *frame = thread->top_frame;
    int num = thread->count_stack_frames();
    /*
     * 栈顶两帧正在执行 fillInStackTrace(int) 和 fillInStackTrace() 方法，所以需要跳过这两帧。
     * 这两帧下面的几帧正在执行异常类的构造函数，所以也要跳过。
     * 具体要跳过多少帧数则要看异常类的继承层次。
     *
     * (RuntimeException extends Exception extends Throwable extends Object)
     *
     * 比如一个异常抛出示例
     * java.lang.RuntimeException: BAD!
     * at exception/UncaughtTest.main(UncaughtTest.java:6)
     * at exception/UncaughtTest.foo(UncaughtTest.java:10)
     * at exception/UncaughtTest.bar(UncaughtTest.java:14)
     * at exception/UncaughtTest.bad(UncaughtTest.java:18)
     * at java/lang/RuntimeException.<init>(RuntimeException.java:62)
     * at java/lang/Exception.<init>(Exception.java:66)
     * at java/lang/Throwable.<init>(Throwable.java:265)
     * at java/lang/Throwable.fillInStackTrace(Throwable.java:783)
     * at java/lang/Throwable.fillInStackTrace(Native Method)
     */
    Frame *f = frame->prev->prev;
    num -= 2;

    for (Class *c = throwable->clazz; c != nullptr; c = c->super_class) {
        f = f->prev; // jump 执行异常类的构造函数的frame
        num--;
        if (equals(c->name, "java/lang/Throwable")) {
            break; // 可以了，遍历到 Throwable 就行了，因为现在在执行 Throwable 的 fillInStackTrace 方法。
        }
    }

    jarrRef backtrace = Allocator::object_array(num);
    auto trace = (Object **) backtrace->data;

    Class *c = load_boot_class("java/lang/StackTraceElement");
    for (int i = 0; f != nullptr; f = f->prev) {
        Object *o = Allocator::object(c);
        assert(i < num);
        trace[i++] = o;

        // public StackTraceElement(String declaringClass, String methodName, String fileName, int lineNumber)
        // may be should call <init>, but 直接赋值 is also ok. todo

        jstrRef file_name = f->method->clazz->source_file_name != nullptr
                        ? Allocator::string(f->method->clazz->source_file_name)
                        : nullptr;
        jstrRef class_name = Allocator::string(f->method->clazz->name);
        jstrRef method_name = Allocator::string(f->method->name);

        jint line_number = f->method->get_line_number(f->reader.pc - 1); // todo why 减1？ 减去opcode的长度

        o->set_field_value<jref>("fileName", "Ljava/lang/String;", file_name);
        o->set_field_value<jref>("declaringClass", "Ljava/lang/String;", class_name);
        o->set_field_value<jref>("methodName", "Ljava/lang/String;", method_name);
        o->set_field_value<jint>("lineNumber", line_number);

        // private transient Class<?> declaringClassObject;
        o->set_field_value<jref>("declaringClassObject", "Ljava/lang/Class;", c->java_mirror);
    }

    /*
     * Native code saves some indication of the stack backtrace in this slot.
     *
     * private transient Object backtrace;
     */
    throwable->set_field_value<jref>("backtrace", "Ljava/lang/Object;", backtrace);

    /*
     * The JVM code sets the depth of the backtrace for later retrieval
     * todo test-java on jdk15
     * private transient int depth;
     */
    throwable->set_field_value<jint>("depth", backtrace->arr_len);
}

/*
 * java.lang.StackTraceElement
 */

// Sets the given stack trace elements with the backtrace of the given Throwable.
JNIEXPORT void JNICALL
JVM_InitStackTraceElementArray(JNIEnv *env, jobjectArray _elements, jobject throwable) {
    TRACE("JVM_InitStackTraceElementArray(env=%p, elements=%p, throwable=%p)", env, _elements, throwable);
    auto elements = (jarrRef) _elements;
    jref x = (jref) throwable;

    jref backtrace = x->get_field_value<jref>("backtrace", "Ljava/lang/Object;");
    if (!backtrace->is_array_object()) {
        panic("error"); // todo
    }

    assert(elements->arr_len <= backtrace->arr_len);
    memcpy(elements->data, backtrace->data, elements->arr_len*sizeof(jref));
}

// Sets the given stack trace element with the given StackFrameInfo
JNIEXPORT void JNICALL
JVM_InitStackTraceElement(JNIEnv* env, jobject element, jobject stackFrameInfo) {
    TRACE("JVM_InitStackTraceElement(env=%p, elements=%p, throwable=%p)", env, element, stackFrameInfo);
    unimplemented
}

/*
 * java.lang.NullPointerException
 */

/**
 * Get an extended exception message. This returns a string describing
 * the location and cause of the exception. It returns null for
 * exceptions where this is not applicable.
 *
 * private native String getExtendedNPEMessage();
 */
JNIEXPORT jstring JNICALL
JVM_GetExtendedNPEMessage(JNIEnv *env, jthrowable throwable) {
    TRACE("JVM_GetExtendedNPEMessage(env=%p, throwable=%p)", env, throwable);
    unimplemented
}


// /*
//  * java.lang.Compiler
//  */
// JNIEXPORT void JNICALL
// JVM_InitializeCompiler (JNIEnv *env, jclass compCls)
// {
//     TRACE("JVM_InitializeCompiler(env=%p, compCls=%p)", env, compCls);
//     unimplemented
// }

// JNIEXPORT jboolean JNICALL
// JVM_IsSilentCompiler(JNIEnv *env, jclass compCls)
// {
//     TRACE("JVM_IsSilentCompiler(env=%p, compCls=%p)", env, compCls);
//     unimplemented
// }

// JNIEXPORT jboolean JNICALL
// JVM_CompileClass(JNIEnv *env, jclass compCls, jclass cls)
// {
//     TRACE("JVM_CompileClass(env=%p, compCls=%p, cls=%p)", env, compCls, cls);
//     unimplemented
// }

// JNIEXPORT jboolean JNICALL
// JVM_CompileClasses(JNIEnv *env, jclass cls, jstring jname)
// {
//     TRACE("JVM_CompileClasses(env=%p, cls=%p, jname=%p)", env, cls, jname);
//     unimplemented
// }

// JNIEXPORT jobject JNICALL
// JVM_CompilerCommand(JNIEnv *env, jclass compCls, jobject arg)
// {
//     TRACE("JVM_CompilerCommand(env=%p, compCls=%p, arg=%p)", env, compCls, arg);
//     unimplemented
// }

// JNIEXPORT void JNICALL
// JVM_EnableCompiler(JNIEnv *env, jclass compCls)
// {
//     TRACE("JVM_EnableCompiler(env=%p, compCls=%p)", env, compCls);
//     unimplemented
// }

// JNIEXPORT void JNICALL
// JVM_DisableCompiler(JNIEnv *env, jclass compCls)
// {
//     TRACE("JVM_DisableCompiler(env=%p, compCls=%p)", env, compCls);
//     unimplemented
// }

/*
 * java.lang.StackWalker
 */
enum {
    JVM_STACKWALK_CLASS_INFO_ONLY            = 0x2,
    JVM_STACKWALK_FILL_CLASS_REFS_ONLY       = 0x2,
    JVM_STACKWALK_GET_CALLER_CLASS           = 0x04,
    JVM_STACKWALK_SHOW_HIDDEN_FRAMES         = 0x20,
    JVM_STACKWALK_FILL_LIVE_STACK_FRAMES     = 0x100
};

JNIEXPORT void JNICALL
JVM_ExpandStackFrameInfo(JNIEnv *env, jobject obj) {
    TRACE("JVM_ExpandStackFrameInfo(env=%p)", env);
    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_CallStackWalk(JNIEnv *env, jobject stackStream, jint mode,
                  jint skip_frames, jobject contScope, jobject cont,
                  jint buffer_size, jint start_index, jobjectArray frames) {
    TRACE("JVM_CallStackWalk(env=%p)", env);
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_MoreStackWalk(JNIEnv *env, jobject stackStream, jint mode, jlong anchor,
                  jint last_batch_count, jint buffer_size, jint start_index,
                  jobjectArray frames) {
    TRACE("JVM_MoreStackWalk(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_SetStackWalkContinuation(JNIEnv *env, jobject stackStream,
                    jlong anchor, jobjectArray frames, jobject cont) {
    TRACE("JVM_SetStackWalkContinuation(env=%p)", env);
    unimplemented
}

/*
 * java.lang.Thread
 */

JNIEXPORT void JNICALL
JVM_StartThread(JNIEnv *env, jobject thread) {
    TRACE("JVM_StartThread(env=%p, thread=%p)", env, thread);
    java_lang_Thread::start((jref) thread);
}

JNIEXPORT void JNICALL
JVM_StopThread(JNIEnv *env, jobject thread, jobject exception) {
    TRACE("JVM_StopThread(env=%p, thread=%p, exception=%p)", env, thread, exception);

    // Thread *t = thread_from_tobj((jref) thread);

    unimplemented
}

/*
 * Tests if this thread is alive. A thread is alive if it has
 * been started and has not yet died.
 */
JNIEXPORT jboolean JNICALL
JVM_IsThreadAlive(JNIEnv *env, jobject thread) {
    TRACE("JVM_IsThreadAlive(env=%p, thread=%p)", env, thread);
    return java_lang_Thread::isAlive((jref) thread);
}

JNIEXPORT void JNICALL
JVM_SuspendThread(JNIEnv *env, jobject thread) {
    TRACE("JVM_SuspendThread(env=%p, thread=%p)", env, thread);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_ResumeThread(JNIEnv *env, jobject thread) {
    TRACE("JVM_ResumeThread(env=%p, thread=%p)", env, thread);
    unimplemented
}

/**
 * Changes the priority of this thread.
 * <p>
 * First the <code>checkAccess</code> method of this thread is called
 * with no arguments. This may result in throwing a
 * <code>SecurityException</code>.
 * <p>
 * Otherwise, the priority of this thread is set to the smaller of
 * the specified <code>newPriority</code> and the maximum permitted
 * priority of the thread's thread group.
 *
 * @param newPriority priority to set this thread to
 * @exception  IllegalArgumentException  If the priority is not in the
 *               range <code>MIN_PRIORITY</code> to
 *               <code>MAX_PRIORITY</code>.
 * @exception  SecurityException  if the current thread cannot modify
 *               this thread.
 * @see        #getPriority
 * @see        #checkAccess()
 * @see        #getThreadGroup()
 * @see        #MAX_PRIORITY
 * @see        #MIN_PRIORITY
 * @see        ThreadGroup#getMaxPriority()
 */
JNIEXPORT void JNICALL
JVM_SetThreadPriority(JNIEnv *env, jobject thread, jint prio) {
    TRACE("JVM_SetThreadPriority(env=%p, thread=%p, prio=%d)", env, thread, prio);
    // unimplemented
}

JNIEXPORT void JNICALL
JVM_Yield(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_Yield(env=%p, threadClass=%p)", env, threadClass);
    this_thread::yield();
}

JNIEXPORT void JNICALL
JVM_Sleep(JNIEnv *env, jclass threadClass, jlong millis) {
    TRACE("JVM_Sleep(env=%p, thread=%p, millis=%ld)", env, threadClass, millis);
    if (millis <= 0) {
        JNI_THROW_IllegalArgumentException(env, nullptr); // todo msg
        return;
    }
    if (millis == 0)
        return;

    // todo 是不是要释放锁？？？

    this_thread::sleep_for(chrono::microseconds(millis));

    // todo 怎么处理 InterruptedException
}

JNIEXPORT void JNICALL
JVM_SleepNanos(JNIEnv *env, jclass threadClass, jlong nanos) {
    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_CurrentCarrierThread(JNIEnv *env, jclass threadClass) {
    unimplemented
}

// Returns a reference to the currently executing thread Object.
JNIEXPORT jobject JNICALL
JVM_CurrentThread(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_CurrentThread(env=%p, threadClass=%p)", env, threadClass);
    Thread *t = get_current_thread();
    return (jobject) t->tobj;
}

JNIEXPORT void JNICALL
JVM_SetCurrentThread(JNIEnv *env, jobject thisThread, jobject theThread) {
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_CountStackFrames(JNIEnv *env, jobject thread) {
    TRACE("JVM_CountStackFrames(env=%p, thread=%p)", env, thread);
    return Thread::from((jref) thread)->count_stack_frames();
}

// inform VM of interrupt
JNIEXPORT void JNICALL
JVM_Interrupt(JNIEnv *env, jobject thread) {
    TRACE("JVM_Interrupt(env=%p, thread=%p)", env, thread);
    Thread *t = Thread::from((jref) thread);
    t->interrupted = true;
}

// /*
//  * Tests if some Thread has been interrupted.  The interrupted state
//  * is reset or not based on the value of ClearInterrupted that is passed.
//  */
// JNIEXPORT jboolean JNICALL
// JVM_IsInterrupted(JNIEnv *env, jobject thread, jboolean clearInterrupted)
// {
//     TRACE("JVM_IsInterrupted(env=%p, thread=%p, clearInterrupted=%d)", env, thread, clearInterrupted);
//     Thread *t = thread_from_tobj(thread);
//     jbool b = t->interrupted;
//     if (b && clearInterrupted) {
//         t->interrupted = jfalse;
//     }
//     return b;
// }

JNIEXPORT jboolean JNICALL
JVM_HoldsLock(JNIEnv *env, jclass threadClass, jobject obj) {
    TRACE("JVM_HoldsLock(env=%p, threadClass=%p, obj=%p)", env, threadClass, obj);
    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_GetStackTrace(JNIEnv *env, jobject thread) {
    TRACE("JVM_GetStackTrace(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_DumpAllStacks(JNIEnv *env, jclass dummy) {
    TRACE("JVM_DumpAllStacks(env=%p)", env);
    unimplemented
}

JNIEXPORT jobjectArray JNICALL
JVM_GetAllThreads(JNIEnv *env, jclass dummy) {
    TRACE("JVM_GetAllThreads(env=%p, dummy=%p)", env, dummy);
    jarrRef threads = Allocator::array("[Ljava/lang/Thread;", g_all_java_thread.size());

    int i = 0;
    for (Thread *t: g_all_java_thread) {
        threads->setRefElt(i, t->tobj);
        i++;
    }

    return (jobjectArray) threads;
}

JNIEXPORT void JNICALL
JVM_SetNativeThreadName(JNIEnv *env, jobject jthread, jstring name) {
    TRACE("JVM_SetNativeThreadName(env=%p, jthread=%p, name=%p)", env, jthread, name);
    unimplemented
}

/* getStackTrace() and getAllStackTraces() method */
JNIEXPORT jobjectArray JNICALL
JVM_DumpThreads(JNIEnv *env, jclass threadClass, jobjectArray _threads) {
    TRACE("JVM_DumpThreads(env=%p, threadClass=%p, threads=%p)", env, threadClass, _threads);
    jarrRef threads = (jarrRef) (_threads);
    assert(threads->is_array_object());

    size_t len = threads->arr_len;
    jarrRef result = Allocator::array("[[java/lang/StackTraceElement", len);

    for (size_t i = 0; i < len; i++) {
        jref tobj = threads->getElt<jref>(i);
        Thread *thread = Thread::from(tobj);
        jarrRef arr = thread->dump(-1);
        result->setRefElt(i, arr);
    }

    return (jobjectArray) result;
}

JNIEXPORT jobject JNICALL
JVM_ScopedValueCache(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_ScopedValueCache(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_SetScopedValueCache(JNIEnv *env, jclass threadClass, jobject theCache) {
    TRACE("JVM_SetScopedValueCache(env=%p)", env);
    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_FindScopedValueBindings(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_FindScopedValueBindings(env=%p)", env);
    unimplemented
}

JNIEXPORT jlong JNICALL
JVM_GetNextThreadIdOffset(JNIEnv *env, jclass threadClass) {
    TRACE("JVM_GetNextThreadIdOffset(env=%p)", env);
    unimplemented
}

/*
 * jdk.internal.vm.Continuation
 */
JNIEXPORT void JNICALL
JVM_RegisterContinuationMethods(JNIEnv *env, jclass cls) {
    TRACE("JVM_RegisterContinuationMethods(env=%p)", env);
    unimplemented
}

/*
 * java.lang.SecurityManager
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetClassContext(JNIEnv *env) {
    TRACE("JVM_GetClassContext(env=%p)", env);
    unimplemented
}

// JNIEXPORT jint JNICALL
// JVM_ClassDepth(JNIEnv *env, jstring name)
// {
//     TRACE("JVM_ClassDepth(env=%p, name=%p)", env, name);
//     unimplemented
// }

// JNIEXPORT jint JNICALL
// JVM_ClassLoaderDepth(JNIEnv *env)
// {
//     TRACE("JVM_ClassLoaderDepth(env=%p)", env);
//     unimplemented
// }

/*
 * java.lang.Package
 */
JNIEXPORT jstring JNICALL
JVM_GetSystemPackage(JNIEnv *env, jstring name) {
    TRACE("JVM_GetSystemPackage(env=%p, name=%p)", env, name);
    const char *utf8_name = java_lang_String::to_utf8((jstrRef)name);

    const char *pkg = get_boot_package(utf8_name);
    if (pkg == nullptr) {
        return nullptr;
    } else {
        return (jstring) Allocator::string(pkg);
    }
}

JNIEXPORT jobjectArray JNICALL
JVM_GetSystemPackages(JNIEnv *env) {
    TRACE("JVM_GetSystemPackages(env=%p)", env);

    utf8_set &packages = get_boot_packages();
    auto size = packages.size();

    auto ao = Allocator::string_array(size);
    auto p = (Object **) ao->data;
    for (auto pkg : packages) {
        *p++ = Allocator::string(pkg);
    }

    return (jobjectArray) ao;
}

/*
 * java.lang.ref.Reference
 */
// Atomically get and clear (set to null) the VM's pending-Reference list.
JNIEXPORT jobject JNICALL
JVM_GetAndClearReferencePendingList(JNIEnv *env) {
    TRACE("JVM_GetAndClearReferencePendingList(env=%p)", env);
    return (jobject) getAndClearReferencePendingList();
}

// Test whether the VM's pending-Reference list contains any entries.
JNIEXPORT jboolean JNICALL
JVM_HasReferencePendingList(JNIEnv *env) {
    TRACE("JVM_HasReferencePendingList(env=%p)", env);
    return hasReferencePendingList();
}

// Wait until the VM's pending-Reference list may be non-null.
JNIEXPORT void JNICALL
JVM_WaitForReferencePendingList(JNIEnv *env) {
    TRACE("JVM_WaitForReferencePendingList(env=%p)", env);
    waitForReferencePendingList();
}

// java.lang.ref.Reference
JNIEXPORT jboolean JNICALL
JVM_ReferenceRefersTo(JNIEnv *env, jobject ref, jobject o) {
    TRACE("JVM_ReferenceRefersTo(env=%p, ref=%p, o=%p)", env, ref, o);
    return referenceRefersTo((jref) ref, (jref) o);
}

JNIEXPORT void JNICALL
JVM_ReferenceClear(JNIEnv *env, jobject ref) {
    TRACE("JVM_ReferenceClear(env=%p, ref=%p)", env, ref);
    referenceClear((jref) ref);
}

/*
 * java.lang.ref.PhantomReference
 */
JNIEXPORT jboolean JNICALL
JVM_PhantomReferenceRefersTo(JNIEnv *env, jobject ref, jobject o) {
    TRACE("JVM_PhantomReferenceRefersTo(env=%p, ref=%p, o=%p)", env, ref, o);
    return phantomReferenceRefersTo((jref) ref, (jref) o);
}

/*
 * java.io.ObjectInputStream
 */
// JNIEXPORT jobject JNICALL
// JVM_AllocateNewObject(JNIEnv *env, jobject obj, jclass currClass, jclass initClass)
// {
//     TRACE("JVM_AllocateNewObject(env=%p, obj=%p, currClass=%p, initClass=%p)", env, obj, currClass, initClass);
//     unimplemented
// }

// JNIEXPORT jobject JNICALL
// JVM_AllocateNewArray(JNIEnv *env, jobject obj, jclass currClass, jint length)
// {
//     TRACE("JVM_AllocateNewArray(env=%p, obj=%p, currClass=%p, length=%d)", env, obj, currClass, length);
//     unimplemented
// }

JNIEXPORT jobject JNICALL
JVM_LatestUserDefinedLoader(JNIEnv *env) {
    TRACE("JVM_LatestUserDefinedLoader(env=%p)", env);
    unimplemented
}

// /*
//  * This function has been deprecated and should not be considered
//  * part of the specified JVM interface.
//  */
// JNIEXPORT jclass JNICALL
// JVM_LoadClass0(JNIEnv *env, jobject obj, jclass currClass, jstring currClassName)
// {
//     TRACE("JVM_LoadClass0(env=%p, obj=%p, currClass=%p, currClassName=%p)",
//                     env, obj, currClass, currClassName);
//     JVM_PANIC("This function has been deprecated "
//             "and should not be considered part of the specified JVM interface.");
// }

/*
 * java.lang.reflect.Array
 */
JNIEXPORT jint JNICALL
JVM_GetArrayLength(JNIEnv *env, jobject arr) {
    TRACE("JVM_GetArrayLength(env=%p, array=%p)", env, arr);
    jref array = (jref) arr;
    if (array == nullptr) {
        JNI_THROW_NPE(env, nullptr);
        return -1;
    }
    if (!array->is_array_object()) {
        JNI_THROW_IllegalArgumentException(env, "Argument is not an array");
        return -1;
    }
    return (*env)->GetArrayLength(env, arr);
    // return array->arr_len;
}

JNIEXPORT jobject JNICALL
JVM_GetArrayElement(JNIEnv *env, jobject arr, jint index) {
    TRACE("JVM_GetArrayElement(env=%p, arr=%p, index=%d)", env, arr, index);

    auto array = (jarrRef) arr;
    if (array == nullptr) {
        JNI_THROW_NPE(env, nullptr);
        return nullptr;
    }
    if (!array->is_array_object()) {
        JNI_THROW_IllegalArgumentException(env, "Argument is not an array");
        return nullptr;
    }

    if (index < 0 || index >= array->arr_len) {
        JNI_THROW_ArrayIndexOutOfBoundsException(env, nullptr); // todo msg
        return nullptr;
    }

    switch (array->clazz->name[1]) {
    case 'Z': // boolean[]
        return (jobject) bool_box(array->getElt<jbool>(index));
    case 'B': // byte[]
        return (jobject) byte_box(array->getElt<jbyte>(index));
    case 'C': // char[]
        return (jobject) char_box(array->getElt<jchar>(index));
    case 'S': // short[]
        return (jobject) short_box(array->getElt<jshort>(index));
    case 'I': // int[]
        return (jobject) int_box(array->getElt<jint>(index));
    case 'J': // long[]
        return (jobject) long_box(array->getElt<jlong>(index));
    case 'F': // float[]
        return (jobject) float_box(array->getElt<jfloat>(index));
    case 'D': // double[]
        return (jobject) double_box(array->getElt<jdouble>(index));
    default:  // reference array
        return (jobject) array->getElt<jref>(index);
    }
}

JNIEXPORT jvalue JNICALL
JVM_GetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jint wCode) {
    TRACE("JVM_GetPrimitiveArrayElement(env=%p, arr=%p, index=%d, wCode=%d)", env, arr, index, wCode);

    jvalue v{};
    auto array = (jarrRef) arr;
    if (array == nullptr) {
        JNI_THROW_NPE(env, nullptr);
        return v;
    }
    if (!array->is_array_object()) {
        JNI_THROW_IllegalArgumentException(env, "Argument is not an array");
        return v;
    }

    if (index < 0 || index >= array->arr_len) {
        JNI_THROW_NegativeArraySizeException(env, nullptr); // todo msg
        return v;
    }

    if (!array->is_type_array()) {
        JNI_THROW_IllegalArgumentException(env, nullptr); // todo msg
        return v;
    }

    switch (wCode) {
    case JVM_AT_BOOLEAN:
        v.z = array->getElt<jbool>(index);
        return v;
    case JVM_AT_CHAR:
        v.c = array->getElt<jchar>(index);
        return v;
    case JVM_AT_FLOAT:
        v.f = array->getElt<jfloat>(index);
        return v;
    case JVM_AT_DOUBLE:
        v.d = array->getElt<jdouble>(index);
        return v;
    case JVM_AT_BYTE:
        v.b = array->getElt<jbyte>(index);
        return v;
    case JVM_AT_SHORT:
        v.s = array->getElt<jshort>(index);
        return v;
    case JVM_AT_INT:
        v.i = array->getElt<jint>(index);
        return v;
    case JVM_AT_LONG:
        v.j = array->getElt<jlong>(index);
        return v;
    default:
        UNREACHABLE("%d", wCode); // todo
    }
}

JNIEXPORT void JNICALL
JVM_SetArrayElement(JNIEnv *env, jobject arr, jint index, jobject val) {
    TRACE("JVM_SetArrayElement(env=%p, arr=%p, index=%d, val=%p)", env, arr, index, val);
    auto array = (jarrRef) arr;
    jref value = (jref) val;

    if (array == nullptr) {
        JNI_THROW_NPE(env, nullptr);
        return;
    }
    if (!array->is_array_object()) {
        JNI_THROW_IllegalArgumentException(env, "Argument is not an array");
        return;
    }

    if (index < 0 || index >= array->arr_len) {
        JNI_THROW_NegativeArraySizeException(env, nullptr); // todo msg
        return;
    }

    if (value == nullptr) {
        if (array->is_type_array()) {
            // 基本类型的数组无法设空值
            JNI_THROW_IllegalArgumentException(env, nullptr); // todo msg
            return;
        }
        array->setRefElt(index, value);
        return;
    }

    switch (array->clazz->name[1]) {
    case 'Z': // boolean[]
        if (!equals(value->clazz->name, "java/lang/Boolean")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setBoolElt(index, get<jbool>(value->unbox()));
        }
        return;
    case 'B': // byte[]
        if (!equals(value->clazz->name, "java/lang/Byte")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setByteElt(index, get<jbyte>(value->unbox()));
        }
        return;
    case 'C': // char[]
        if (!equals(value->clazz->name, "java/lang/Character")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setCharElt(index, get<jchar>(value->unbox()));
        }
        return;
    case 'S': // short[]
        if (!equals(value->clazz->name, "java/lang/Short")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setShortElt(index, get<jshort>(value->unbox()));
        }
        return;
    case 'I': // int[]
        if (!equals(value->clazz->name, "java/lang/Integer")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setIntElt(index, get<jint>(value->unbox()));
        }
        return;
    case 'J': // long[]
        if (!equals(value->clazz->name, "java/lang/Long")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setLongElt(index, get<jlong>(value->unbox()));
        }
        return;
    case 'F': // float[]
        if (!equals(value->clazz->name, "java/lang/Float")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setFloatElt(index, get<jfloat>(value->unbox()));
        }
        return;
    case 'D': // double[]
        if (!equals(value->clazz->name, "java/lang/Double")) {
            JNI_THROW_IllegalArgumentException(env, "argument type mismatch");
            return;
        } else {
            array->setDoubleElt(index, get<jdouble>(value->unbox()));
        }
        return;
    default:  // reference array
        array->setRefElt(index, value);
        return;
    }
}

JNIEXPORT void JNICALL
JVM_SetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jvalue v, unsigned char vCode) {
    TRACE("JVM_SetPrimitiveArrayElement(env=%p, arr=%p, index=%d, v=%lld, vCode=%d)",
                                         env, arr, index, v.j, vCode);

    auto array = (jarrRef) arr;
    if (array == nullptr) {
        JNI_THROW_NPE(env, nullptr);
        return;
    }
    if (!array->is_array_object()) {
        JNI_THROW_IllegalArgumentException(env, "Argument is not an array");
        return;
    }

    if (index < 0 || index >= array->arr_len) {
        JNI_THROW_NegativeArraySizeException(env, nullptr); // todo msg
        return;
    }

    if (!array->is_type_array()) {
        JNI_THROW_IllegalArgumentException(env, nullptr); // todo msg
        return;
    }

    switch (vCode) {
    case JVM_AT_BOOLEAN:
        array->setBoolElt(index, v.z);
        return;
    case JVM_AT_CHAR:
        array->setCharElt(index, v.c);
        return;
    case JVM_AT_FLOAT:
        array->setFloatElt(index, v.f);
        return;
    case JVM_AT_DOUBLE:
        array->setDoubleElt(index, v.d);
        return;
    case JVM_AT_BYTE:
        array->setByteElt(index, v.b);
        return;
    case JVM_AT_SHORT:
        array->setShortElt(index, v.s);
        return;
    case JVM_AT_INT:
        array->setIntElt(index, v.i);
        return;
    case JVM_AT_LONG:
        array->setLongElt(index, v.j);
        return;
    default:
        UNREACHABLE("%d", vCode); // todo
    }
}

JNIEXPORT jobject JNICALL
JVM_NewArray(JNIEnv *env, jclass _eltClass, jint length) {
    TRACE("JVM_NewArray(env=%p, eltClass=%p, length=%d)", env, _eltClass, length);

    auto eltClass = (jclsRef) _eltClass;
    if (eltClass == nullptr) {
        JNI_THROW_NPE(env, "component type is null");
        return nullptr;
    }
    if (length < 0) {
        JNI_THROW_NegativeArraySizeException(env,  nullptr);  // todo msg
        return nullptr;
    }
    return (jobject) Allocator::array(eltClass->jvm_mirror->generate_array_class(), length);
}

JNIEXPORT jobject JNICALL
JVM_NewMultiArray(JNIEnv *env, jclass _eltClass, jintArray _dim) {
    TRACE("JVM_NewMultiArray(env=%p, eltClass=%p, length=%p)", env, _eltClass, _dim);

    if (_eltClass == nullptr) {
        JNI_THROW_NPE(env, "component type is null");
        return nullptr;
    }

    Class *c = JVM_MIRROR(_eltClass);
    auto dim = (jarrRef) _dim;

    jsize dim_count = dim->array_len();
    auto lens = new jint[dim_count];
    for (jsize i = 0; i < dim_count; i++) {
        lens[i] = dim->getElt<jint>(i);
        c = c->generate_array_class();
    }


    ArrayClass *ac = (ArrayClass *) c;
    jref a = Allocator::multi_array(ac, dim_count, lens);
    // jref a = ((ArrayClass *)c)->alloc_multi_array(dim_count, lens);
    delete[] lens;
    return (jobject) a;
}

/*
 * java.lang.Class and java.lang.ClassLoader
 */

/*
 * Returns the immediate caller class of the native method invoking
 * JVM_GetCallerClass.  The Method.invoke and other frames due to
 * reflection machinery are skipped.
 *
 * The caller is expected to be marked with
 * jdk.internal.reflect.CallerSensitive. The JVM will throw an
 * error if it is not marked properly.
 */
JNIEXPORT jclass JNICALL
JVM_GetCallerClass(JNIEnv *env) {
    TRACE("JVM_GetCallerClass(env=%p)", env);

    // top0, current frame is executing getCallerClass()
    // top1, who called getCallerClass, the one who wants to know his caller.
    // top2, the caller of top1, the result.
    auto frame = (Frame *) get_current_thread()->top_frame;
    Frame *top1 = frame->prev;
    assert(top1 != nullptr);

    Frame *top2 = top1->prev;
    assert(top2 != nullptr);

    jclsRef o = top2->method->clazz->java_mirror;
    return (jclass) o;
}


// /*
//  * Returns the class in which the code invoking the native method
//  * belongs.
//  *
//  * Note that in JDK 1.1, native methods did not create a frame.
//  * In 1.2, they do. Therefore native methods like Class.forName
//  * can no longer look at the current frame for the caller class.
//  */
// JNIEXPORT jclass JNICALL
// JVM_GetCallerClass(JNIEnv *env, int n)
// {
//     TRACE("JVM_GetCallerClass(env=%p, n=%d)", env, n);

//     // n < 0的情况
//     // top0, current frame is executing getCallerClass()
//     // top1, who called getCallerClass, the one who wants to know his caller.
//     // top2, the caller of top1, the result.
//     if (n < 0)
//         n = 2;

//     Frame *frame = (Frame *) getCurrentThread()->top_frame;
//     for (; n > 0 && frame != nullptr; n--) {
//         frame = frame->prev;
//     }

//     if (frame == nullptr) {
//         return nullptr;
//     }
//     return frame->method->clazz->java_mirror;
// }

/*
 * Find primitive classes
 * utf: class name
 */
JNIEXPORT jclass JNICALL
JVM_FindPrimitiveClass(JNIEnv *env, const char *utf) {
    TRACE("JVM_FindPrimitiveClass(env=%p, utf=%s)", env, utf);
    // 这里的 class name 是诸如 "int, float" 之类的 primitive type
    return (jclass) load_boot_class(utf)->java_mirror;
}

// /*
//  * Link the class
//  */
// JNIEXPORT void JNICALL
// JVM_ResolveClass(JNIEnv *env, jclass cls)
// {
//     TRACE("JVM_ResolveClass(env=%p, cls=%p)", env, cls);
//     unimplemented
// }

// /*
//  * Find a class from a given class loader. Throw ClassNotFoundException
//  * or NoClassDefFoundError depending on the value of the last argument.
//  */
// JNIEXPORT jclass JNICALL
// JVM_FindClassFromClassLoader(JNIEnv *env, const char *name, jboolean init,
//                              jobject loader, jboolean throwError)
// {
//     TRACE("JVM_FindClassFromClassLoader(env=%p, name=%s, init=%d, loader=%p, throwError=%d)",
//                                      env, name, init, loader, throwError);
//     unimplemented
// }

/*
 * Find a class from a boot class loader. Returns NULL if class not found.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromBootLoader(JNIEnv *env, const char *name) {
    TRACE("JVM_FindClassFromBootLoader(env=%p, name=%s)", env, name);
    if (name == nullptr)
        return nullptr;
    Class *c = load_boot_class(name);
    return c != nullptr ? (jclass) c->java_mirror : nullptr;
}

/*
 * Find a class from a given class loader.  Throws ClassNotFoundException.
 *  name:   name of class
 *  init:   whether initialization is done
 *  loader: class loader to look up the class. This may not be the same as the caller's
 *          class loader.
 *  caller: initiating class. The initiating class may be null when a security
 *          manager is not installed.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromCaller(JNIEnv *env, const char *name, jboolean init,
                        jobject loader, jclass caller) {
    TRACE("JVM_FindClassFromCaller(env=%p)", env); // todo

    // println("class name: %s", name);

    // todo
    Class *c = loadClass((jref) loader, name);
    if (c == nullptr) {
        // todo ClassNotFoundException
        unimplemented
        // (*env)->ThrowNew(env, )
    }

    return (jclass) c->java_mirror;

    // unimplemented
}

/*
 * Find a class from a given class.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromClass(JNIEnv *env, const char *name, jboolean init, jclass from) {
    TRACE("JVM_FindClassFromClass(env=%p, name=%s, init=%d, from=%p)",
                                     env, name, init, from);
    unimplemented
}

/* Find a loaded class cached by the VM */
JNIEXPORT jclass JNICALL
JVM_FindLoadedClass(JNIEnv *env, jobject loader, jstring name) {
    TRACE("JVM_FindLoadedClass(env=%p, loader=%p, name=%p)", env, loader, name);
    if (name == nullptr)
        return nullptr;
    utf8_t *slash_name = utf8::dot_2_slash_dup(java_lang_String::to_utf8((jstrRef)name));
    Class *c = find_loaded_class((jref) loader, slash_name);
    return c != nullptr ? (jclass) c->java_mirror : nullptr;
}

/* Define a class */
JNIEXPORT jclass JNICALL
JVM_DefineClass(JNIEnv *env, const char *name,
            jobject loader, const jbyte *buf, jsize len, jobject pd) {
    TRACE("JVM_DefineClass(env=%p, name=%s, loader=%p, buf=%p, len=%d, pd=%p)",
                    env, name, loader, buf, len, pd);

    Class *c = define_class((jref) loader, name, buf, len, (jref) pd, nullptr);
    return (jclass) c->java_mirror;
}

/* Define a class with a source (added in JDK1.5) */
JNIEXPORT jclass JNICALL
JVM_DefineClassWithSource(JNIEnv *env, const char *name, jobject loader,
                          const jbyte *buf, jsize len, jobject pd,
                          const char *source) {
    TRACE("JVM_DefineClassWithSource(env=%p, name=%s, loader=%p, buf=%p, len=%d, pd=%p, source=%s)",
                    env, name, loader, buf, len, pd, source);

    Class *c = define_class((jref) loader, name, buf, len, (jref) pd, source);
    return (jclass) c->java_mirror;
}

// /* Define a class with a source with conditional verification (added HSX 14)
//  * -Xverify:all will verify anyway, -Xverify:none will not verify,
//  * -Xverify:remote (default) will obey this conditional
//  * i.e. true = should_verify_class
//  */
// JNIEXPORT jclass JNICALL
// JVM_DefineClassWithSourceCond(JNIEnv *env, const char *name,
//                               jobject loader, const jbyte *buf,
//                               jsize len, jobject pd, const char *source,
//                               jboolean verify)
// {
//     TRACE("JVM_DefineClassWithSourceCond(env=%p, name=%s, loader=%p, buf=%p, "
//                                     "len=%d, pd=%p, source=%s, verify=%d)",
//                                      env, name, loader, buf, len, pd, source, verify);
//     unimplemented
// }

/*
 * Define a class with the specified lookup class.
 *  lookup:  Lookup class
 *  name:    the name of the class
 *  buf:     class bytes
 *  len:     length of class bytes
 *  pd:      protection domain
 *  init:    initialize the class
 *  flags:   properties of the class
 *  class_data: private static pre-initialized field; may be null
 */
JNIEXPORT jclass JNICALL
JVM_LookupDefineClass(JNIEnv *env, jclass _lookup, const char *name, const jbyte *buf,
                      jsize len, jobject pd, jboolean init, int flags, jobject class_data) {
    // From java/lang/invoke/MethodHandleNatives.java
    // Flags for Lookup.ClassOptions：
    static const int
        NESTMATE_CLASS        = 0x00000001,
        HIDDEN_CLASS          = 0x00000002,
        STRONG_LOADER_LINK    = 0x00000004,
        ACCESS_VM_ANNOTATIONS = 0x00000008;

    TRACE("JVM_LookupDefineClass(env=%p)", env); // todo

    Class *lookup = JVM_MIRROR(_lookup);
    Class *c = define_class(lookup, name, buf, len, (jref) pd, init, flags, (jref) class_data);

    if ((flags & NESTMATE_CLASS) != 0) {
        c->set_nest_host(lookup->get_nest_host());
    }
    if ((flags & HIDDEN_CLASS) != 0) {
        c->hidden = true;
    }
    if ((flags & STRONG_LOADER_LINK) != 0) {
        // unimplemented
    }
    if ((flags & ACCESS_VM_ANNOTATIONS) != 0) {
        // unimplemented
    }

    return (jclass) c->java_mirror;
}

/*
 * Module support funcions
 */

/*
 * Define a module with the specified packages and bind the module to the
 * given class loader.
 *  module:       module to define
 *  is_open:      specifies if module is open (currently ignored)
 *  version:      the module version
 *  location:     the module location
 *  packages:     array of packages in the module
 */
JNIEXPORT void JNICALL
JVM_DefineModule(JNIEnv *env, jobject _module, jboolean is_open, jstring _version,
                 jstring _location, jobjectArray _packages) {
    TRACE("JVM_DefineModule(env=%p)", env); // todo

    /* define module to VM */

    auto module = (jref) _module;
    auto version = (jstrRef) _version;
    auto location = (jstrRef) _location;
    auto packages = (jarrRef) _packages;

    // auto x = java_lang_String::toUtf8(version);
    // auto u = java_lang_String::toUtf8(location);


    // auto name = java_lang_Module::getName(module);
    // auto loader = java_lang_Module::getLoader(module);

    define_module_to_vm(module, is_open, version, location, packages);
}

/*
 * Set the boot loader's unnamed module.
 *  module: boot loader's unnamed module
 */
JNIEXPORT void JNICALL
JVM_SetBootLoaderUnnamedModule(JNIEnv *env, jobject module) {
    TRACE("JVM_SetBootLoaderUnnamedModule(env=%p, module=%p)", env, module);
    set_boot_loader_unnamed_module((jref) module);
}

/*
 * Do a qualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 *  to_module:   module to export the package to
 */
JNIEXPORT void JNICALL
JVM_AddModuleExports(JNIEnv *env, jobject from_module, jstring package, jobject to_module) {
    TRACE("JVM_AddModuleExports(env=%p, from_module=%p)", env, from_module); // todo
    return;
    unimplemented
}

/*
 * Do an export of a package to all unnamed modules.
 *  from_module: module containing the package to export
 *  package:     name of the package to export to all unnamed modules
 */
JNIEXPORT void JNICALL
JVM_AddModuleExportsToAllUnnamed(JNIEnv *env, jobject from_module, jstring package) {
    TRACE("JVM_AddModuleExportsToAllUnnamed(env=%p, from_module=%p, package=%p)", env, from_module, package);
    unimplemented
}

/*
 * Do an unqualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 */
JNIEXPORT void JNICALL
JVM_AddModuleExportsToAll(JNIEnv *env, jobject from_module, jstring package) {
    TRACE("JVM_AddModuleExportsToAll(env=%p, from_module=%p, package=%p)", env, from_module, package);
    return;
    unimplemented
}

/*
 * Add a module to the list of modules that a given module can read.
 *  from_module:   module requesting read access
 *  source_module: module that from_module wants to read
 */
JNIEXPORT void JNICALL
JVM_AddReadsModule(JNIEnv *env, jobject from_module, jobject source_module) {
    TRACE("JVM_AddReadsModule(env=%p, from_module=%p, source_module=%p)",
                            env, from_module, source_module);

    // jref from = (jref) from_module;
    // // source_module couble be null, 表示 ALL_UNNAMED_MODULE
    // jref source = (jref) source_module;

    // auto x = java_lang_Module::getName(from);
    // decltype(x) y = nullptr;
    // if (source != nullptr) {
    //     y = java_lang_Module::getName(source);
    // }

    return;
    unimplemented
}

/*
 * Ensure that the native representation of all archived java.lang.Module objects
 * are properly restored.
 *
 * Define all modules that have been stored in the CDS archived heap.
 *  platform_loader: the built-in platform class loader
 *  system_loader:   the built-in system class loader
 *
 * jdk\internal\misc\CDS.java
 */
JNIEXPORT void JNICALL
JVM_DefineArchivedModules(JNIEnv *env, jobject platform_loader, jobject system_loader) {
    TRACE("JVM_DefineArchivedModules(env=%p, platform_loader=%p, system_loader=%p)",
                                    env, platform_loader, system_loader);
    unimplemented
}


/*
 * Reflection support functions
 */

// /*
//  * Returns the name of the entity (class, interface, array class,
//  * primitive type, or void) represented by this Class object, as a String.
//  *
//  * Examples:
//  * String.class.getName()
//  *     returns "java.lang.String"
//  * byte.class.getName()
//  *     returns "byte"
//  * (new Object[3]).getClass().getName()
//  *     returns "[Ljava.lang.Object;"
//  * (new int[3][4][5][6][7][8][9]).getClass().getName()
//  *     returns "[[[[[[[I"
//  */
// JNIEXPORT jstring JNICALL
// JVM_GetClassName(JNIEnv *env, jclass cls)
// {
//     TRACE("JVM_GetClassName(env=%p, cls=%p)", env, cls);

//     jstring name = allocString(slash_to_dot_dup(JVM_MIRROR(cls)->name));
//     assert(g_string_class != nullptr);
//     return intern_string(name);
// }

/*
 * Cache the name to reduce the number of calls into the VM.
 * This field would be set by VM itself during initClassName call.
 *
 * private transient String name;
 * private native String initClassName();
 */
JNIEXPORT jstring JNICALL
JVM_InitClassName(JNIEnv *env, jclass cls) {
    TRACE("JVM_InitClassName(env=%p, cls=%p)", env, cls);

    jstrRef class_name = Allocator::string(slash_2_dot_dup(JVM_MIRROR(cls)->name));
    class_name = intern(class_name);
    ((jclsRef) cls)->set_field_value<jref>("name", "Ljava/lang/String;", class_name);
    return (jstring) class_name;
}

/**
 * Determines the itf_offsets implemented by the class or interface
 * represented by this object.
 *
 * <p> If this object represents a class, the return value is an array
 * containing objects representing all itf_offsets implemented by the
 * class. The order of the interface objects in the array corresponds to
 * the order of the interface names in the {@code implements} clause
 * of the declaration of the class represented by this object. For
 * example, given the declaration:
 * <blockquote>
 * {@code class Shimmer implements FloorWax, DessertTopping { ... }}
 * </blockquote>
 * suppose the value of {@code s} is an instance of
 * {@code Shimmer}; the value of the expression:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[0]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code FloorWax}; and the value of:
 * <blockquote>
 * {@code s.getClass().getInterfaces()[1]}
 * </blockquote>
 * is the {@code Class} object that represents interface
 * {@code DessertTopping}.
 *
 * <p> If this object represents an interface, the array contains objects
 * representing all itf_offsets extended by the interface. The order of the
 * interface objects in the array corresponds to the order of the interface
 * names in the {@code extends} clause of the declaration of the
 * interface represented by this object.
 *
 * <p> If this object represents a class or interface that implements no
 * itf_offsets, the method returns an array of length 0.
 *
 * <p> If this object represents a primitive type or void, the method
 * returns an array of length 0.
 *
 * <p> If this {@code Class} object represents an array type, the
 * itf_offsets {@code Cloneable} and {@code java.io.Serializable} are
 * returned in that order.
 *
 * @return an array of itf_offsets implemented by this class.
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetClassInterfaces(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassInterfaces(env=%p, cls=%p)", env, cls);

    Class *c = JVM_MIRROR(cls);
    auto size = (jint) c->interfaces.size();
    jarrRef interfaces = Allocator::class_array(size);
    for (int i = 0; i < size; i++) {
        assert(c->interfaces[i] != nullptr);
        interfaces->setRefElt(i, c->interfaces[i]->java_mirror);
    }

    return (jobjectArray) interfaces;
}

// JNIEXPORT jobject JNICALL
// JVM_GetClassLoader(JNIEnv *env, jclass cls)
// {
//     TRACE("JVM_GetClassLoader(env=%p, cls=%p)", env, cls);
//     return JVM_MIRROR(cls)->loader;
// }

JNIEXPORT jboolean JNICALL
JVM_IsInterface(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsInterface(env=%p, cls=%p)", env, cls);
    return JVM_MIRROR(cls)->is_interface() ? JNI_TRUE : JNI_FALSE;
}

/*
 * Gets the signers of this class.
 *
 * the signers of this class, or null if there are no signers.
 * In particular, this method returns null
 * if this object represents a primitive type or void.
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetClassSigners(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassSigners(env=%p, cls=%p)", env, cls);
    // Class *c = (Class *) cls;
    // return nullptr; // todo
    unimplemented
}

JNIEXPORT void JNICALL
JVM_SetClassSigners(JNIEnv *env, jclass cls, jobjectArray signers) {
    TRACE("JVM_SetClassSigners(env=%p, cls=%p, signers=%p)", env, cls, signers);
    unimplemented
}

/*
 * Returns the ProtectionDomain of this class.
 */
// private native java.security.ProtectionDomain getProtectionDomain0();
JNIEXPORT jobject JNICALL
JVM_GetProtectionDomain(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetProtectionDomain(env=%p, cls=%p)", env, cls);
    auto ref = (jclsRef) cls;
    return (jobject) ref->protection_domain;
    // unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsArrayClass(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsArrayClass(env=%p, cls=%p)", env, cls);
    return JVM_MIRROR(cls)->is_array_class() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
JVM_IsPrimitiveClass(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsPrimitiveClass(env=%p, cls=%p)", env, cls);
    return JVM_MIRROR(cls)->is_prim_class() ? JNI_TRUE : JNI_FALSE;
}

/**
 * Returns {@code true} if and only if the underlying class is a hidden class.
 *
 * @return {@code true} if and only if this class is a hidden class.
 *
 * @since 15
 * @see MethodHandles.Lookup#defineHiddenClass
 *
 * @HotSpotIntrinsicCandidate
 * public native boolean isHidden();
 *
 * http://openjdk.java.net/jeps/371
 */
JNIEXPORT jboolean JNICALL
JVM_IsHiddenClass(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsHiddenClass(env=%p, cls=%p)", env, cls);
    Class *c = JVM_MIRROR(cls);
    return c->hidden ? JNI_TRUE : JNI_FALSE;
}

// /*
//  * Returns the representing the component type of an array.
//  * If this class does not represent an array class this method returns null.
//  *
//  * like:
//  * [[I -> [I -> int -> null
//  */
// JNIEXPORT jclass JNICALL
// JVM_GetComponentType(JNIEnv *env, jclass cls)
// {
//     TRACE("JVM_GetComponentType(env=%p, cls=%p)", env, cls);
//     Class *c = ((jclsRef) cls)->jvm_mirror;
//     if (is_array_class(c)) {
//         return component_class(c)->java_mirror;
//     } else {
//         return nullptr;
//     }
// }

JNIEXPORT jint JNICALL
JVM_GetClassModifiers(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassModifiers(env=%p, cls=%p)", env, cls);
    return JVM_MIRROR(cls)->access_flags; // todo
}

/*
 * getClasses 和 getDeclaredClasses 的区别：
 * getClasses 得到该类及其父类所有的 public 的内部类。
 * getDeclaredClasses 得到该类所有的内部类，除去父类的。
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetDeclaredClasses(JNIEnv *env, jclass ofClass) {
    TRACE("JVM_GetDeclaredClasses(env=%p, ofClass=%p)", env, ofClass);
    Class *c = JVM_MIRROR(ofClass);
    return (jobjectArray) get_inner_classes_as_class_array(c, false);
}

/**
 * If the class or interface represented by this {@code Class} object
 * is a member of another class, returns the {@code Class} object
 * representing the class in which it was declared.  This method returns
 * null if this class or interface is not a member of any other class.  If
 * this {@code Class} object represents an array class, a primitive
 * type, or void,then this method returns null.
 *
 * 如果此类为内部类，返回其外部类
 *
 * @return the declaring class for this class
 * @throws SecurityException
 *         If a security manager, <i>s</i>, is present and the caller's
 *         class loader is not the same as or an ancestor of the class
 *         loader for the declaring class and invocation of {@link
 *         SecurityManager#checkPackageAccess s.checkPackageAccess()}
 *         denies access to the package of the declaring class
 * @since JDK1.1
 *
 * private native Class<?> getDeclaringClass0();
 */
JNIEXPORT jclass JNICALL
JVM_GetDeclaringClass(JNIEnv *env, jclass of_class) {
    TRACE("JVM_GetDeclaringClass(env=%p, of_class=%p)", env, of_class);
    Class *c = JVM_MIRROR(of_class);
    Class *d = get_declaring_class(c);
    return (jclass) (d != nullptr ? d->java_mirror : nullptr);
    // if (c->isArrayClass()) {
    //     return nullptr;
    // }

    // char buf[strlen(c->name) + 1];
    // strcpy(buf, c->name);
    // char *last_dollar = strrchr(buf, '$'); // 内部类标识：out_class_name$inner_class_name
    // if (last_dollar == nullptr) {
    //     return nullptr;
    // }

    // *last_dollar = 0;
    // c = loadClass(c->loader, buf);
    // assert(c != nullptr);
    // return (jclass) c->java_mirror;
}

/*
 * Returns the "simple binary name" of the underlying class, i.e.,
 * the binary name without the leading enclosing class name.
 * Returns null if the underlying class is a top level class.
 *
 * private native String getSimpleBinaryName0();
 */
JNIEXPORT jstring JNICALL
JVM_GetSimpleBinaryName(JNIEnv *env, jclass of_class) {
    TRACE("JVM_GetSimpleBinaryName(env=%p, of_class=%p)", env, of_class);
    // of_class 不会是 array，参见 function Class::getSimpleName0

    // examples:
    // int -> int
    // java.lang.String -> String
    // pkg.Bar$Foo -> Foo

    Class *c = JVM_MIRROR(of_class);
    const utf8_t *simple_binary_name = c->name;
    if (!c->is_prim_class()) {
        const char *p = strrchr(c->name, '$');
        if (p != nullptr) {
            simple_binary_name = ++p; // strip the leading enclosing class name
        } else if ((p = strrchr(c->name, '.')) != nullptr) {
            simple_binary_name = ++p; // strip the package name
        }
    }

    return (jstring) Allocator::string(simple_binary_name);
}

/* Generics support (JDK 1.5) */
JNIEXPORT jstring JNICALL
JVM_GetClassSignature(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassSignature(env=%p, cls=%p)", env, cls);
    Class *c = JVM_MIRROR(cls);
    if (c->signature != nullptr)
        return (jstring) Allocator::string(c->signature);
    return nullptr;
}

/* Annotations support (JDK 1.5) */
JNIEXPORT jbyteArray JNICALL
JVM_GetClassAnnotations(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassAnnotations(env=%p, cls=%p)", env, cls);
    Class *c = JVM_MIRROR(cls);
    return (jbyteArray) get_annotation_as_byte_array(c->rt_visi_annos);
}

// /* Annotations support (JDK 1.6) */

// // field is a handle to a java.lang.reflect.Field object
// JNIEXPORT jbyteArray JNICALL
// JVM_GetFieldAnnotations(JNIEnv *env, jobject field)
// {
//     TRACE("JVM_GetFieldAnnotations(env=%p, field=%p)", env, field);
//     unimplemented
// }

// // method is a handle to a java.lang.reflect.Method object
// JNIEXPORT jbyteArray JNICALL
// JVM_GetMethodAnnotations(JNIEnv *env, jobject method)
// {
//     TRACE("JVM_GetMethodAnnotations(env=%p, method=%p)", env, method);
//     unimplemented
// }

// // method is a handle to a java.lang.reflect.Method object
// JNIEXPORT jbyteArray JNICALL
// JVM_GetMethodDefaultAnnotationValue(JNIEnv *env, jobject method)
// {
//     TRACE("JVM_GetMethodDefaultAnnotationValue(env=%p, method=%p)", env, method);
//     unimplemented
// }

// // method is a handle to a java.lang.reflect.Method object
// JNIEXPORT jbyteArray JNICALL
// JVM_GetMethodParameterAnnotations(JNIEnv *env, jobject method)
// {
//     TRACE("JVM_GetMethodParameterAnnotations(env=%p, method=%p)", env, method);
//     unimplemented
// }

/* Type use annotations support (JDK 1.8) */

JNIEXPORT jbyteArray JNICALL
JVM_GetClassTypeAnnotations(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassTypeAnnotations(env=%p, cls=%p)", env, cls);
    Class *c = JVM_MIRROR(cls);
    return (jbyteArray) get_annotation_as_byte_array(c->rt_visi_type_annos);
}

// field is a handle to a java.lang.reflect.Field object
JNIEXPORT jbyteArray JNICALL
JVM_GetFieldTypeAnnotations(JNIEnv *env, jobject field) {
    TRACE("JVM_GetFieldTypeAnnotations(env=%p, field=%p)", env, field);

    jref fo = (jref) field;
    Field *f = get_field_from_reflect_object(fo);
    return (jbyteArray) get_annotation_as_byte_array(f->rt_visi_type_annos);
}

// method is a handle to a java.lang.reflect.Method object
JNIEXPORT jbyteArray JNICALL
JVM_GetMethodTypeAnnotations(JNIEnv *env, jobject method) {
    TRACE("JVM_GetMethodTypeAnnotations(env=%p, method=%p)", env, method);

    jref mo = (jref) method;
    Method *m = get_method_from_reflect_object(mo);
    return (jbyteArray) get_annotation_as_byte_array(m->rt_visi_type_annos);
}

// #undef GET_ANNOTATIONS_AS_BYTE_ARRAY

/*
 * New (JDK 1.4) reflection implementation
 */

/*
 * 注意 getDeclaredMethods 和 getMethods 方法的不同。
 * getDeclaredMethods(),该方法是获取本类中的所有方法，包括私有的(private、protected、默认以及public)的方法。
 * getMethods(),该方法是获取本类以及父类或者父接口中所有的公共方法(public修饰符修饰的)
 *
 * getDeclaredMethods 强调的是本类中定义的方法，不包括继承而来的。
 * 不包括 class initialization method(<clinit>)和构造函数(<init>)
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredMethods(JNIEnv *env, jclass of_class, jboolean public_only) {
    TRACE("JVM_GetClassDeclaredMethods(env=%p, of_class=%p, public_only=%d)", env, of_class, public_only);

    Class *c = ((jclsRef) of_class)->jvm_mirror;
    Class *method_reflect_class = load_boot_class("java/lang/reflect/Method");

    /*
     * Method(Class<?> declaringClass, String name, Class<?>[] parameterTypes, Class<?> returnType,
     *      Class<?>[] checkedExceptions, int modifiers, int slot, String signature,
     *      byte[] annotations, byte[] parameterAnnotations, byte[] annotationDefault)
     */
    Method *constructor = method_reflect_class->get_constructor(
                "(Ljava/lang/Class;" "Ljava/lang/String;" "[" "Ljava/lang/Class;"
                "Ljava/lang/Class;" "[" "Ljava/lang/Class;" "II" "Ljava/lang/String;" "[B[B[B)V");

    u2 count = 0;
    auto objects = new Object*[c->methods.size()];

    for (size_t i = 0; i < c->methods.size(); i++) {
        Method *m = c->methods[i];
        if (public_only && !m->isPublic())
            continue;
        if ((strcmp(m->name, "<clinit>") == 0) || (strcmp(m->name, "<init>") == 0))
            continue;

        Object *o = Allocator::object(method_reflect_class);
        objects[count++] = o;
        auto sig = m->signature != nullptr ? Allocator::string(m->signature) : nullptr;

        execJava(constructor, {
                rslot(o),                                               // this
                rslot(c->java_mirror),                                  // declaring class
                // name must be interned.
                // 参见 java/lang/reflect/Method 的说明
                rslot(intern(Allocator::string(m->name))),                    // name
                rslot(m->get_parameter_types()),                          // parameter types
                rslot(m->get_return_type()),                              // return type
                rslot(m->get_exception_types()),                          // checked exceptions
                islot(m->access_flags),                                 // modifiers todo
                islot(i),                                               // slot
                rslot(sig),                                             // signature
                rslot(get_annotation_as_byte_array(m->rt_visi_annos)),      // annotations
                rslot(get_annotation_as_byte_array(m->rt_visi_para_annos)), // parameter annotations
                rslot(get_annotation_as_byte_array(m->annotation_default)), // annotation default
        });
    }

    jarrRef method_array = Allocator::array(method_reflect_class->generate_array_class(), count);
    for (size_t i = 0; i < count; i++) {
        method_array->setRefElt(i, objects[i]);
    }

    delete[] objects;
    return (jobjectArray) method_array;
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredFields(JNIEnv *env, jclass ofClass, jboolean public_only) {
    TRACE("JVM_GetClassDeclaredFields(env=%p, ofClass=%p, public_only=%d)", env, ofClass, public_only);

    Class *c = ((jclsRef) ofClass)->jvm_mirror;
    Class *field_reflect_class = load_boot_class("java/lang/reflect/Field");

    Method *constructor;
    // Field(Class<?> declaringClass, String name, Class<?> type, int modifiers,
    //     boolean trustedFinal, int slot, String signature, byte[] annotations)
    constructor = field_reflect_class->get_constructor(
                        "(Ljava/lang/Class;" "Ljava/lang/String;" "Ljava/lang/Class;"
                        "IZI" "Ljava/lang/String;" "[B)V");

    u2 count = 0;
    auto objects = new Object*[c->fields.size()];

    // invoke constructor of class java/lang/reflect/Field
    for (size_t i = 0; i < c->fields.size(); i++) {
        auto f = c->fields[i];
        if (public_only && !f->isPublic())
            continue;

        Object *o = Allocator::object(field_reflect_class);
        objects[count++] = o;
        auto sig = f->signature != nullptr ? Allocator::string(f->signature) : nullptr;

        execJava(constructor, {
                rslot(o),                                          // this
                rslot(c->java_mirror),                             // declaring class
                // name must be interned.
                // 参见 java/lang/reflect/Field 的说明
                rslot(intern(Allocator::string(f->name))),               // name
                rslot(f->get_type()),                               // type
                islot(f->access_flags),                            // modifiers todo
                islot(f->isFinal() ? jtrue : jfalse),              // trusted Final todo
                islot(f->isStatic() ? i : f->id),                  // slot
                rslot(sig),                                        // signature
                rslot(get_annotation_as_byte_array(f->rt_visi_annos)), // annotations
        });
    }

    jarrRef field_array = Allocator::array(field_reflect_class->generate_array_class(), count);
    for (u2 i = 0; i < count; i++)
        field_array->setRefElt(i, objects[i]);

    delete[] objects;
    return (jobjectArray) field_array;
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredConstructors(JNIEnv *env, jclass ofClass, jboolean public_only) {
    TRACE("JVM_GetClassDeclaredConstructors(env=%p, ofClass=%p, public_only=%d)", env, ofClass, public_only);

    Class *c = ((jclsRef) ofClass)->jvm_mirror;
    Class *constructor_class = load_boot_class("java/lang/reflect/Constructor");

    vector<Method *> constructors = c->get_constructors(public_only);
    int count = constructors.size();

    jarrRef constructor_array = Allocator::array(constructor_class->generate_array_class(), count);

    /*
     * Constructor(Class<T> declaringClass, Class<?>[] parameterTypes,
     *      Class<?>[] checkedExceptions, int modifiers, int slot,
     *      String signature, byte[] annotations, byte[] parameterAnnotations)
     */
    Method *constructor = constructor_class->get_constructor(
        "(Ljava/lang/Class;" "[Ljava/lang/Class;"
        "[Ljava/lang/Class;" "IILjava/lang/String;" "[B[B)V");

    // invoke constructor of class java/lang/reflect/Constructor
    for (int i = 0; i < count; i++) {
        Method *cons = constructors[i];
        Object *o = Allocator::object(constructor_class);
        constructor_array->setRefElt(i, o);

        jstrRef sig = cons->signature != nullptr ? Allocator::string(cons->signature) : nullptr;

        execJava(constructor, {
                rslot(o),                                                  // this
                rslot(c->java_mirror),                                     // declaring class
                rslot(cons->get_parameter_types()),                          // parameter types
                rslot(cons->get_exception_types()),                          // checked exceptions
                islot(cons->access_flags),                                 // modifiers todo
                islot(i),                                                  // slot
                rslot(sig),                                                // signature
                rslot(get_annotation_as_byte_array(cons->rt_visi_annos)),      // annotations
                rslot(get_annotation_as_byte_array(cons->rt_visi_para_annos)), // parameter annotations
        });
    }

    return (jobjectArray) constructor_array;
}

/* Differs from JVM_GetClassModifiers in treatment of inner classes.
   This returns the access flags for the class as specified in the
   class file rather than searching the InnerClasses attribute (if
   present) to find the source-level access flags. Only the values of
   the low 13 bits (i.e., a mask of 0x1FFF) are guaranteed to be
   valid. */
JNIEXPORT jint JNICALL
JVM_GetClassAccessFlags(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassAccessFlags(env=%p, cls=%p)", env, cls);
    return JVM_MIRROR(cls)->access_flags; // todo
}

/* Nestmates - since JDK 11 */

JNIEXPORT jboolean JNICALL
JVM_AreNestMates(JNIEnv *env, jclass current, jclass member) {
    TRACE("JVM_AreNestMates(env=%p, current=%p, member=%p)", env, current, member);

    Class *c = JVM_MIRROR(current);
    Class *d = JVM_MIRROR(member);
    return c->test_nest_mate(d);
}

JNIEXPORT jclass JNICALL
JVM_GetNestHost(JNIEnv *env, jclass current) {
    TRACE("JVM_GetNestHost(env=%p, current=%p)", env, current);
    Class *c = JVM_MIRROR(current);
    return (jclass) c->get_nest_host()->java_mirror;
}

JNIEXPORT jobjectArray JNICALL
JVM_GetNestMembers(JNIEnv *env, jclass current) {
    TRACE("JVM_GetNestMembers(env=%p, current=%p)", env, current);

    Class *c = JVM_MIRROR(current);
    vector<void *> &members = c->get_nest_members();
    jarrRef o = Allocator::object_array(members.size());

    int i = 0;
    for (auto mem_cls: members) {
        assert(mem_cls != nullptr);
        o->setRefElt(i, ((Class *) mem_cls)->java_mirror);
        i++;
    }

    return (jobjectArray) o;
}

/* Records - since JDK 14 */

JNIEXPORT jboolean JNICALL
JVM_IsRecord(JNIEnv *env, jclass cls) {
    TRACE("JVM_IsRecord(env=%p, cls=%p)", env, cls);
    unimplemented
}

JNIEXPORT jobjectArray JNICALL
JVM_GetRecordComponents(JNIEnv *env, jclass ofClass) {
    TRACE("JVM_GetRecordComponents(env=%p, ofClass=%p)", env, ofClass);
    unimplemented
}

/* Sealed types - since JDK 17 */

JNIEXPORT jobjectArray JNICALL
JVM_GetPermittedSubclasses(JNIEnv *env, jclass current) {
    TRACE("JVM_GetPermittedSubclasses(env=%p, current=%p)", env, current);
    unimplemented
}

/* The following two reflection routines are still needed due to startup time issues */
/*
 * java.lang.reflect.Method
 */
JNIEXPORT jobject JNICALL
JVM_InvokeMethod(JNIEnv *env, jobject method, jobject obj, jobjectArray args0) {
    TRACE("JVM_InvokeMethod(env=%p, method=%p, obj=%p, args0=%p)", env, method, obj, args0);
    return (jobject) invoke_method((jref) method, (jref) obj, (jarrRef) args0);

    // auto method = (jref) _method;
    // auto o = (jref) obj; // If method is static, o is NULL.
    // auto os = (jarrRef) args0;

    // private Class<?>   clazz;
    // private String     name;
    // private Class<?>   returnType;
    // private Class<?>[] parameterTypes;
    // Class *c = method->getRefField("clazz", "Ljava/lang/Class;")->jvm_mirror;
    // jstrRef name = method->getRefField("name", "Ljava/lang/String;");
    // jref rtype = method->getRefField("returnType", "Ljava/lang/Class;");
    // jref ptypes = method->getRefField("parameterTypes", "[Ljava/lang/Class;");

    // auto _name = java_lang_String::toUtf8(name);
    // string desc = unparseMethodDescriptor(ptypes, rtype);
    // Method *m;
    // if (o != nullptr) // instance method
    //     m = o->clazz->lookupMethod(_name, desc.c_str());
    // else // static method
    //     m = c->lookupMethod(_name, desc.c_str());
    // assert(m != nullptr);

    // slot_t *result = execJava(m, o, os);
    // switch (m->ret_type) {
    // case Method::RET_VOID:
    //     return (jobject) voidBox();
    // case Method::RET_BYTE:
    //     return (jobject) byteBox(get<jbyte>(result));
    // case Method::RET_BOOL:
    //     return (jobject) boolBox(get<jbool>(result));
    // case Method::RET_CHAR:
    //     return (jobject) charBox(get<jchar>(result));
    // case Method::RET_SHORT:
    //     return (jobject) shortBox(get<jshort>(result));
    // case Method::RET_INT:
    //     return (jobject) intBox(get<jint>(result));
    // case Method::RET_FLOAT:
    //     return (jobject) floatBox(get<jfloat>(result));
    // case Method::RET_LONG:
    //     return (jobject) longBox(get<jlong>(result));
    // case Method::RET_DOUBLE:
    //     return (jobject) doubleBox(get<jdouble>(result));
    // case Method::RET_REFERENCE:
    //     return (jobject) getRef(result);
    // case Method::RET_INVALID:
    // default:
    //     ShouldNotReachHere("%d\n", m->ret_type);
    // }
}

/*
 * java.lang.reflect.Constructor
 */
JNIEXPORT jobject JNICALL
JVM_NewInstanceFromConstructor(JNIEnv *env, jobject c, jobjectArray args0) {
    TRACE("JVM_NewInstanceFromConstructor(env=%p, c=%p, args0=%p)", env, c, args0);
    return (jobject) new_instance_from_constructor((jref) c, (jarrRef) args0);

    // jref co = (jref) c; // Object of java.lang.reflect.Constructor
    // auto args = (jarrRef) args0; // could be null

    // // private Class<T> clazz;
    // Class *clazz = co->getRefField("clazz", "Ljava/lang/Class;")->jvm_mirror;
    // // private Class<?>[] parameterTypes;
    // jarrRef parameter_types = co->getRefField("parameterTypes", "[Ljava/lang/Class;");

    // Method *constructor = clazz->getConstructor(parameter_types);
    // jref new_instance = clazz->allocObject();
    // execJava(constructor, new_instance, args);
    // return (jobject) new_instance;
}

/*
 * Constant pool access; currently used to implement reflective access to annotations (JDK 1.5)
 */

JNIEXPORT jobject JNICALL
JVM_GetClassConstantPool(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetClassConstantPool(env=%p, cls=%p)", env, cls);
    Class *c = load_boot_class("jdk/internal/reflect/ConstantPool");
    jref cp = Allocator::object(c);
    cp->set_field_value<jref>("constantPoolOop", "Ljava/lang/Object;", (jref) &(JVM_MIRROR(cls)->cp));
    return (jobject) cp;
}

JNIEXPORT jint JNICALL
JVM_ConstantPoolGetSize(JNIEnv *env, jobject obj, jobject unused) {
    TRACE("JVM_ConstantPoolGetSize(env=%p, obj=%p)", env, obj);
    auto cp = (ConstantPool *) obj;
    return cp->size;
}

JNIEXPORT jclass JNICALL
JVM_ConstantPoolGetClassAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetClassAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return (jclass) (cp->resolve_class((u2) index)->java_mirror);
}

JNIEXPORT jclass JNICALL
JVM_ConstantPoolGetClassAtIfLoaded(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetClassAtIfLoaded(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    // cp->type[index]
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_ConstantPoolGetClassRefIndexAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetClassRefIndexAt(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_ConstantPoolGetMethodAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetMethodAt(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_ConstantPoolGetMethodAtIfLoaded(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetMethodAtIfLoaded(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_ConstantPoolGetFieldAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetFieldAt(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_ConstantPoolGetFieldAtIfLoaded(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetFieldAtIfLoaded(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jobjectArray JNICALL
JVM_ConstantPoolGetMemberRefInfoAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetMemberRefInfoAt(env=%p, obj=%p, index=%d)", env, obj, index);
    // ConstantPool *cp = (ConstantPool *) obj;

    unimplemented
}

JNIEXPORT jint JNICALL
JVM_ConstantPoolGetNameAndTypeRefIndexAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetNameAndTypeRefIndexAt(env=%p, obj=%p, index=%d)", env, obj, index);
    unimplemented
}

JNIEXPORT jobjectArray JNICALL
JVM_ConstantPoolGetNameAndTypeRefInfoAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetNameAndTypeRefInfoAt(env=%p, obj=%p, index=%d)", env, obj, index);
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_ConstantPoolGetIntAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetIntAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return cp->get_int((u2) index);
}

JNIEXPORT jlong JNICALL
JVM_ConstantPoolGetLongAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetLongAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return cp->get_long((u2) index);
}

JNIEXPORT jfloat JNICALL
JVM_ConstantPoolGetFloatAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetFloatAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return cp->get_float((u2) index);
}

JNIEXPORT jdouble JNICALL
JVM_ConstantPoolGetDoubleAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetDoubleAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return cp->get_double((u2) index);
}

JNIEXPORT jstring JNICALL
JVM_ConstantPoolGetStringAt(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetStringAt(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    return (jstring) cp->resolve_string(index);
}

JNIEXPORT jstring JNICALL
JVM_ConstantPoolGetUTF8At(JNIEnv *env, jobject obj, jobject unused, jint index) {
    TRACE("JVM_ConstantPoolGetUTF8At(env=%p, obj=%p, index=%d)", env, obj, index);
    auto cp = (ConstantPool *) obj;
    utf8_t *utf8 = cp->utf8(index);
    return (jstring) Allocator::string(utf8);
}

JNIEXPORT jbyte JNICALL
JVM_ConstantPoolGetTagAt(JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    TRACE("JVM_ConstantPoolGetTagAt()"); // todo
    unimplemented
}

/*
 * Parameter reflection
 */

JNIEXPORT jobjectArray JNICALL
JVM_GetMethodParameters(JNIEnv *env, jobject method) {
    TRACE("JVM_GetMethodParameters(env=%p, method=%p)", env, method);
    unimplemented
}

/*
 * java.security.*
 */

// JNIEXPORT jobject JNICALL
// JVM_DoPrivileged(JNIEnv *env, jclass cls, jobject action, jobject context, jboolean wrapException)
// {
//     TRACE("JVM_DoPrivileged(env=%p, cls=%p, action=%p, context=%p, wrapException=%d)",
//                             env, cls, action, context, wrapException);
//     unimplemented
// }

JNIEXPORT jobject JNICALL
JVM_GetInheritedAccessControlContext(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetInheritedAccessControlContext(env=%p, cls=%p)", env, cls);
    unimplemented
}

/*
 * Ensure that code doing a stackwalk and using javaVFrame::locals() to
 * get the value will see a materialized value and not a scalar-replaced
 * null value.
 */
#define JVM_EnsureMaterializedForStackWalk(env, value) \
    do {} while(0) // Nothing to do.  The fact that the value escaped
                   // through a native method is enough.
JNIEXPORT void JNICALL
JVM_EnsureMaterializedForStackWalk_func(JNIEnv* env, jobject vthread, jobject value) {
    TRACE("JVM_EnsureMaterializedForStackWalk_func(env=%p)", env);
    unimplemented
}

JNIEXPORT jobject JNICALL
JVM_GetStackAccessControlContext(JNIEnv *env, jclass cls) {
    TRACE("JVM_GetStackAccessControlContext(env=%p, cls=%p)", env, cls);
    // unimplemented
    return nullptr; // todo
}

/*
 * Signal support, used to implement the shutdown sequence.  Every VM must
 * support JVM_SIGINT and JVM_SIGTERM, raising the former for user interrupts
 * (^C) and the latter for external termination (kill, system shutdown, etc.).
 * Other platform-dependent signal values may also be supported.
 */

JNIEXPORT void * JNICALL
JVM_RegisterSignal(jint sig, void *handler) {
    TRACE("JVM_RegisterSignal(sig=%d, handler=%p)", sig, handler);

    // todo
    return handler;
}

JNIEXPORT jboolean JNICALL
JVM_RaiseSignal(jint sig) {
    TRACE("JVM_RaiseSignal(sig=%d)", sig);
    unimplemented
}

JNIEXPORT jint JNICALL
JVM_FindSignal(const char *name) {
    TRACE("JVM_FindSignal(name=%s)", name);
    // unimplemented
    return 0; // todo
}

/*
 * Retrieve the assertion directives for the specified class.
 */
JNIEXPORT jboolean JNICALL
JVM_DesiredAssertionStatus(JNIEnv *env, jclass unused, jclass cls) {
    TRACE("JVM_DesiredAssertionStatus(env=%p, cls=%p)", env, cls);
    // todo 本vm不讨论断言。desiredAssertionStatus0（）方法把false推入操作数栈顶
    return false;
}

/*
 * Retrieve the assertion directives from the VM.
 */
JNIEXPORT jobject JNICALL
JVM_AssertionStatusDirectives(JNIEnv *env, jclass unused) {
    TRACE("JVM_AssertionStatusDirectives(env=%p)", env);
    unimplemented
}

/*
 * java.lang.ref.Finalizer
 */
JNIEXPORT void JNICALL
JVM_ReportFinalizationComplete(JNIEnv *env, jobject finalize) {
    TRACE("JVM_ReportFinalizationComplete(env=%p)", env);
    unimplemented
}

JNIEXPORT jboolean JNICALL
JVM_IsFinalizationEnabled(JNIEnv *env) {
    TRACE("JVM_IsFinalizationEnabled(env=%p)", env);
    unimplemented
}


/*
 * java.util.concurrent.atomic.AtomicLong
 */
JNIEXPORT jboolean JNICALL
JVM_SupportsCX8(void) {
    // todo 这函数啥意思？？
    TRACE("JVM_SupportsCX8()");
    return JNI_FALSE;
}

// JNIEXPORT jboolean JNICALL
// JVM_CX8Field(JNIEnv *env, jobject obj, jfieldID fldID, jlong oldVal, jlong newVal)
// {
//     TRACE("JVM_CX8Field(env=%p, obj=%p, fldID=%p, oldVal=%lld, newVal=%lld)",
//                         env, obj, fldID, oldVal, newVal);
//     unimplemented
// }

/*
 * com.sun.dtrace.jsdt support
 */

#define JVM_TRACING_DTRACE_VERSION 1

/*
 * Structure to pass one probe description to JVM.
 *
 * The VM will overwrite the definition of the referenced method with
 * code that will fire the probe.
 */
struct JVM_DTraceProbe {
    jmethodID method;
    jstring   function;
    jstring   name;
    void*     reserved[4];     // for future use
};

/**
 * Encapsulates the stability ratings for a DTrace provider field
 */
struct JVM_DTraceInterfaceAttributes {
    jint nameStability;
    jint dataStability;
    jint dependencyClass;
};

/*
 * Structure to pass one provider description to JVM
 */
struct JVM_DTraceProvider {
    jstring                       name;
    JVM_DTraceProbe*              probes;
    jint                          probe_count;
    JVM_DTraceInterfaceAttributes providerAttributes;
    JVM_DTraceInterfaceAttributes moduleAttributes;
    JVM_DTraceInterfaceAttributes functionAttributes;
    JVM_DTraceInterfaceAttributes nameAttributes;
    JVM_DTraceInterfaceAttributes argsAttributes;
    void*                         reserved[4]; // for future use
};

/*
 * Get the version number the JVM was built with
 */
JNIEXPORT jint JNICALL
JVM_DTraceGetVersion(JNIEnv* env) {
    TRACE("JVM_DTraceGetVersion(env=%p", env);
    unimplemented
}

/*
 * Register new probe with given signature, return global handle
 *
 * The version passed in is the version that the library code was
 * built with.
 */
JNIEXPORT jlong JNICALL
JVM_DTraceActivate(JNIEnv* env, jint version, jstring moduleName,
                    jint providersCount, JVM_DTraceProvider* providers) {
    TRACE("JVM_DTraceActivate(env=%p, version=%d, moduleName=%p, providersCount=%d, providers=%p)",
                        env, version, moduleName, providersCount, providers);
    unimplemented
}

/*
 * Check JSDT probe
 */
JNIEXPORT jboolean JNICALL
JVM_DTraceIsProbeEnabled(JNIEnv* env, jmethodID method) {
    TRACE("JVM_DTraceIsProbeEnabled(env=%p, method=%p", env, method);
    unimplemented
}

/*
 * Destroy custom DOF
 */
JNIEXPORT void JNICALL
JVM_DTraceDispose(JNIEnv* env, jlong handle) {
    TRACE("JVM_DTraceDispose(env=%p, handle=%lld", env, handle);
    unimplemented
}

/*
 * Check to see if DTrace is supported by OS
 */
JNIEXPORT jboolean JNICALL
JVM_DTraceIsSupported(JNIEnv* env) {
    TRACE("JVM_DTraceIsSupported(env=%p", env);
    unimplemented
}

/*************************************************************************
 PART 2: Support for the Verifier and Class File Format Checker
 ************************************************************************/
/*
 * Return the class name in UTF format. The result is valid
 * until JVM_ReleaseUTf is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetClassNameUTF(JNIEnv *env, jclass cb) {
    TRACE("JVM_GetClassNameUTF(env=%p, cb=%p", env, cb);
    unimplemented
    // Class *c = JVM_MIRROR(cb);
    // return c->name;
}

// Returns the constant pool types in the buffer provided by "types."
JNIEXPORT void JNICALL
JVM_GetClassCPTypes(JNIEnv *env, jclass cb, unsigned char *types) {
    TRACE("JVM_GetClassCPTypes(env=%p, cb=%p, types=%p", env, cb, types);
    unimplemented
}

// Returns the number of Constant Pool entries.
JNIEXPORT jint JNICALL
JVM_GetClassCPEntriesCount(JNIEnv *env, jclass cb) {
    TRACE("JVM_GetClassCPEntriesCount(env=%p, cb=%p", env, cb);
    Class *c = JVM_MIRROR(cb);
    return c->cp->size;
}

// Returns the number of *declared* fields or methods.
JNIEXPORT jint JNICALL
JVM_GetClassFieldsCount(JNIEnv *env, jclass cb) {
    TRACE("JVM_GetClassFieldsCount(env=%p, cb=%p", env, cb);
    Class *c = JVM_MIRROR(cb);
    return c->fields.size();
}

JNIEXPORT jint JNICALL
JVM_GetClassMethodsCount(JNIEnv *env, jclass cb) {
    TRACE("JVM_GetClassMethodsCount(env=%p, cb=%p", env, cb);
    Class *c = JVM_MIRROR(cb);
    return c->methods.size();
}

/*
 * Returns the CP indexes of exceptions raised by a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxExceptionIndexes(JNIEnv *env, jclass cb, jint method_index, unsigned short *exceptions) {
    TRACE("JVM_GetMethodIxExceptionIndexes(env=%p, cb=%p, method_index=%d, exceptions=%p",
                                            env, cb, method_index, exceptions);
    unimplemented
}

/*
 * Returns the number of exceptions raised by a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxExceptionsCount(JNIEnv *env, jclass cb, jint method_index) {
    TRACE("JVM_GetMethodIxExceptionsCount(env=%p, cb=%p, method_index=%d)",
                                            env, cb, method_index);
    unimplemented
}

/*
 * Returns the byte code sequence of a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxByteCode(JNIEnv *env, jclass cb, jint method_index, unsigned char *code) {
    TRACE("JVM_GetMethodIxByteCode(env=%p, cb=%p, method_index=%d, code=%p",
                                            env, cb, method_index, code);
    unimplemented
}

/*
 * Returns the length of the byte code sequence of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxByteCodeLength(JNIEnv *env, jclass cb, jint method_index) {
    TRACE("JVM_GetMethodIxByteCodeLength(env=%p, cb=%p, method_index=%d)",
                                            env, cb, method_index);
    unimplemented
}

/*
 * A structure used to a capture exception table entry in a Java method.
 */
struct JVM_ExceptionTableEntryType {
    jint start_pc;
    jint end_pc;
    jint handler_pc;
    jint catchType;
};

/*
 * Returns the exception table entry at entry_index of a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxExceptionTableEntry(JNIEnv *env, jclass cb, jint method_index,
                                   jint entry_index, JVM_ExceptionTableEntryType *entry) {
    TRACE("JVM_GetMethodIxExceptionTableEntry(env=%p, cb=%p, method_index=%d, entry_index=%d, entry=%p)",
                                            env, cb, method_index, entry_index, entry);
    unimplemented
}

/*
 * Returns the length of the exception table of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxExceptionTableLength(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetMethodIxExceptionTableLength(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the modifiers of a given field.
 * The field is identified by field_index.
 */
JNIEXPORT jint JNICALL
JVM_GetFieldIxModifiers(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetFieldIxModifiers(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the modifiers of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxModifiers(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetMethodIxModifiers(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the number of local variables of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxLocalsCount(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetMethodIxLocalsCount(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the number of arguments (including this pointer) of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxArgsSize(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetMethodIxArgsSize(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the maximum amount of stack (in words) used by a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxMaxStack(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_GetMethodIxMaxStack(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Is a given method a constructor.
 * The method is identified by method_index.
 */
JNIEXPORT jboolean JNICALL
JVM_IsConstructorIx(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_IsConstructorIx(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Is the given method generated by the VM.
 * The method is identified by method_index.
 */
JNIEXPORT jboolean JNICALL
JVM_IsVMGeneratedMethodIx(JNIEnv *env, jclass cb, int index) {
    TRACE("JVM_IsVMGeneratedMethodIx(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the name of a given method in UTF format.
 * The result remains valid until JVM_ReleaseUTF is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetMethodIxNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetMethodIxNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the signature of a given method in UTF format.
 * The result remains valid until JVM_ReleaseUTF is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetMethodIxSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetMethodIxSignatureUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the name of the field refered to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPFieldNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPFieldNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the name of the method refered to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPMethodNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPMethodNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the signature of the method refered to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPMethodSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPMethodSignatureUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the signature of the field refered to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPFieldSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPFieldSignatureUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the class name refered to at a given constant pool index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPClassNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the class name refered to at a given constant pool index.
 *
 * The constant pool entry must refer to a CONSTANT_Fieldref.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPFieldClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPFieldClassNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the class name refered to at a given constant pool index.
 *
 * The constant pool entry must refer to CONSTANT_Methodref or
 * CONSTANT_InterfaceMethodref.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char * JNICALL
JVM_GetCPMethodClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    TRACE("JVM_GetCPMethodClassNameUTF(env=%p, cb=%p, index=%d)", env, cb, index);
    unimplemented
}

/*
 * Returns the modifiers of a field in calledClass. The field is
 * referred to in class cb at constant pool entry index.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 *
 * Returns -1 if the field does not exist in calledClass.
 */
JNIEXPORT jint JNICALL
JVM_GetCPFieldModifiers(JNIEnv *env, jclass cb, int index, jclass calledClass) {
    TRACE("JVM_GetCPFieldModifiers(env=%p, cb=%p, index=%d, calledClass=%p)",
                                    env, cb, index, calledClass);
    unimplemented
}

/*
 * Returns the modifiers of a method in calledClass. The method is
 * referred to in class cb at constant pool entry index.
 *
 * Returns -1 if the method does not exist in calledClass.
 */
JNIEXPORT jint JNICALL
JVM_GetCPMethodModifiers(JNIEnv *env, jclass cb, int index, jclass calledClass) {
    TRACE("JVM_GetCPMethodModifiers(env=%p, cb=%p, index=%d, calledClass=%p)",
                                    env, cb, index, calledClass);
    unimplemented
}

// Releases the UTF string obtained from the VM.
JNIEXPORT void JNICALL
JVM_ReleaseUTF(const char *utf) {
    TRACE("JVM_ReleaseUTF(utf=%s)", utf);
    unimplemented
}

// Compare if two classes are in the same package.
JNIEXPORT jboolean JNICALL
JVM_IsSameClassPackage(JNIEnv *env, jclass class1, jclass class2) {
    TRACE("JVM_IsSameClassPackage(env=%p, class1=%p, class2=%p)", env, class1, class2);
    return equals(JVM_MIRROR(class1)->pkg_name, JVM_MIRROR(class2)->pkg_name);
}

// /*
//  * A function defined by the byte-code verifier and called by the VM.
//  * This is not a function implemented in the VM.
//  *
//  * Returns JNI_FALSE if verification fails. A detailed error message
//  * will be places in msg_buf, whose length is specified by buf_len.
//  */
// typedef jboolean (*verifier_fn_t)(JNIEnv *env,
//                                   jclass cb,
//                                   char * msg_buf,
//                                   jint buf_len);


/*
 * Support for a VM-independent class format checker.
 */
typedef struct {
    unsigned long code;    /* byte code */
    unsigned long excs;    /* exceptions */
    unsigned long etab;    /* catch table */
    unsigned long lnum;    /* line number */
    unsigned long lvar;    /* local vars */
} method_size_info;

typedef struct {
    unsigned int constants;    /* constant pool */
    unsigned int fields;
    unsigned int methods;
    unsigned int interfaces;
    unsigned int fields2;      /* number of static 2-word fields */
    unsigned int innerclasses; /* # of records in InnerClasses attr */

    method_size_info clinit;   /* memory used in clinit */
    method_size_info main;     /* used everywhere else */
} class_size_info;

// /*
//  * Functions defined in libjava.so to perform string conversions.
//  *
//  */

// typedef jstring (*to_java_string_fn_t)(JNIEnv *env, char *str);

// typedef char *(*to_c_string_fn_t)(JNIEnv *env, jstring s, jboolean *b);

// /* This is the function defined in libjava.so that performs class
//  * format checks. This functions fills in size information about
//  * the class file and returns:
//  *
//  *   0: good
//  *  -1: out of memory
//  *  -2: bad format
//  *  -3: unsupported version
//  *  -4: bad class name
//  */

// typedef jint (*check_format_fn_t)(char *class_name,
//                                   unsigned char *data,
//                                   unsigned int data_size,
//                                   class_size_info *class_size,
//                                   char *message_buffer,
//                                   jint buffer_length,
//                                   jboolean measure_only,
//                                   jboolean check_relaxed);

#define JVM_RECOGNIZED_CLASS_MODIFIERS (JVM_ACC_PUBLIC | \
                                        JVM_ACC_FINAL | \
                                        JVM_ACC_SUPER | \
                                        JVM_ACC_INTERFACE | \
                                        JVM_ACC_ABSTRACT | \
                                        JVM_ACC_ANNOTATION | \
                                        JVM_ACC_ENUM | \
                                        JVM_ACC_SYNTHETIC)

#define JVM_RECOGNIZED_FIELD_MODIFIERS (JVM_ACC_PUBLIC | \
                                        JVM_ACC_PRIVATE | \
                                        JVM_ACC_PROTECTED | \
                                        JVM_ACC_STATIC | \
                                        JVM_ACC_FINAL | \
                                        JVM_ACC_VOLATILE | \
                                        JVM_ACC_TRANSIENT | \
                                        JVM_ACC_ENUM | \
                                        JVM_ACC_SYNTHETIC)

#define JVM_RECOGNIZED_METHOD_MODIFIERS (JVM_ACC_PUBLIC | \
                                         JVM_ACC_PRIVATE | \
                                         JVM_ACC_PROTECTED | \
                                         JVM_ACC_STATIC | \
                                         JVM_ACC_FINAL | \
                                         JVM_ACC_SYNCHRONIZED | \
                                         JVM_ACC_BRIDGE | \
                                         JVM_ACC_VARARGS | \
                                         JVM_ACC_NATIVE | \
                                         JVM_ACC_ABSTRACT | \
                                         JVM_ACC_STRICT | \
                                         JVM_ACC_SYNTHETIC)


/*************************************************************************
 PART 3: I/O and Network Support
 ************************************************************************/

/*
 * Convert a pathname into native format.  This function does syntactic
 * cleanup, such as removing redundant separator characters.  It modifies
 * the given pathname string in place.
 */
JNIEXPORT char * JNICALL
JVM_NativePath(char *path) {
    TRACE("JVM_NativePath(path=%p)", path);
    return path;
}

/*
 * The standard printing functions supported by the Java VM. (Should they
 * be renamed to JVM_* in the future?
 */

/* jio_snprintf() and jio_vsnprintf() behave like snprintf(3) and vsnprintf(3),
 *  respectively, with the following differences:
 * - The string written to str is always zero-terminated, also in case of
 *   truncation (count is too small to hold the result string), unless count
 *   is 0. In case of truncation count-1 characters are written and '\0'
 *   appendend.
 * - If count is too small to hold the whole string, -1 is returned across
 *   all platforms. */

JNIEXPORT int
jio_vsnprintf(char *str, size_t count, const char *fmt, va_list args) {
    TRACE("jio_vsnprintf()"); // todo

    if ((intptr_t) count <= 0)
        return -1;

    int len = vsnprintf(str, count, fmt, args);
    return len;
}

JNIEXPORT int
jio_snprintf(char *str, size_t count, const char *fmt, ...) {
    TRACE("jio_snprintf()"); // todo

    va_list ap;
    int len;

    va_start(ap, fmt);
    len = jio_vsnprintf(str, count, fmt, ap);
    va_end(ap);

    return len;
}

JNIEXPORT int
jio_vfprintf(FILE *f, const char *fmt, va_list args) {
    TRACE("jio_vfprintf()"); // todo
    unimplemented
}

JNIEXPORT int
jio_fprintf(FILE *f, const char *fmt, ...) {
    TRACE("jio_fprintf()"); // todo

    va_list ap;
    int len;

    va_start(ap, fmt);
    len = jio_vfprintf(f, fmt, ap);
    va_end(ap);

    return len;
}

JNIEXPORT void * JNICALL
JVM_RawMonitorCreate(void) {
    TRACE("JVM_RawMonitorCreate()");
    return new mutex;
}

JNIEXPORT void JNICALL
JVM_RawMonitorDestroy(void *mon) {
    TRACE("JVM_RawMonitorDestroy(mon=%p)", mon);
    delete ((mutex *) mon);
}

JNIEXPORT jint JNICALL
JVM_RawMonitorEnter(void *mon) {
    TRACE("JVM_RawMonitorEnter(mon=%p)", mon);
    ((mutex *) mon)->lock();
    return JNI_OK;
}

JNIEXPORT void JNICALL
JVM_RawMonitorExit(void *mon) {
    TRACE("JVM_RawMonitorExit(mon=%p)", mon);
    ((mutex *) mon)->unlock();
}

/*
 * jdk.management support
 */
JNIEXPORT void* JNICALL
JVM_GetManagement(jint version) {
    TRACE("JVM_GetManagement(version=%d)", version);
    return getJmmInterface(version);
}

/*
 * com.sun.tools.attach.VirtualMachine support
 *
 * Initialize the agent properties with the properties maintained in the VM.
 */
JNIEXPORT jobject JNICALL
JVM_InitAgentProperties(JNIEnv *env, jobject agent_props) {
    TRACE("JVM_InitAgentProperties(env=%p, agent_props=%p)", env, agent_props);
    unimplemented
}

JNIEXPORT jstring JNICALL
JVM_GetTemporaryDirectory(JNIEnv *env) {
    TRACE("JVM_GetTemporaryDirectory(env=%p)", env);
    unimplemented
}

JNIEXPORT jstring JNICALL
JVM_GetThreadInterruptEvent(JNIEnv *env) {
    // ------- todo 参数和返回值都不对 ------------------
    TRACE("JVM_GetThreadInterruptEvent(env=%p)", env);
    unimplemented
}

/* 
 * Generics reflection support.
 *
 * Returns information about the given class's EnclosingMethod
 * attribute, if present, or null if the class had no enclosing
 * method.
 *
 * If non-null, the returned array contains three elements. Element 0
 * is the java.lang.Class of which the enclosing method is a member,
 * and elements 1 and 2 are the java.lang.Strings for the enclosing
 * method's name and descriptor, respectively.
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetEnclosingMethodInfo(JNIEnv* env, jclass ofClass) {
    TRACE("JVM_GetEnclosingMethodInfo(env=%p, ofClass=%p)", env, ofClass);

    Class *c = JVM_MIRROR(ofClass);
    if (c->enclosing.clazz == nullptr)
        return nullptr;

    jarrRef a = Allocator::object_array({
        c->enclosing.clazz->java_mirror, c->enclosing.name, c->enclosing.descriptor });
    return (jobjectArray) a;
}

/*
 * Virtual thread support.
 */
JNIEXPORT void JNICALL
JVM_VirtualThreadStart(JNIEnv* env, jobject vthread) {
    TRACE("JVM_VirtualThreadStart(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_VirtualThreadEnd(JNIEnv* env, jobject vthread) {
    TRACE("JVM_VirtualThreadEnd(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_VirtualThreadMount(JNIEnv* env, jobject vthread, jboolean hide) {
    TRACE("JVM_VirtualThreadMount(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_VirtualThreadUnmount(JNIEnv* env, jobject vthread, jboolean hide) {
    TRACE("JVM_VirtualThreadUnmount(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_VirtualThreadHideFrames(JNIEnv* env, jclass clazz, jboolean hide) {
    TRACE("JVM_VirtualThreadHideFrames(env=%p)", env);
    unimplemented
}

JNIEXPORT void JNICALL
JVM_VirtualThreadDisableSuspend(JNIEnv* env, jclass clazz, jboolean enter) {
    TRACE("JVM_VirtualThreadDisableSuspend(env=%p)", env);
    unimplemented
}

/*
 * Core reflection support.
 */
JNIEXPORT jint JNICALL
JVM_GetClassFileVersion(JNIEnv *env, jclass current) {
    TRACE("JVM_GetClassFileVersion(env=%p)", env);
    unimplemented
}

/*
 * Return JNI_TRUE if warnings are printed when agents are dynamically loaded.
 */
JNIEXPORT jboolean JNICALL
JVM_PrintWarningAtDynamicAgentLoad(void) {
    TRACE("JVM_PrintWarningAtDynamicAgentLoad()");
    unimplemented
}

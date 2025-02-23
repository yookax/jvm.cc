#ifndef CABIN_THREAD_H
#define CABIN_THREAD_H

#include <vector>
#include <string>
#include <thread>
#include "../cabin.h"
#include "../slot.h"
#include "frame.h"

/*
 * jvm中所定义的线程
 *
 * 如果Java虚拟机栈有大小限制，且执行线程所需的栈空间超出了这个限制，
 * 会导致StackOverflowError异常抛出。如果Java虚拟机栈可以动态扩展，
 * 但是内存已经耗尽，会导致OutOfMemoryError异常抛出。
 */


/* Thread states */

#define JVMTI_THREAD_STATE_ALIVE                     0x001
#define JVMTI_THREAD_STATE_TERMINATED                0x002
#define JVMTI_THREAD_STATE_RUNNABLE                  0x004
#define JVMTI_THREAD_STATE_WAITING_INDEFINITELY      0x010
#define JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT      0x020
#define JVMTI_THREAD_STATE_SLEEPING                  0x040
#define JVMTI_THREAD_STATE_WAITING                   0x080
#define JVMTI_THREAD_STATE_IN_OBJECT_WAIT            0x100
#define JVMTI_THREAD_STATE_PARKED                    0x200
#define JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER  0x400

#define CREATING           0x0
#define RUNNING            (JVMTI_THREAD_STATE_ALIVE \
                           |JVMTI_THREAD_STATE_RUNNABLE)
#define WAITING            (JVMTI_THREAD_STATE_ALIVE \
                           |JVMTI_THREAD_STATE_WAITING \
                           |JVMTI_THREAD_STATE_WAITING_INDEFINITELY)
#define TIMED_WAITING      (JVMTI_THREAD_STATE_ALIVE \
                           |JVMTI_THREAD_STATE_WAITING \
                           |JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT)
#define OBJECT_WAIT        (JVMTI_THREAD_STATE_IN_OBJECT_WAIT|WAITING)
#define OBJECT_TIMED_WAIT  (JVMTI_THREAD_STATE_IN_OBJECT_WAIT|TIMED_WAITING)
#define SLEEPING           (JVMTI_THREAD_STATE_SLEEPING|TIMED_WAITING)
#define PARKED             (JVMTI_THREAD_STATE_PARKED|WAITING)
#define TIMED_PARKED       (JVMTI_THREAD_STATE_PARKED|TIMED_WAITING)
#define BLOCKED            JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER
#define TERMINATED         JVMTI_THREAD_STATE_TERMINATED

/* thread priorities */

#define THREAD_MIN_PRIORITY   1
#define THREAD_NORM_PRIORITY  5
#define THREAD_MAX_PRIORITY   10

///* Suspend states */
//
//#define SUSP_NONE      0
//#define SUSP_BLOCKING  1
//#define SUSP_CRITICAL  2
//#define SUSP_SUSPENDED 3
//
///* Park states */
//
//#define PARK_BLOCKED   0
//#define PARK_RUNNING   1
//#define PARK_PERMIT    2

class Thread {
    /*
     * VM stack 中的 Frame 布局：
     * ------------------------------------------------------------------
     * |lvars|Frame|ostack|, |lvars|Frame|ostack|, |lvars|Frame|ostack| ...
     * ------------------------------------------------------------------
     */
    u1 vm_stack[VM_STACK_SIZE]; // 虚拟机栈，一个线程只有一个虚拟机栈
public:   
    Frame *top_frame = nullptr;

    Object *tobj = nullptr;  // 所关联的 Object of java.lang.Thread
    std::thread::id tid;     // 所关联的 local thread 对应的id

    jbool interrupted = false;
    
private:
    jref jni_excep = nullptr;    
public:
    static void jniThrow(jref excep);
    static void jniThrow(Class *excep_class, const char *msg);
    static jref jniExceptionOccurred();
    static void jniExceptionClear();

    // Thread *next = nullptr;

    Thread();
    
    void bind(Object *tobj = nullptr);

    static Thread *from(Object *tobj);
    static Thread *from(jlong tid);

    Frame *alloc_frame(Method *, bool vm_invoke);
    void pop_frame() { top_frame = top_frame->prev; }

    int count_stack_frames() const;

   [[noreturn]] void terminate(ExitCode code);

    /*
     * return [Ljava/lang/StackTraceElement;
     * where @max_depth < 0 to request entire stack dump
     */
    jarrRef dump(int max_depth) const;
};

extern std::vector<Thread *> g_all_java_thread;
#define ALL_JAVA_THREADS(x) for (Thread *x: g_all_java_thread)
#define ALL_JAVA_THREADS_COUNT g_all_java_thread.size()

namespace java_lang_Thread {
    void start(jref tobj);
    bool isAlive(jref tobj);

    void setStatus(jref tobj, jint status);
    jint getStatus(jref tobj);

    const char *getNameUtf8(jref tobj);
    const jstrRef getName(jref tobj);
    const jlong getTid(jref tobj);
};

extern Thread *g_main_thread;

void init_thread();

Thread *get_current_thread();

/*
 * return a reference of java/lang/management/ThreadInfo(in java.management module)
 * where maxDepth < 0 to request entire stack dump
 */
// jref to_java_lang_management_ThreadInfo(const Thread *, jbool locked_monitors, jbool locked_synchronizers, jint max_depth);

#endif // CABIN_THREAD_H
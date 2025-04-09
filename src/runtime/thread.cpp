module;
#include <cassert>
#include "../vmdef.h"

module runtime;

import std.core;
import std.threading;
import object;
import class_loader;
import interpreter;

using namespace std;
using namespace slot;
using namespace java_lang_Thread;


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
#define RUNNING            (JVMTI_THREAD_STATE_ALIVE |JVMTI_THREAD_STATE_RUNNABLE)
#define WAITING            (JVMTI_THREAD_STATE_ALIVE |JVMTI_THREAD_STATE_WAITING |JVMTI_THREAD_STATE_WAITING_INDEFINITELY)
#define TIMED_WAITING      (JVMTI_THREAD_STATE_ALIVE |JVMTI_THREAD_STATE_WAITING |JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT)
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

// static pthread_key_t key;
thread_local Thread *curr_thread = nullptr;

Thread *get_current_thread() {
    assert(curr_thread != nullptr);
    return curr_thread;
}

static mutex new_thread_mutex;

Thread::Thread() {
    scoped_lock lock(new_thread_mutex);

    curr_thread = this;
    tid = this_thread::get_id();
    g_all_java_thread.push_back(this);
}

// Cached java.lang.Thread class
static Class *thread_class;

void Thread::bind(Object *tobj0) {
    assert(tobj0 != nullptr);
    if (tobj != nullptr) {
        return;
    }
    tobj = tobj0;
    java_lang_Thread::set_vm_thread(tobj, this);
}

// jref to_java_lang_management_ThreadInfo(const Thread *thrd, jbool locked_monitors, 
//                                         jbool locked_synchronizers, jint max_depth)
// {
//     // todo
// //    JVM_PANIC("to_java_lang_management_ThreadInfo\n");
// //    return nullptr;

//     // private volatile String name;
//     jstrRef name = thrd->tobj->getRefField("name", "Ljava/lang/String;");
//     // private long tid;
//     jlong tid = thrd->tobj->getLongField("tid");

//     Class *c = loadBootClass("java/lang/management/ThreadInfo");
//     init_class(c);
//     jref thread_info = c->allocObject();
//     // private String threadName;
//     thread_info->setRefField("threadName", "Ljava/lang/String;", name);
//     // private long threadId;
//     thread_info->setLongField("threadId", tid);

//     return thread_info;
// }

void init_java_thread();

void init_thread() {
    g_main_thread = new Thread;

    init_java_thread();

    thread_class = load_boot_class("java/lang/Thread");
    init_class(thread_class);
    
    /* Get system Thread group */

    Class *thread_group_class = load_boot_class("java/lang/ThreadGroup");
    g_sys_thread_group = Allocator::object(thread_group_class);

    // 初始化 system_thread_group
    // java/lang/ThreadGroup 的无参数构造函数主要用来：
    // Creates an empty Thread group that is not in any Thread group.
    // This method is used to create the system Thread group.
    init_class(thread_group_class);
    Method *constructor = thread_group_class->get_constructor("()V");
    execJava(constructor, { rslot(g_sys_thread_group) });

    /* Init main thread */

    jref java_thread = Allocator::object(thread_class);
    g_main_thread->bind(java_thread);
    java_lang_Thread::init(java_thread, g_sys_thread_group, MAIN_THREAD_NAME);
}

Frame *Thread::alloc_frame(Method *m, bool vm_invoke) {
    assert(m != nullptr);

    intptr_t mem = top_frame == nullptr ? (intptr_t) vm_stack : top_frame->end_address();
    size_t size = sizeof(Frame) + (m->max_locals + m->max_stack) * sizeof(slot_t);
    if (mem + size - (intptr_t) vm_stack > VM_STACK_SIZE) {
//        thread_throw(new StackOverflowError);
        // todo 栈已经溢出无法执行程序了。不要抛异常了，无法执行了。
        panic("StackOverflowError");
    }

    auto lvars = (slot_t *)(mem);
    auto new_frame = (Frame *)(lvars + m->max_locals);
    auto ostack = (slot_t *)(new_frame + 1);
    // thrd->top_frame = new (new_frame) Frame(m, vm_invoke, lvars, ostack, thrd->top_frame);  
    new ((void *)new_frame) Frame(m, vm_invoke, lvars, ostack, top_frame);
    top_frame = new_frame;
    return top_frame;
}

void Thread::pop_frame() {
    top_frame = top_frame->prev;
}

int Thread::count_stack_frames() const {
    int count = 0;
    for (Frame *frame = top_frame; frame != nullptr; frame = frame->prev) {
        count++;
    }
    return count;
}

[[noreturn]] void Thread::terminate(ExitCode code) {
    // todo 要不要调用 java.lang.Thread 中的 private void exit() 方法？ 

    exit(code);
}

jarrRef Thread::dump(int max_depth) const {
    int count = count_stack_frames();
    if (max_depth >= 0 && count > max_depth) {
        count = max_depth;
    }

    Class *c = load_boot_class("java/lang/StackTraceElement");
    // public StackTraceElement(String declaringClass, String methodName, String fileName, int lineNumber);
    Method *constructor = c->get_constructor("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

    jarrRef arr = Allocator::array("[Ljava/lang/StackTraceElement;", count);
    size_t i = count - 1;
    for (Frame *f = top_frame; f != nullptr && i >= 0; f = f->prev, i--) {
        jref o = Allocator::object(c);
        execJava(constructor, { rslot(o),
                                rslot(Allocator::string(f->method->clazz->name)),
                                rslot(Allocator::string(f->method->name)),
                                rslot(Allocator::string(f->method->clazz->source_file_name)),
                                islot(f->method->get_line_number(f->reader.pc)) }
        );
        arr->setRefElt(i, o);
    }

    return arr;
}

void Thread::jni_throw(jref excep) {
    auto t = get_current_thread();
    t->jni_exceptions.emplace_back(t->top_frame, excep);
}

void Thread::jni_throw(Class *excep_class, const char *msg) {
    assert(excep_class != nullptr);

    init_class(excep_class);
    jref o = Allocator::object(excep_class);
    if (msg == nullptr) {
        execJava(excep_class->get_constructor("()V"), { rslot(o) });
    } else {
        Method *constructor = excep_class->get_constructor("(Ljava/lang/String;)V");
        execJava(constructor, { rslot(o), rslot(Allocator::string(msg)) });
    }

    jni_throw(o);
}

jref Thread::jni_exception_occurred(Frame *f) {
    for (auto &e: get_current_thread()->jni_exceptions) {
        if (e.f == f)
            return e.o;
    }
    return nullptr;
}

bool Thread::jni_exception_clear(Frame *f) {
    auto &e = get_current_thread()->jni_exceptions;
    for (auto iter = e.begin(); iter != e.end(); iter++) {
        if (iter->f == f) {
            e.erase(iter);
            return true;
        }
    }
    return false;
}
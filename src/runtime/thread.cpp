module;
#include <cassert>
#include "../cabin.h"
#include "../classfile/class_loader.h"

module runtime;

import std.core;
import std.threading;
import vmstd;
import object;

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

//vector<Thread *> g_all_java_thread;

// static pthread_key_t key;
thread_local Thread *curr_thread;

Thread *get_current_thread() {
    return curr_thread;
}

static mutex new_thread_mutex;

Thread::Thread() {
    scoped_lock lock(new_thread_mutex);

    curr_thread = this;
    tid = this_thread::get_id();
    g_all_java_thread.push_back(this);
}

// Various field and method into java.lang.Thread cached at startup and used in thread creation
static int eetop_id;
static int thread_status_id;
static int name_id;
static int tid_id;

// Cached java.lang.Thread class
static Class *thread_class;

//Thread *g_main_thread;

void Thread::bind(Object *tobj0) {
    if (tobj != nullptr) {
        // todo error
    }
    if (tobj0 == nullptr)    
        tobj0 = Allocator::object(thread_class);

    tobj = tobj0;
    tobj->set_field_value<jlong>(eetop_id, (jlong) this);
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

void init_thread() {
    // pthread_key_create(&key, nullptr);
    
    g_main_thread = new Thread;

    thread_class = load_boot_class("java/lang/Thread");
    init_class(thread_class);

    eetop_id = thread_class->lookup_field("eetop", "J")->id;
    thread_status_id = thread_class->lookup_field("threadStatus", "I")->id;
    name_id = thread_class->lookup_field("name", "Ljava/lang/String;")->id;
    tid_id = thread_class->lookup_field("tid", "J")->id;
    
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
 
    g_main_thread->bind();
    g_main_thread->tobj->set_field_value<jint>("priority", THREAD_NORM_PRIORITY);
    setStatus(g_main_thread->tobj, RUNNING);

    // public Thread(ThreadGroup group, String name)
    constructor = thread_class->get_constructor("(Ljava/lang/ThreadGroup;Ljava/lang/String;)V");
    execJava(constructor, {
                                rslot(g_main_thread->tobj), 
                                rslot(g_sys_thread_group), 
                                rslot(Allocator::string(MAIN_THREAD_NAME))
                            });
}

Thread *Thread::from(Object *tobj) {
    assert(tobj != nullptr);
    assert(0 <= eetop_id && eetop_id < tobj->clazz->inst_fields_count);

    jlong eetop = tobj->get_field_value<jlong>(eetop_id);
    auto t = (Thread *)eetop;

    assert(t != nullptr && t->tobj == tobj);
    return t;
}

Thread *Thread::from(jlong tid) {
    unimplemented
}

void java_lang_Thread::setStatus(Object *tobj, jint status) {
    tobj->set_field_value<jint>(thread_status_id, status);
}

jint java_lang_Thread::getStatus(Object *tobj) {
    return tobj->get_field_value<jint>(thread_status_id);
}

static void *invoke_run(void *args) {
    return nullptr;

// 下面调用run方法，reference.cpp waitForReferencePendingList 会造成死循环。
// 暂时先屏蔽调用， 待 waitForReferencePendingList 正确实现后再解除   

    // jref tobj = (jref) args;
    // auto t = new Thread;
    // t->bind(tobj);
    // setStatus(tobj, RUNNING);
    // TRACE("Create a thread(%s).\n", getNameUtf8(tobj));

    // Method *run = tobj->clazz->lookupMethod(S(run), "()V");
    // return (void *) execJava(run, { rslot(tobj) });
}

void java_lang_Thread::start(jref tobj) {
//    pthread_t th;
//    if (pthread_create(&th, nullptr, invoke_run, tobj) != 0) {
//        // todo error
//        panic("pthread_create");
//    }

//    std::thread t(invoke_run, tobj);
}

bool java_lang_Thread::isAlive(jref tobj) {
    assert(tobj != nullptr);
    jint status = getStatus(tobj);
    // todo BLOCKED 算不算 alive
    return (status > 0 && status != TERMINATED); 
}

const char *java_lang_Thread::getNameUtf8(jref tobj) {
    assert(tobj != nullptr);
    return java_lang_String::to_utf8(java_lang_Thread::getName(tobj));
}

const jstrRef java_lang_Thread::getName(jref tobj) {
    assert(tobj != nullptr);
    return tobj->get_field_value<jref>(name_id);
}

const jlong java_lang_Thread::getTid(jref tobj) {
    assert(tobj != nullptr);
    return tobj->get_field_value<jlong>(tid_id);
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

void Thread::jniThrow(jref excep) {
    get_current_thread()->jni_excep = excep; 
}

void Thread::jniThrow(Class *excep_class, const char *msg) {
    assert(excep_class != nullptr);

    Thread *t = get_current_thread();
    // t->jni_excep_class = excep_class;
    // if (msg != nullptr)
    //     t->jni_excep_msg = msg;
    init_class(excep_class);
    t->jni_excep = Allocator::object(excep_class);
    assert(t->jni_excep != nullptr);
    if (msg == nullptr) {
        execJava(excep_class->get_constructor("()V"), { rslot(t->jni_excep) });
    } else {
        Method *constructor = excep_class->get_constructor("(Ljava/lang/String;)V");
        execJava(constructor, { rslot(t->jni_excep), rslot(Allocator::string(msg)) });
    }
}

jref Thread::jniExceptionOccurred() {
    return get_current_thread()->jni_excep;
    // Thread *t = getCurrentThread();
    // if (t->jni_excep != nullptr) {
    //     return t->jni_excep;
    // }

    // if (t->jni_excep_class != nullptr) {
    //     init_class(t->jni_excep_class);
    //     t->jni_excep = t->jni_excep_class->allocObject();
    //     assert(t->jni_excep != nullptr);
    //     if (t->jni_excep_msg.empty()) {
    //         execJava(t->jni_excep_class->getConstructor("()V"), { rslot(t->jni_excep) });
    //     } else {
    //         Method *constructor = t->jni_excep_class->getConstructor(S(_java_lang_String__V));
    //         execJava(constructor, { rslot(t->jni_excep), rslot(allocString(t->jni_excep_msg.c_str())) });
    //     }

    //     return t->jni_excep;
    // }
    
    // return nullptr;
}

void Thread::jniExceptionClear() {
    Thread *t = get_current_thread();
    t->jni_excep = nullptr;
    // t->jni_excep_class = nullptr;
    // t->jni_excep_msg.clear();
}
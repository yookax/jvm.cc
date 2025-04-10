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

// Various field and method into java.lang.Thread cached at startup and used in thread creation
static int eetop_id;
static int thread_status_id;
static int holder_id;
static int name_id;
static int tid_id;
static int priority_id;

// Cached java.lang.Thread class
static Class *thread_class;

// Cached java.lang.Thread$FieldHolder class
static Class *field_holder_class;

void init_java_thread() {
    thread_class = load_boot_class("java/lang/Thread");
    field_holder_class = load_boot_class("java/lang/Thread$FieldHolder");
    init_class(thread_class);
    init_class(field_holder_class);

    eetop_id = thread_class->lookup_field("eetop", "J")->id;
    name_id = thread_class->lookup_field("name", "Ljava/lang/String;")->id;
    holder_id = thread_class->lookup_field("holder", "Ljava/lang/Thread$FieldHolder;")->id;
    tid_id = thread_class->lookup_field("tid", "J")->id;

    thread_status_id = field_holder_class->lookup_field("threadStatus", "I")->id;
}

void java_lang_Thread::init(jref java_thread, jref thread_group, const char *name) {
    // public Thread(ThreadGroup group, String name)
    Method *constructor = thread_class->get_constructor("(Ljava/lang/ThreadGroup;Ljava/lang/String;)V");
    execJava(constructor, {
            rslot(java_thread),
            rslot(thread_group),
            rslot(Allocator::string(name))
    });
}

void java_lang_Thread::set_vm_thread(Object *java_thread, Thread *t) {
    assert(java_thread != nullptr);
    assert(t != nullptr);
    java_thread->set_field_value<jlong>(eetop_id, (jlong) t);
}

Thread *java_lang_Thread::get_vm_thread(Object *java_thread) {
    assert(java_thread != nullptr);
    return (Thread *) get_eetop(java_thread);
}

void java_lang_Thread::setStatus(Object *java_thread, jint status) {
    jref holder = java_thread->get_field_value<jref>(holder_id);
    holder->set_field_value<jint>(thread_status_id, status);
}

jint java_lang_Thread::getStatus(Object *java_thread) {
    jref holder = java_thread->get_field_value<jref>(holder_id);
    return holder->get_field_value<jint>(thread_status_id);
}

jlong java_lang_Thread::get_eetop(jref java_thread) {
    assert(java_thread != nullptr);
    return java_thread->get_field_value<jlong>(eetop_id);
}

bool java_lang_Thread::isAlive(jref java_thread) {
    assert(java_thread != nullptr);
    return get_eetop(java_thread) != 0;
}

const char *java_lang_Thread::getNameUtf8(jref java_thread) {
    assert(java_thread != nullptr);
    return java_lang_String::to_utf8(java_lang_Thread::getName(java_thread));
}

const jstrRef java_lang_Thread::getName(jref java_thread) {
    assert(java_thread != nullptr);
    return java_thread->get_field_value<jref>(name_id);
}

const jlong java_lang_Thread::getTid(jref java_thread) {
    assert(java_thread != nullptr);
    return java_thread->get_field_value<jlong>(tid_id);
}

static condition_variable cv;
static mutex cv_mtx;

static void invoke_run(std::thread *local_thread, jref java_thread) {
// 下面调用run方法，reference.cpp waitForReferencePendingList 会造成死循环。
// 暂时先屏蔽调用， 待 waitForReferencePendingList 正确实现后再解除

    unique_lock<mutex> lock(cv_mtx);

    //printvm("%s\n", getNameUtf8(java_thread));

     auto t = new Thread;
     t->bind(local_thread, java_thread);
//     setStatus(java_thread, RUNNING);
     TRACE("Create a thread(%s).\n", getNameUtf8(java_thread));
    lock.unlock();
    cv.notify_one();

    Method *run = java_thread->clazz->lookup_method("run", "()V");
    execJava(run, { rslot(java_thread) });
}

static mutex new_thread_mutex;

void java_lang_Thread::start(jref java_thread) {
    scoped_lock lock(new_thread_mutex);
    unique_lock<mutex> _lock(cv_mtx);

    auto x = new std::thread;
    std::thread t(invoke_run, x, java_thread);
    cv.wait(_lock);

    *x = std::move(t);
//    t.detach();
}

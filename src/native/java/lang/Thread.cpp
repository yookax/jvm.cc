module;
#include "../../../vmdef.h"

module native;

import std.core;
import std.threading;
import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

using namespace std;

// Search the stack for the most recent scoped-value bindings.
// static native Object findScopedValueBindings();
void findScopedValueBindings(Frame *f) {
    unimplemented
}

/*
 * Returns the Thread object for the current platform thread. If the
 * current thread is a virtual thread then this method returns the carrier.
 */
// static native Thread currentCarrierThread();
void currentCarrierThread(Frame *f) {
    //Thread *t = get_current_thread();
    //f->pushr(t->tobj);
    // todo virtual thread
    unimplemented
}

/*
 * Returns the Thread object for the current thread.
 * @return  the current thread
 */
// public static native Thread currentThread();
void currentThread(Frame *f) {
    Thread *t = get_current_thread();
    f->pushr(t->java_thread);
}

/*
 * Sets the Thread object to be returned by Thread.currentThread().
 */
// native void setCurrentThread(Thread thread);
void setCurrentThread(Frame *f) {
    unimplemented
}

// static native Object[] scopedValueCache();
void scopedValueCache(Frame *f) {
    unimplemented
}

// static native void setScopedValueCache(Object[] cache);
void setScopedValueCache(Frame *f) {
    unimplemented
}

// static native void ensureMaterializedForStackWalk(Object o);
void ensureMaterializedForStackWalk(Frame *f) {
    unimplemented
}

// private static native void yield0();
void yield0(Frame *f) {
    this_thread::yield();
}

// private static native void sleepNanos0(long nanos) throws InterruptedException;
void sleepNanos0(Frame *f) {
    slot_t *args = f->lvars;
    auto nanos = slot::get<jlong>(args);
    this_thread::sleep_for(std::chrono::nanoseconds(nanos));
}

// private native void start0();
void start0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    java_lang_Thread::start(_this);
}

/*
 * Returns {@code true} if and only if the current thread holds the
 * monitor lock on the specified object.
 *
 * <p>This method is designed to allow a program to assert that
 * the current thread already holds a specified lock:
 * <pre>
 *     assert Thread.holdsLock(obj);
 * </pre>
 *
 * @param  obj the object on which to test lock ownership
 * @return {@code true} if the current thread holds the monitor lock on
 *         the specified object.
 * @since 1.4
 */
// public static native boolean holdsLock(Object obj);
void holdsLock(Frame *f) {
    unimplemented
}

// private native Object getStackTrace0();
void getStackTrace0(Frame *f) {
    unimplemented
}

// private static native StackTraceElement[][] dumpThreads(Thread[] threads);
void dumpThreads(Frame *f) {
    slot_t *args = f->lvars;
    auto threads = slot::get<jref>(args);
    assert(threads->is_array_object());

    size_t len = threads->arr_len;
    jarrRef result = Allocator::array("[[java/lang/StackTraceElement", len);

    for (size_t i = 0; i < len; i++) {
        jref tobj = threads->getElt<jref>(i);
        Thread *thread = java_lang_Thread::get_vm_thread(tobj);
        jarrRef arr = thread->dump(-1);
        result->setRefElt(i, arr);
    }

    f->pushr(result);
}

// private static native Thread[] getThreads();
void getThreads(Frame *f) {
    jarrRef threads = Allocator::array("[Ljava/lang/Thread;", g_all_java_thread.size());

    int i = 0;
    for (Thread *t: g_all_java_thread) {
        threads->setRefElt(i, t->java_thread);
        i++;
    }

    f->pushr(threads);
}

// private native void setPriority0(int newPriority);
void setPriority0(Frame *f) {
    // todo
}

// private native void interrupt0();
void interrupt0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    Thread *t = java_lang_Thread::get_vm_thread(_this);
    t->interrupted = true;
}

// private static native void clearInterruptEvent();
void clearInterruptEvent(Frame *f) {
    unimplemented
}

// private native void setNativeName(String name);
void setNativeName(Frame *f) {
    unimplemented
}

// The address of the next thread identifier, see ThreadIdentifiers.
// private static native long getNextThreadIdOffset();
void getNextThreadIdOffset(Frame *f) {
    // todo
    static jlong NEXT_TID_OFFSET = 100;
    f->pushl((jlong) &NEXT_TID_OFFSET);
}

// private static native void registerNatives();
void java_lang_Thread_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Thread", #method, method_descriptor, method)

#define THD "Ljava/lang/Thread;"
#define OBJ "Ljava/lang/Object;"
#define STE "Ljava/lang/StackTraceElement;"
#define STR "Ljava/lang/String;"

    R(start0, "()V");
    R(setPriority0, "(I)V");
    R(yield0, "()V");
    R(sleepNanos0, "(J)V");
    R(currentCarrierThread, "()" THD);
    R(currentThread, "()" THD);
    R(setCurrentThread, "(" THD ")V");
    R(interrupt0, "()V");
    R(holdsLock, "(" OBJ ")Z");
    R(getThreads, "()[" THD);
    R(dumpThreads, "([" THD ")[[" STE);
    R(getStackTrace0, "()" OBJ);
    R(setNativeName, "(" STR ")V");
    R(scopedValueCache, "()[" OBJ);
    R(setScopedValueCache, "([" OBJ ")V");
    R(getNextThreadIdOffset, "()J");
    R(findScopedValueBindings, "()" OBJ);
    R(ensureMaterializedForStackWalk, "(" OBJ ")V");
}
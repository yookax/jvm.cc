module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;

/* Returns the class of the caller of the method calling this method,
    ignoring frames associated with java.lang.reflect.Method.invoke()
    and its implementation. */
//public static native Class<?> getCallerClass();
void getCallerClass(Frame *f) {
    // top0, current frame is executing getCallerClass()
    // top1, who called getCallerClass, the one who wants to know his caller.
    // top2, the caller of top1, the result.
    auto frame = (Frame *) get_current_thread()->top_frame;
    Frame *top1 = frame->prev;
    assert(top1 != nullptr);

    Frame *top2 = top1->prev;
    assert(top2 != nullptr);

    jclsRef o = top2->method->clazz->java_mirror;
    f->pushr(o);
}

/* Retrieves the access flags written to the class file. For
    inner classes these flags may differ from those returned by
    Class.getModifiers(), which searches the InnerClasses
    attribute to find the source-level access flags. This is used
    instead of Class.getModifiers() for run-time access checks due
    to compatibility reasons; see 4471811. Only the values of the
    low 13 bits (i.e., a mask of 0x1FFF) are guaranteed to be
    valid. */
//public static native int getClassAccessFlags(Class<?> c);
void getClassAccessFlags(Frame *f) {
    slot_t *args = f->lvars;
    auto co = slot::get<jref>(args);
    f->pushi(co->jvm_mirror->access_flags.get()); // todo
}

/*
 * Returns true if {@code currentClass} and {@code memberClass}
 * are nestmates - that is, if they have the same nesthost as
 * determined by the VM.
 */
//public static native boolean areNestMates(Class<?> currentClass, Class<?> memberClass);
void areNestMates(Frame *f) {
    slot_t *args = f->lvars;
    auto current = slot::get<jref>(args++);
    auto member = slot::get<jref>(args);

    Class *c = current->jvm_mirror;
    Class *d = member->jvm_mirror;
    auto x = c->test_nest_mate(d);
    f->pushi(x ? jtrue : jfalse);
}

void jdk_internal_reflect_Reflection_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/reflect/Reflection", #method, method_descriptor, method)

    R(getCallerClass, "()Ljava/lang/Class;");
    R(getClassAccessFlags, "(Ljava/lang/Class;)I");
    R(areNestMates, "(Ljava/lang/Class;Ljava/lang/Class;)Z");
}
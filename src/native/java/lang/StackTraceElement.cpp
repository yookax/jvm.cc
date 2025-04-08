module;
#include "../../../vmdef.h"

module native;

import slot;
import object;
import runtime;

/*
 * Sets the given stack trace elements with the backtrace of the given Throwable.
 */
//private static native void initStackTraceElements(StackTraceElement[] elements, Object x, int depth);
void initStackTraceElements(Frame *f) {
    unimplemented

//    slot_t *args = f->lvars;
//    auto elements = slot::get<jref>(args++);
//    auto throwable = slot::get<jref>(args++);
//    auto depth = slot::get<jint>(args);
//
//    jref x = (jref) throwable;
//
//    jref backtrace = x->get_field_value<jref>("backtrace", "Ljava/lang/Object;");
//    if (!backtrace->is_array_object()) {
//        panic("error"); // todo
//    }
//
//    assert(elements->arr_len <= backtrace->arr_len);
//    assert(depth <= backtrace->arr_len);
//    assert(depth <= elements->arr_len);
//    int skip = backtrace->arr_len - depth;
//    auto src = ((jref) backtrace->data) + skip;
//    memcpy(elements->data, src, elements->arr_len*sizeof(jref));
}

/*
 * Sets the given stack trace element with the given StackFrameInfo
 */
//private static native void initStackTraceElement(StackTraceElement element, StackFrameInfo sfi);
void initStackTraceElement(Frame *f) {
    unimplemented
}

void java_lang_StackTraceElement_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/StackTraceElement", #method, method_descriptor, method)

    R(initStackTraceElements, "([Ljava/lang/StackTraceElement;Ljava/lang/Object;I)V");
}
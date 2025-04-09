module;
#include "../../../../vmdef.h"

module native;

import std.threading;
import slot;
import runtime;
import object;
import class_loader;

/*
 * Atomically get and clear (set to null) the VM's pending-Reference list.
 */
// private static native Reference<?> getAndClearReferencePendingList();
void getAndClearReferencePendingList(Frame *f) {
    f->pushr(nullptr);
    //unimplemented // todo
}

/*
 * Test whether the VM's pending-Reference list contains any entries.
 */
// private static native boolean hasReferencePendingList();
void hasReferencePendingList(Frame *f) {
    unimplemented // todo
}

/*
 * Wait until the VM's pending-Reference list may be non-null.
 */
// private static native void waitForReferencePendingList();
void waitForReferencePendingList(Frame *f) {
    // todo
    std::this_thread::sleep_for(std::chrono::hours(100));
}

// private native boolean refersTo0(Object o);
void refersTo0(Frame *f) {
    // todo

    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto obj = slot::get<jref>(args);

    Class *c = load_boot_class("java/lang/ref/Reference");
    auto referent_field_id = c->get_field("referent")->id;
    jref referent = _this->get_field_value<jref>(referent_field_id);
    bool b = (referent == obj);
    f->pushi(b ? jtrue : jfalse);
}

// private native void clear0();
void clear0(Frame *f) {
    // todo
}

void java_lang_ref_Reference_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/ref/Reference", #method, method_descriptor, method)

    R(getAndClearReferencePendingList, "()Ljava/lang/ref/Reference;");
    R(hasReferencePendingList, "()Z");
    R(waitForReferencePendingList, "()V");
    R(refersTo0, "(Ljava/lang/Object;)Z");
    R(clear0, "()V");
}
module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;

/* Find the signal number, given a name. Returns -1 for unknown signals. */
//private static native int findSignal0(String sigName);
void findSignal0(Frame *f) {
    f->pushi(0); // todo
}

/* Registers a native signal handler, and returns the old handler.
 * Handler values:
 *   0     default handler
 *   1     ignore the signal
 *   2     call back to Signal.dispatch
 *   other arbitrary native signal handlers
 */
//private static native long handle0(int sig, long nativeHandler);
void handle0(Frame *f) {
    slot_t *args = f->lvars;
    auto sig = slot::get<jint>(args++);
    auto handler = slot::get<jlong>(args++);

    f->pushl(handler);
}

/* Raise a given signal number */
//private static native void raise0(int sig);
void raise0(Frame *f) {
    unimplemented
}

void jdk_internal_misc_Signal_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/Signal", #method, method_descriptor, method)

    R(findSignal0, "(Ljava/lang/String;)I");
    R(handle0, "(IJ)J");
    R(raise0, "(I)V");
}
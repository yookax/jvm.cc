module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;
import object;
import exception;


/*
 * Atomically get and clear (set to null) the VM's pending-Reference list.
 */
// private static native Reference<?> getAndClearReferencePendingList();

/*
 * Test whether the VM's pending-Reference list contains any entries.
 */
// private static native boolean hasReferencePendingList();

/*
 * Wait until the VM's pending-Reference list may be non-null.
 */
// private static native void waitForReferencePendingList();

// private native boolean refersTo0(Object o);

// private native void clear0();





void java_lang_ref_Reference_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/ref/Reference", #method, method_descriptor, method)

}
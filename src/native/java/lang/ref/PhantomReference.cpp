module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;

// private native boolean refersTo0(Object o);

// private native void clear0();
static void clear0(Frame *f) {
    // todo
}

void java_lang_ref_PhantomReference_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/ref/PhantomReference", #method, method_descriptor, method)

    R(clear0, "()V");
}
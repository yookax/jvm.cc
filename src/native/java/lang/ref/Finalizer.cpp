module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;

// private static native boolean isFinalizationEnabled();
void isFinalizationEnabled(Frame *f) {
    // todo
    f->pushi(jfalse);
}

// private static native void reportComplete(Object finalize);
void reportComplete(Frame *f) {
    unimplemented
}

void java_lang_ref_Finalizer_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/ref/Finalizer", #method, method_descriptor, method)

    R(isFinalizationEnabled, "()Z");
}
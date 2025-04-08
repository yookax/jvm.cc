module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;

// private static native Object invoke0(Method m, Object obj, Object[] args);
static void invoke0(Frame *f) {
    slot_t *args = f->lvars;
    auto m = slot::get<jref>(args++);
    auto obj = slot::get<jref>(args++);
    auto _args = slot::get<jref>(args);

    unimplemented
}

void jdk_internal_reflect_DirectMethodHandleAccessor$NativeAccessor_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/reflect/DirectMethodHandleAccessor$NativeAccessor", #method, method_descriptor, method)

    R(invoke0, "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
}
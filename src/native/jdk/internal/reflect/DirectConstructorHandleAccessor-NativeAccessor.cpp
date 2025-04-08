module;
#include "../../../../vmdef.h"

module native;

import slot;
import runtime;

// private static native Object newInstance0(Constructor<?> c, Object[] args)
//                    throws InstantiationException, InvocationTargetException;
static void newInstance0(Frame *f) {
    slot_t *args = f->lvars;
    auto c = slot::get<jref>(args++);
    auto _args = slot::get<jref>(args);

    unimplemented
}

void jdk_internal_reflect_DirectConstructorHandleAccessor$NativeAccessor_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/reflect/DirectConstructorHandleAccessor$NativeAccessor", #method, method_descriptor, method)

    R(newInstance0, "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)Ljava/lang/Object;");
}
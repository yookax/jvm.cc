module;
#include "../../../../vmdef.h"

module native;

import std;
import slot;
import classfile;
import object;
import runtime;

// native void closeScope0(MemorySessionImpl session, ScopedAccessError error);
void closeScope0(Frame *f) {
    unimplemented
}

void jdk_internal_misc_ScopedMemoryAccess_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/ScopedMemoryAccess", #method, method_descriptor, method)

    R(closeScope0, "(Ljdk/internal/foreign/MemorySessionImpl;"
                    "Ljdk/internal/misc/ScopedMemoryAccess$Scope$ScopedAccessError;)V");
}
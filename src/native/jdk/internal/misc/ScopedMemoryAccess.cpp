module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;


void jdk_internal_misc_ScopedMemoryAccess_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/ScopedMemoryAccess", #method, method_descriptor, method)


}
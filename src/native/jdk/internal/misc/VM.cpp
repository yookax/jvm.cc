module;
#include <assert.h>
#include "../../../../vmdef.h"

module native;

import std.core;
import std.threading;
import slot;
import object;
import classfile;
import constants;
import runtime;
import class_loader;

//private static native void initialize();
static void initialize(Frame *f) {
    // TODO
}

void jdk_internal_misc_VM_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/VM", #method, method_descriptor, method)

    R(initialize, "()V");
}
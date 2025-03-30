#ifdef _WIN64
module;
#include "../../../vmdef.h"
#include <windows.h>

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

// Win32 SetErrorMode
// private static native long setErrorMode(long mode);
static void setErrorMode(Frame *f) {
    slot_t *args = f->lvars;
    auto mode = slot::get<jlong>(args);
    auto old_mode  = (jlong) SetErrorMode((UINT)mode);
    f->pushl(old_mode);
}

void sun_io_Win32ErrorMode_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("sun/io/Win32ErrorMode", #method, method_descriptor, method)

    R(setErrorMode, "(J)J");
}
#endif
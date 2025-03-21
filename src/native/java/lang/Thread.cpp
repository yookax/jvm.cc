#include <cassert>

module native;

import runtime;

// private static native void registerNatives();
void java_lang_Thread_registerNatives(Frame *f) {
    assert(f != nullptr);
    //registry("", "", "", nullptr);
}
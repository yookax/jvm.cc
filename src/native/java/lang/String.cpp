module;
#include "../../../vmdef.h"

module native;

import slot;
import object;
import runtime;

// public native String intern();
void intern(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    f->pushr(java_lang_String::intern(_this));
}

void java_lang_String_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/String", #method, method_descriptor, method)

    R(intern, "()Ljava/lang/String;");
}
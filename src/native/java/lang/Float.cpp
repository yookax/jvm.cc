module;
#include "../../../vmdef.h"

module native;

import slot;
import runtime;

// Find the bit pattern corresponding to a given float, NOT collapsing NaNs
// public static native int floatToRawIntBits(float value);
void floatToRawIntBits(Frame *f) {
    slot_t *args = f->lvars;
    auto value = slot::get<jfloat>(args);

    union {
        jint i;
        jfloat f;
    } u;
    u.f = value;
    f->pushi(u.i);
}

// Find the float corresponding to a given bit pattern
// public static native float intBitsToFloat(int bits);
void intBitsToFloat(Frame *f) {
    slot_t *args = f->lvars;
    auto bits = slot::get<jint>(args);

    union {
        jint i;
        jfloat f;
    } u;
    u.i = bits;
    f->pushf(u.f);
}

void java_lang_Float_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Float", #method, method_descriptor, method)

    R(floatToRawIntBits, "(F)I");
    R(intBitsToFloat, "(I)F");
}


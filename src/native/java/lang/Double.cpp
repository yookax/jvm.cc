module;
#include "../../../vmdef.h"

module native;

import slot;
import runtime;

// Find the bit pattern corresponding to a given float, NOT collapsing NaNs
// public static native long doubleToRawLongBits(double value);
void doubleToRawLongBits(Frame *f) {
    unimplemented
}

// Find the double float corresponding to a given bit pattern
// public static native double longBitsToDouble(long bits);
void longBitsToDouble(Frame *f) {
    unimplemented
}

void java_lang_Double_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/double", #method, method_descriptor, method)

    R(doubleToRawLongBits, "(D)J");
    R(longBitsToDouble, "(J)D");
}


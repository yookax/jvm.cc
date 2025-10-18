module;
#include "../../../vmdef.h"

module native;

import std.core;
import slot;
import runtime;
import convert;

// Find the bit pattern corresponding to a given float, NOT collapsing NaNs
// public static native long doubleToRawLongBits(double value);
void doubleToRawLongBits(Frame *f) {
    slot_t *args = f->lvars;
    auto value = slot::get<jdouble>(args);

    uint8_t bytes[sizeof(value)];
    doubleToBytes(value, bytes, std::endian::native);
    jlong x = bytesToInt64(bytes, std::endian::native);
    f->pushl(x);
}

// Find the double float corresponding to a given bit pattern
// public static native double longBitsToDouble(long bits);
void longBitsToDouble(Frame *f) {
    slot_t *args = f->lvars;
    auto bits = slot::get<jlong>(args);

    uint8_t bytes[sizeof(bits)];
    int64ToBytes(bits, bytes, std::endian::native);
    jdouble x = bytesToDouble(bytes, std::endian::native);
    f->pushd(x);
}

void java_lang_Double_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Double", #method, method_descriptor, method)

    R(doubleToRawLongBits, "(D)J");
    R(longBitsToDouble, "(J)D");
}


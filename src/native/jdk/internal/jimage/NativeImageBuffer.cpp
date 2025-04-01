module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;
import class_loader;

// static native ByteBuffer getNativeMap(String imagePath);
void getNativeMap(Frame *f) {
    slot_t *args = f->lvars;
    auto path = slot::get<jref>(args);

    auto image_path = java_lang_String::to_utf8(path);
    cout << image_path << endl; // C:\Java\jdk-17.0.12\lib\modules

    int i =3;
}

void jdk_internal_jimage_NativeImageBuffer_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/jimage/NativeImageBuffer", #method, method_descriptor, method)

    //R(getNativeMap, "(Ljava/lang/String;)Ljava/nio/ByteBuffer;");
}
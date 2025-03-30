module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;
import class_loader;

/*
 * Return true if the given library is successfully loaded.
 * If the given library cannot be loaded for any reason,
 * if throwExceptionIfFail is false, then this method returns false;
 * otherwise, UnsatisfiedLinkError will be thrown.
 *
 * JNI FindClass expects the caller class if invoked from JNI_OnLoad
 * and JNI_OnUnload is NativeLibrary class.
 */
// private static native boolean load(NativeLibraryImpl impl, String name,
//                                    boolean isBuiltin, boolean throwExceptionIfFail);
void load(Frame *f) {

}

/*
 * Unload the named library.  JNI_OnUnload, if present, will be invoked
 * before the native library is unloaded.
 */
// private static native void unload(String name, boolean isBuiltin, long handle);
void unload(Frame *f) {
    unimplemented
}

// private static native String findBuiltinLib(String name);
void findBuiltinLib(Frame *f) {

}

void jdk_internal_loader_NativeLibraries_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/loader/NativeLibraries", #method, method_descriptor, method)

//    R(load, "(Ljdk/internal/loader/NativeLibraries$NativeLibraryImpl;Ljava/lang/String;ZZ)Z");
//    R(unload, "(Ljava/lang/String;ZJ)V");
//    R(findBuiltinLib, "(Ljava/lang/String;)Ljava/lang/String;");
}
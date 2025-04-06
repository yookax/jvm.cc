module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;
import exception;
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
// private static native boolean load(NativeLibraryImpl lib, String name,
//                                    boolean isBuiltin, boolean throwExceptionIfFail);
void load(Frame *f) {
    slot_t *args = f->lvars;
    auto lib = slot::get<jref>(args++);
    auto name = slot::get<jref>(args++);
    auto is_builtin = slot::get<jbool>(args++);
    auto throw_exception_if_fail = slot::get<jbool>(args);

    unimplemented
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
    slot_t *args = f->lvars;
    auto name_obj = slot::get<jref>(args);

    if (name_obj == nullptr) {
        throw java_lang_InternalError("NULL filename for native library");
    }

    size_t prefix_len = strlen(JNI_LIB_PREFIX);
    size_t suffix_len = strlen(JNI_LIB_SUFFIX);
    auto name = java_lang_String::to_utf8(name_obj);
    size_t len = strlen(name);
    if (len <= (prefix_len + suffix_len)) {
        f->pushr(nullptr);
        return;
    }

    auto lib_name = new char[len + 1]; // +1 for null if prefix+suffix == 0
    strcpy(lib_name, name + prefix_len);
    // Strip SUFFIX
    lib_name[strlen(lib_name) - suffix_len] = '\0';

    auto so = Allocator::string(lib_name);
    delete[] lib_name;
    f->pushr(so);
}

void jdk_internal_loader_NativeLibraries_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/loader/NativeLibraries", #method, method_descriptor, method)

    R(load, "(Ljdk/internal/loader/NativeLibraries$NativeLibraryImpl;Ljava/lang/String;ZZ)Z");
    R(unload, "(Ljava/lang/String;ZJ)V");
    R(findBuiltinLib, "(Ljava/lang/String;)Ljava/lang/String;");
}
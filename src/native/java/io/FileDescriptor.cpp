module;
#include "../../../vmdef.h"
#ifdef _WIN64
#include <windows.h>
#elifdef __linux__
#endif

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

/* field id for jint 'fd' in java.io.FileDescriptor */
static Field *fd_field;

/* field id for jlong 'handle' in java.io.FileDescriptor */
static Field *handle_field;

/* field id for jboolean 'append' in java.io.FileDescriptor */
static Field *append_field;

/* This routine initializes JNI field offsets for the class */
// private static native void initIDs();
static void initIDs(Frame *f) {
    Class *c = load_boot_class("java/io/FileDescriptor");
    fd_field = c->get_field("fd", "I");
    handle_field = c->get_field("handle", "J");
    append_field = c->get_field("append", "Z");
}

/* fsync/equivalent this file descriptor */
// private native void sync0() throws SyncFailedException;
static void sync0(Frame *f) {
    unimplemented
}

/*
 * On Windows return the handle for the standard streams.
 */
// private static native long getHandle(int fd);
static void getHandle(Frame *f) {
    slot_t *args = f->lvars;
    auto fd = slot::get<jint>(args);
#ifdef _WIN64
    jlong handle = -1;
    if (fd == 0) {
        handle = (jlong) GetStdHandle(STD_INPUT_HANDLE);
    } else if (fd == 1) {
        handle = (jlong) GetStdHandle(STD_OUTPUT_HANDLE);
    } else if (fd == 2) {
        handle = (jlong) GetStdHandle(STD_ERROR_HANDLE);
    }
    assert(handle != -1);
    f->pushl(handle);
#elifdef __linux__
    unimplemented
#endif
}

/*
 * Returns true, if the file was opened for appending.
 */
// private static native boolean getAppend(int fd);
static void getAppend(Frame *f) {
    f->pushi(0);
}

/*
 * Close the raw file descriptor or handle, if it has not already been closed
 * and set the fd and handle to -1.
 */
// private native void close0() throws IOException;
static void close0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);

    auto handle = _this->get_field_value<jlong>(handle_field);
    if (handle != -1) {
        CloseHandle((HANDLE) handle);
        _this->set_field_value(handle_field, (jlong) -1);
    }

    _this->set_field_value(fd_field, (jlong) -1);
}

void java_io_FileDescriptor_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileDescriptor", #method, method_descriptor, method)

   // R(initIDs, "()V");
   // R(sync0, "()V");
    R(getHandle, "(I)J");
    R(getAppend, "(I)Z");
  //  R(close0, "()V");
}
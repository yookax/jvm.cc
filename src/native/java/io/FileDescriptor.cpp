module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;


/* fsync/equivalent this file descriptor */
// private native void sync0() throws SyncFailedException;

/* This routine initializes JNI field offsets for the class */
// private static native void initIDs();

/*
 * On Windows return the handle for the standard streams.
 */
// private static native long getHandle(int d);

/*
 * Returns true, if the file was opened for appending.
 */
// private static native boolean getAppend(int fd);

/*
 * Close the raw file descriptor or handle, if it has not already been closed
 * and set the fd and handle to -1.
 */
// private native void close0() throws IOException;


void java_io_FileDescriptor_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileDescriptor", #method, method_descriptor, method)

}
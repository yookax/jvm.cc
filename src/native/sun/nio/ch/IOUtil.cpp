module;
#include "../../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

//static native void initIDs();
static void initIDs(Frame *f) {
    unimplemented
}

//static native boolean randomBytes(byte[] someBytes);
static void randomBytes(Frame *f) {
    unimplemented
}

/*
 * Returns two file descriptors for a pipe encoded in a long.
 * The read end of the pipe is returned in the high 32 bits,
 * while the write end is returned in the low 32 bits.
 */
//static native long makePipe(boolean blocking) throws IOException;
static void makePipe(Frame *f) {
    unimplemented
}

//static native int write1(int fd, byte b) throws IOException;
static void write1(Frame *f) {
    unimplemented
}

/*
 * Read and discard all bytes.
 */
//static native boolean drain(int fd) throws IOException;
static void drain(Frame *f) {
    unimplemented
}

/*
 * Read and discard at most one byte
 * @return the number of bytes read or IOS_INTERRUPTED
 */
//static native int drain1(int fd) throws IOException;
static void drain1(Frame *f) {
    unimplemented
}

//public static native void configureBlocking(FileDescriptor fd, boolean blocking) throws IOException;
static void configureBlocking(Frame *f) {
    unimplemented
}

//public static native int fdVal(FileDescriptor fd);
static void fdVal(Frame *f) {
    unimplemented
}

//static native void setfdVal(FileDescriptor fd, int value);
static void setfdVal(Frame *f) {

}

//static native int fdLimit();
static void fdLimit(Frame *f) {
    unimplemented
}

//static native int iovMax();
static void iovMax(Frame *f) {
    unimplemented
}

//static native long writevMax();
static void writevMax(Frame *f) {
    unimplemented
}

void sun_nio_ch_IOUtil_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("sun/nio/ch/IOUtil", #method, method_descriptor, method)

//    R(initIDs, "()V");
//    R(randomBytes, "");
//    R(makePipe, "");
//    R(write1, "");
//    R(drain, "");
//    R(drain1, "");
//    R(configureBlocking, "");
//    R(fdVal, "");
//    R(setfdVal, "");
//    R(fdLimit, "");
//    R(iovMax, "()I");
//    R(writevMax, "");
}
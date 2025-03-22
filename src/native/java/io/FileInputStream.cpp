module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

/*
 * Opens the specified file for reading.
 * @param name the name of the file
 */
//private native void open0(String name) throws FileNotFoundException;

//private native int read0() throws IOException;

/*
 * Reads a subarray as a sequence of bytes.
 * @param     b the data to be written
 * @param     off the start offset in the data
 * @param     len the number of bytes that are written
 * @throws    IOException If an I/O error has occurred.
 */
//private native int readBytes(byte[] b, int off, int len) throws IOException;

//private native long length0() throws IOException;

//private native long position0() throws IOException;

//private native long skip0(long n) throws IOException;

//private native int available0() throws IOException;

//private native boolean isRegularFile0(FileDescriptor fd);

//private static native void initIDs();

void java_io_FileInputStream_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileInputStream", #method, method_descriptor, method)

}
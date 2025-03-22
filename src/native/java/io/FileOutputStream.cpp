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
 * Opens a file, with the specified name, for overwriting or appending.
 * @param name name of file to be opened
 * @param append whether the file is to be opened in append mode
 */
//private native void open0(String name, boolean append) throws FileNotFoundException;

/*
 * Writes the specified byte to this file output stream.
 *
 * @param   b   the byte to be written.
 * @param   append   {@code true} if the write operation first
 *     advances the position to the end of file
 */
//private native void write(int b, boolean append) throws IOException;


/*
 * Writes a sub array as a sequence of bytes.
 * @param b the data to be written
 * @param off the start offset in the data
 * @param len the number of bytes that are written
 * @param append {@code true} to first advance the position to the
 *     end of file
 * @throws    IOException If an I/O error has occurred.
 */
//private native void writeBytes(byte[] b, int off, int len, boolean append) throws IOException;


//private static native void initIDs();


void java_io_FileOutputStream_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileOutputStream", #method, method_descriptor, method)

}
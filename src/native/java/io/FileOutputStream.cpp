module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

// The system dependent file descriptor.
static Field *fd_field;

//private static native void initIDs();
static void initIDs(Frame *f) {
    Class *c = load_boot_class("java/io/FileOutputStream");
    fd_field = c->get_field("fd", "Ljava/io/FileDescriptor;");
}

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
static void writeBytes(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto b = slot::get<jref>(args++);
    auto off = slot::get<jint>(args++);
    auto len = slot::get<jint>(args++);
    auto append = slot::get<jbool>(args);

    if (b == nullptr)
        throw java_lang_NullPointerException();
    if (off < 0 || off + len > b->arr_len)
        throw java_lang_ArrayIndexOutOfBoundsException();

    auto fd = _this->get_field_value<jref>(fd_field);
    assert(fd != nullptr);
    FILE *file = (FILE *) fd->get_field_value<jlong>("handle", "J");
    if (file == nullptr)
        throw java_io_IOException("Stream Closed");

    void *buf = b->index(off);
    size_t size = fwrite(buf, 1, len, file);
    assert(size == len);
    fflush(file);
}

void java_io_FileOutputStream_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileOutputStream", #method, method_descriptor, method)

//    R(initIDs, "()V");
//    R(writeBytes, "([BIIZ)V");
}
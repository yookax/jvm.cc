module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

/* File Descriptor - handle to the open file */
//private final FileDescriptor fd;
static Field *fd_field;

//private static native void initIDs();
static void initIDs(Frame *f) {
    Class *c = load_boot_class("java/io/FileInputStream");
    fd_field = c->get_field("fd", "Ljava/io/FileDescriptor;");
}

/*
 * Opens the specified file for reading.
 * @param name the name of the file
 */
//private native void open0(String name) throws FileNotFoundException;
static void open0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto _name = slot::get<jref>(args);
    auto name = java_lang_String::to_utf8(_name);

    FILE *file = fopen(name, "rb");
    if (file == nullptr)
        throw java_io_FileNotFoundException(name);

    auto fd = _this->get_field_value<jref>(fd_field);
    if (fd != nullptr) {
        fd->set_field_value("handle", "J", (jlong) file);
        fd->set_field_value("append", "Z", (jbool) 0);
    }
}

//private native int read0() throws IOException;

/*
 * Reads a subarray as a sequence of bytes.
 * @param     b the data to be written
 * @param     off the start offset in the data
 * @param     len the number of bytes that are written
 * @throws    IOException If an I/O error has occurred.
 */
//private native int readBytes(byte[] b, int off, int len) throws IOException;
static void readBytes(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto b = slot::get<jref>(args++);
    auto off = slot::get<jint>(args++);
    auto len = slot::get<jint>(args);

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
    size_t size = fread(buf, 1, len, file);
    assert(size == len);
    f->pushi((jint) size);
}

//private native long length0() throws IOException;

//private native long position0() throws IOException;

//private native long skip0(long n) throws IOException;

//private native int available0() throws IOException;

//private native boolean isRegularFile0(FileDescriptor fd);

void java_io_FileInputStream_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/FileInputStream", #method, method_descriptor, method)

//    R(initIDs, "()V");
//    R(open0, "(Ljava/lang/String;)V");
//    R(readBytes, "([BII)I");
}
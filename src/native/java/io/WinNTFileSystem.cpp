#ifdef _WIN64
module;
#include "../../../vmdef.h"
#include <windows.h>

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

static Field *path_field;

//private static native void initIDs();
static void initIDs(Frame *f) {
    Class *file_class = load_boot_class("java/io/File");
    path_field = file_class->get_field("path", "Ljava/lang/String;");
}

//private native String getDriveDirectory(int drive);
static void getDriveDirectory(Frame *f) {
    unimplemented
}

//private native String canonicalize0(String path) throws IOException;
static void canonicalize0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto _path = slot::get<jref>(args);
    auto path = java_lang_String::to_utf8(_path);

    DWORD size = GetFullPathNameA(path, 0, nullptr, nullptr);
    if (size == 0) {
        throw java_io_IOException("Failed to get full path size.");
    }
    std::string full_path(size, '\0');
    if (GetFullPathNameA(path, size, &full_path[0], nullptr) == 0) {
        throw java_io_IOException("Failed to get full path.");
    }

    size = GetLongPathNameA(full_path.c_str(), nullptr, 0);
    if (size == 0) {
        throw java_io_IOException("Failed to get long path size.");
    }

    std::string long_path(size, '\0');
    if (GetLongPathNameA(full_path.c_str(), &long_path[0], size) == 0) {
        throw java_io_IOException("Failed to get long path.");
    }

//    long_path.resize(size - 1); // 去除末尾的空字符
    jref o = Allocator::string(long_path.c_str());
    f->pushr(o);
}

//private native String getFinalPath0(String path) throws IOException;
static void getFinalPath0(Frame *f) {
    unimplemented
}

//private native int getBooleanAttributes0(File f);
static void getBooleanAttributes0(Frame *f) {
    unimplemented
}

//private native boolean checkAccess0(File f, int access);
static void checkAccess0(Frame *f) {
    unimplemented
}

//private native long getLastModifiedTime0(File f);
static void getLastModifiedTime0(Frame *f) {
    unimplemented
}

//private native long getLength0(File f);
static void getLength0(Frame *f) {
    unimplemented
}

//private native boolean setPermission0(File f, int access, boolean enable, boolean owneronly);
static void setPermission0(Frame *f) {
    unimplemented
}

//private native boolean createFileExclusively0(String path) throws IOException;
static void createFileExclusively0(Frame *f) {
    unimplemented
}

//private native String[] list0(File f);
static void list0(Frame *f) {
    unimplemented
}

//private native boolean createDirectory0(File f);
static void createDirectory0(Frame *f) {
    unimplemented
}

//private native boolean setLastModifiedTime0(File f, long time);
static void setLastModifiedTime0(Frame *f) {
    unimplemented
}

//private native boolean setReadOnly0(File f);
static void setReadOnly0(Frame *f) {
    unimplemented
}

//private native boolean delete0(File f);
static void delete0(Frame *f) {
    unimplemented
}

//private native boolean rename0(File f1, File f2);
static void rename0(Frame *f) {
    unimplemented
}

//private static native int listRoots0();
static void listRoots0(Frame *f) {
    unimplemented
}

//private native long getSpace0(File f, int t);
static void getSpace0(Frame *f) {
    unimplemented
}

// Obtain maximum file component length from GetVolumeInformation which
// expects the path to be null or a root component ending in a backslash
//private native int getNameMax0(String path);
static void getNameMax0(Frame *f) {
    unimplemented
}

void java_io_WinNTFileSystem_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/WinNTFileSystem", #method, method_descriptor, method)

//    R(initIDs, "()V");
//    R(getDriveDirectory, "");
    R(canonicalize0, "(Ljava/lang/String;)Ljava/lang/String;");
//    R(getFinalPath0, "");
//    R(getBooleanAttributes0, "(Ljava/io/File;)I");
//    R(checkAccess0, "");
//    R(getLastModifiedTime0, "");
//    R(getLength0, "(Ljava/io/File;)J");
//    R(setPermission0, "");
//    R(createFileExclusively0, "");
//    R(list0, "");
//    R(createDirectory0, "");
//    R(setLastModifiedTime0, "");
//    R(setReadOnly0, "");
//    R(delete0, "");
//    R(rename0, "");
//    R(listRoots0, "");
//    R(getSpace0, "");
//    R(getNameMax0, "");
}
#endif
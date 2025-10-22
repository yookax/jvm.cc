#ifdef _WIN64
module;
#include <assert.h>
#include "../../../vmdef.h"
#include <windows.h>

module native;

//import std;
import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

// java/io/FileSystem.java
/* Constants for simple boolean attributes */
static const int BA_EXISTS    = 0x01;
static const int BA_REGULAR   = 0x02;
static const int BA_DIRECTORY = 0x04;
static const int BA_HIDDEN    = 0x08;

/* Check whether or not the file name in "path" is a Windows reserved
   device name (CON, PRN, AUX, NUL, COM[1-9], LPT[1-9]) based on the
   returned result from GetFullPathName, which should be in thr form of
   "\\.\[ReservedDeviceName]" if the path represents a reserved device
   name.
   Note1: GetFullPathName doesn't think "CLOCK$" (which is no longer
   important anyway) is a device name, so we don't check it here.
   GetFileAttributesEx will catch it later by returning 0 on NT/XP/
   200X.

   Note2: Theoretically the implementation could just lookup the table
   below linearly if the first 4 characters of the fullpath returned
   from GetFullPathName are "\\.\". The current implementation should
   achieve the same result. If Microsoft add more names into their
   reserved device name repository in the future, which probably will
   never happen, we will need to revisit the lookup implementation.

static WCHAR* ReservedDeviceNames[] = {
    L"CON", L"PRN", L"AUX", L"NUL",
    L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
    L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9",
    L"CLOCK$"
};
*/
static bool is_reserved_device_name(char* path) {
#define BUF_SIZE 9
    char buf[BUF_SIZE];
    char *lpf = nullptr;
    DWORD len = GetFullPathNameA(path,
                                    BUF_SIZE,
                                    buf,
                                    &lpf);
    if ((len == BUF_SIZE - 1 || len == BUF_SIZE - 2) &&
        buf[0] == L'\\' && buf[1] == L'\\' &&
        buf[2] == L'.' && buf[3] == L'\\') {
        char* name = strupr(buf + 4);
        if (strcmp(name, "CON") == 0 || strcmp(name, "PRN") == 0
            || strcmp(name, "AUX") == 0 || strcmp(name, "NUL") == 0)
            return true;
        if ((strncmp(name, "COM", 3) == 0 || strncmp(name, "LPT", 3) == 0)
                && name[3] - '0' > 0 && name[3] - '0' <= 9)
            return true;
    }
    return false;
}

static Field *path_field;

char *file_to_path(jref fo) {
    auto po = fo->get_field_value<jref>(path_field);
    return java_lang_String::to_utf8(po);
}

//private static native void initIDs();
static void initIDs(Frame *f) {
    Class *file_class = load_boot_class("java/io/File");
    // private final String path;
    path_field = file_class->get_field("path", "Ljava/lang/String;");
    assert(path_field != nullptr);
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
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto po = slot::get<jref>(args);
    auto path = java_lang_String::to_utf8(po);
    // todo
    f->pushr(po);
}

//private native int getBooleanAttributes0(File f);
static void getBooleanAttributes0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto fo = slot::get<jref>(args);

    char *path = file_to_path(fo);
    if (is_reserved_device_name(path)) {
        f->pushi(0);
        return;
    }

    auto a = GetFileAttributes(path);
    if (a == INVALID_FILE_ATTRIBUTES) {
        f->pushi(0);
        return;
    }

    jint attributes = BA_EXISTS;
    if ((a & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        attributes |= BA_DIRECTORY;
    } else {
        attributes |= BA_REGULAR;
    }
    if ((a & FILE_ATTRIBUTE_HIDDEN) != 0) {
        attributes |= BA_HIDDEN;
    }

    f->pushi(attributes);
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
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto fo = slot::get<jref>(args);

    std::filesystem::path file_path = file_to_path(fo);
    uintmax_t size = std::filesystem::file_size(file_path);
    f->pushl(size);
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

    R(initIDs, "()V");
//    R(getDriveDirectory, "");
    R(canonicalize0, "(Ljava/lang/String;)Ljava/lang/String;");
    R(getFinalPath0, "(Ljava/lang/String;)Ljava/lang/String;");
    R(getBooleanAttributes0, "(Ljava/io/File;)I");
//    R(checkAccess0, "");
//    R(getLastModifiedTime0, "");
    R(getLength0, "(Ljava/io/File;)J");
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
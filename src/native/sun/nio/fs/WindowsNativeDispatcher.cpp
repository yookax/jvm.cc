#ifdef _WIN64
module;
#include "../../../../vmdef.h"
#include <windows.h>

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

//private static native void GetFileAttributesEx0(long lpFileName, long address) throws WindowsException;
void GetFileAttributesEx0(Frame *f) {
    slot_t *args = f->lvars;
    auto file_name = (LPCWSTR) slot::get<jlong>(args);
    args += 2;
    auto file_attr_data = (LPWIN32_FILE_ATTRIBUTE_DATA) slot::get<jlong>(args);

    BOOL res = GetFileAttributesExW(file_name, GetFileExInfoStandard, (LPVOID) file_attr_data);
    if (res == 0)
        throw sun_nio_fs_WindowsException(std::to_string(GetLastError()));
}

void sun_nio_fs_WindowsNativeDispatcher_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("sun/nio/fs/WindowsNativeDispatcher", #method, method_descriptor, method)

    R(GetFileAttributesEx0, "(JJ)V");
}
#endif
module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

//private native String getDriveDirectory(int drive);

//private native String canonicalize0(String path) throws IOException;

//private native String getFinalPath0(String path) throws IOException;
//private native int getBooleanAttributes0(File f);
//private native boolean checkAccess0(File f, int access);
//private native long getLastModifiedTime0(File f);
//private native long getLength0(File f);
//private native boolean setPermission0(File f, int access, boolean enable, boolean owneronly);
//private native boolean createFileExclusively0(String path) throws IOException;


//private native String[] list0(File f);


//private native boolean createDirectory0(File f);


//private native boolean setLastModifiedTime0(File f, long time);


//private native boolean setReadOnly0(File f);

//private native boolean delete0(File f);

//private native boolean rename0(File f1, File f2);

//private static native int listRoots0();

//private native long getSpace0(File f, int t);

// Obtain maximum file component length from GetVolumeInformation which
// expects the path to be null or a root component ending in a backslash
//private native int getNameMax0(String path);

//private static native void initIDs();


void java_io_WinNTFileSystem_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/io/WinNTFileSystem", #method, method_descriptor, method)

}
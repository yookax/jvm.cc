module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;

//------------------------  CDS: Class Data Sharing

//private static native int getCDSConfigStatus();
void getCDSConfigStatus(Frame *f) {
    unimplemented
}

//private static native void logLambdaFormInvoker(String line);
void logLambdaFormInvoker(Frame *f) {
    unimplemented
}

/**
 * Initialize archived static fields in the given Class using archived
 * values from CDS dump time. Also initialize the classes of objects in
 * the archived graph referenced by those fields.
 *
 * Those static fields remain as uninitialized if there is no mapped CDS
 * java heap data or there is any error during initialization of the
 * object class in the archived graph.
 */
//public static native void initializeFromArchive(Class<?> c);
void initializeFromArchive(Frame *f) {
    // todo
}

/**
 * Ensure that the native representation of all archived java.lang.Module objects
 * are properly restored.
 */
//public static native void defineArchivedModules(ClassLoader platformLoader, ClassLoader systemLoader);
void defineArchivedModules(Frame *f) {
    unimplemented
}

/**
 * Returns a predictable "random" seed derived from the VM's build ID and version,
 * to be used by java.util.ImmutableCollections to ensure that archived
 * ImmutableCollections are always sorted the same order for the same VM build.
 */
//public static native long getRandomSeedForDumping();
void getRandomSeedForDumping(Frame *f) {
    f->pushl(static_cast<unsigned int>(std::time(nullptr)));
}

//private static native void dumpClassList(String listFileName);
void dumpClassList(Frame *f) {
    unimplemented
}

//private static native void dumpDynamicArchive(String archiveFileName);
void dumpDynamicArchive(Frame *f) {
    unimplemented
}


void jdk_internal_misc_CDS_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/CDS", #method, method_descriptor, method)

    R(getCDSConfigStatus, "()I");
    R(logLambdaFormInvoker, "(Ljava/lang/String;)V");
    R(initializeFromArchive, "(Ljava/lang/Class;)V");
    R(defineArchivedModules, "(Ljava/lang/ClassLoader;Ljava/lang/ClassLoader;)V");
    R(getRandomSeedForDumping, "()J");
    R(dumpClassList, "(Ljava/lang/String;)V");
    R(dumpDynamicArchive, "(Ljava/lang/String;)V");
}
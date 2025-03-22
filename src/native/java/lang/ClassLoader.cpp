module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;

//static native Class<?> defineClass1(ClassLoader loader, String name,
//                       byte[] b, int off, int len, ProtectionDomain pd, String source);
void defineClass1(Frame *f) {

}

//static native Class<?> defineClass2(ClassLoader loader, String name,
//           java.nio.ByteBuffer b, int off, int len, ProtectionDomain pd, String source);
void defineClass2(Frame *f) {

}

/*
* Defines a class of the given flags via Lookup.defineClass.
*
* @param loader the defining loader
* @param lookup nest host of the Class to be defined
* @param name the binary name or {@code null} if not findable
* @param b class bytes
* @param off the start offset in {@code b} of the class bytes
* @param len the length of the class bytes
* @param pd protection domain
* @param initialize initialize the class
* @param flags flags
* @param classData class data
*/
//static native Class<?> defineClass0(ClassLoader loader, Class<?> lookup,
//                  String name, byte[] b, int off, int len, ProtectionDomain pd,
//                  boolean initialize, int flags, Object classData);
void defineClass0(Frame *f) {

}

// return null if not found
//private static native Class<?> findBootstrapClass(String name);
void findBootstrapClass(Frame *f) {

}

//private final native Class<?> findLoadedClass0(String name);
void findLoadedClass0(Frame *f) {

}

// Retrieves the assertion directives from the VM.
// private static native AssertionStatusDirectives retrieveDirectives();
void retrieveDirectives(Frame *f) {
    unimplemented
}

#define OBJ "Ljava/lang/Object;"
#define STR "Ljava/lang/String;"
#define CLS "Ljava/lang/Class;"
#define CL "Ljava/lang/ClassLoader;"

//private static native void registerNatives();
void java_lang_ClassLoader_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/ClassLoader", #method, method_descriptor, method)

//    R(defineClass0, CL CLS STR "[BIILjava/security/ProtectionDomain;ZI" OBJ);
//    R(defineClass1, "(" STR "[BIILjava/security/ProtectionDomain;" STR ")" CLS);
//    R(defineClass2, CL STR "Ljava/nio/ByteBuffer;IILjava/security/ProtectionDomain;" STR);
//    R(findBootstrapClass, "(Ljava/lang/String;)" CLS);
//    R(findLoadedClass0, "(Ljava/lang/String;)" CLS);
//    R(retrieveDirectives, "()Ljava/lang/AssertionStatusDirectives;");
}
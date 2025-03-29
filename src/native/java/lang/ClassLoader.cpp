module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;
import encoding;

//static native Class<?> defineClass1(ClassLoader loader, String name,
//                       byte[] b, int off, int len, ProtectionDomain pd, String source);
void defineClass1(Frame *f) {
    slot_t *args = f->lvars;
    auto loader = slot::get<jref>(args++);
    auto name = slot::get<jref>(args++);
    auto class_bytes = slot::get<jref>(args++);
    auto off = slot::get<jint>(args++);
    auto len = slot::get<jint>(args++);
    auto pd = slot::get<jref>(args++);
    auto source = slot::get<jref>(args);

    if (class_bytes == nullptr)
        throw java_lang_NullPointerException();
    if (len < 0)
        throw java_lang_ArrayIndexOutOfBoundsException();

    char *class_name = nullptr;
    if (name != nullptr) {
        class_name = utf8::dot_2_slash_dup(java_lang_String::to_utf8(name));
    }

    Class *c = define_class(loader, class_name,
                            (jbyte *) class_bytes->index(off), len,
                            pd, java_lang_String::to_utf8(source));
    f->pushr(c->java_mirror);
}

//static native Class<?> defineClass2(ClassLoader loader, String name,
//           java.nio.ByteBuffer b, int off, int len, ProtectionDomain pd, String source);
void defineClass2(Frame *f) {
    unimplemented
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
* @param classData private static pre-initialized field; may be null
*/
//static native Class<?> defineClass0(ClassLoader loader, Class<?> lookup,
//                  String name, byte[] b, int off, int len, ProtectionDomain pd,
//                  boolean initialize, int flags, Object classData);
void defineClass0(Frame *f) {
    slot_t *args = f->lvars;
    auto loader = slot::get<jref>(args++);
    auto lookup = slot::get<jref>(args++)->jvm_mirror;
    auto name = slot::get<jref>(args++);
    auto class_bytes = slot::get<jref>(args++);
    auto off = slot::get<jint>(args++);
    auto len = slot::get<jint>(args++);
    auto pd = slot::get<jref>(args++);
    auto initialize = slot::get<jbool>(args++);
    auto flags = slot::get<jint>(args++);
    auto class_data = slot::get<jref>(args);

    if (class_bytes == nullptr)
        throw java_lang_NullPointerException();
    if (len < 0)
        throw java_lang_ArrayIndexOutOfBoundsException();

    char *class_name = nullptr;
    if (name != nullptr) {
        class_name = utf8::dot_2_slash_dup(java_lang_String::to_utf8(name));
    }

    Class *c = define_class(lookup, class_name,
                            (jbyte *) class_bytes->index(off), len,
                            pd, initialize, flags, class_data);

    // From java/lang/invoke/MethodHandleNatives.java
    // Flags for Lookup.ClassOptions：
    static const int
            NESTMATE_CLASS        = 0x00000001,
            HIDDEN_CLASS          = 0x00000002,
            STRONG_LOADER_LINK    = 0x00000004,
            ACCESS_VM_ANNOTATIONS = 0x00000008;

    if ((flags & NESTMATE_CLASS) != 0) {
        c->set_nest_host(lookup->get_nest_host());
    }
    if ((flags & HIDDEN_CLASS) != 0) {
        c->hidden = true;
    }
    if ((flags & STRONG_LOADER_LINK) != 0) {
        // unimplemented
    }
    if ((flags & ACCESS_VM_ANNOTATIONS) != 0) {
        // unimplemented
    }

    f->pushr(c->java_mirror);
}

// return null if not found
//private static native Class<?> findBootstrapClass(String name);
void findBootstrapClass(Frame *f) {
    slot_t *args = f->lvars;
    auto name = slot::get<jref>(args);

    if (name == nullptr) {
        f->pushr(nullptr);
        return;
    }
    utf8_t *slash_name = utf8::dot_2_slash_dup(java_lang_String::to_utf8(name));
    Class *c = load_boot_class(slash_name);
    f->pushr(c != nullptr ? c->java_mirror : nullptr);
}

//private final native Class<?> findLoadedClass0(String name);
void findLoadedClass0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto name = slot::get<jref>(args);

    if (name == nullptr) {
        f->pushr(nullptr);
        return;
    }
    utf8_t *slash_name = utf8::dot_2_slash_dup(java_lang_String::to_utf8(name));
    Class *c = find_loaded_class(_this, slash_name);
    f->pushr(c != nullptr ? c->java_mirror : nullptr);
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

    R(defineClass0, "(Ljava/lang/ClassLoader;Ljava/lang/Class;Ljava/lang/String;[BIILjava/security/ProtectionDomain;ZILjava/lang/Object;)Ljava/lang/Class;");
    R(defineClass1, "(Ljava/lang/ClassLoader;Ljava/lang/String;[BIILjava/security/ProtectionDomain;Ljava/lang/String;)Ljava/lang/Class;");
    R(defineClass2, CL STR "Ljava/nio/ByteBuffer;IILjava/security/ProtectionDomain;" STR);
    R(findBootstrapClass, "(Ljava/lang/String;)" CLS);
    R(findLoadedClass0, "(Ljava/lang/String;)" CLS);
    R(retrieveDirectives, "()Ljava/lang/AssertionStatusDirectives;");
}
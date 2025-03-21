module;
#include <cassert>
#include "../../../vmdef.h"

module native;

import object;
import encoding;
import runtime;

using namespace std;
using namespace utf8;
using namespace java_lang_String;

/*
 * Cache the name to reduce the number of calls into the VM.
 * This field would be set by VM itself during initClassName call.
 *
 * private transient String name;
 * private native String initClassName();
 */
void initClassName(Frame *f) {
    Class *c = f->method->clazz;
    jstrRef class_name = Allocator::string(slash_2_dot_dup(c->name));
    class_name = intern(class_name);
    c->java_mirror->set_field_value<jref>("name", "Ljava/lang/String;", class_name);
    f->pushr(class_name);
}

// private native Class<?>[] getInterfaces0();
void getInterfaces0(Frame *f) {
    Class *c = f->method->clazz;
    auto size = (jint) c->interfaces.size();
    jarrRef interfaces = Allocator::class_array(size);
    for (int i = 0; i < size; i++) {
        assert(c->interfaces[i] != nullptr);
        interfaces->setRefElt(i, c->interfaces[i]->java_mirror);
    }

    f->pushr(interfaces);
}

// public native boolean isInterface();
void isInterface(Frame *f) {
    bool b = f->method->clazz->access_flags.is_interface();
    f->pushi(b ? 1 : 0);
}

// public native boolean isArray();
void isArray(Frame *f) {
    bool b = f->method->clazz->is_array_class();
    f->pushi(b ? 1 : 0);
}

// public native boolean isHidden();
void isHidden(Frame *f) {
    bool b = f->method->clazz->hidden;
    f->pushi(b ? 1 : 0);
}

// public native boolean isPrimitive();
void isPrimitive(Frame *f) {
    bool b = f->method->clazz->is_prim_class();
    f->pushi(b ? 1 : 0);
}

#define OBJ "Ljava/lang/Object;"
#define CLS "Ljava/lang/Class;"
#define CPL "Ljdk/internal/reflect/ConstantPool;"
#define STR "Ljava/lang/String;"
#define FLD "Ljava/lang/reflect/Field;"
#define MHD "Ljava/lang/reflect/Method;"
#define CTR "Ljava/lang/reflect/Constructor;"
#define PD  "Ljava/security/ProtectionDomain;"
#define BA  "[B"
#define RC  "Ljava/lang/reflect/RecordComponent;"


// private static native void registerNatives();
void java_lang_Class_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Class", #method, method_descriptor, method)

    R(initClassName, "()" STR);
    R(getInterfaces0, "()[" CLS);
    R(isInterface, "()Z");
    R(isArray, "()Z");
    R(isHidden, "()Z");
    R(isPrimitive, "()Z");
}
module;
#include "../vmdef.h"

module invoke;

import slot;
import runtime;
import object;
import classfile;
import encoding;
import class_loader;
import interpreter;

using namespace std;
using namespace slot;
using namespace utf8;

static Class *constructor_reflect_class; // java.lang.reflect.Constructor
static Class *method_reflect_class;      // java.lang.reflect.Method
static Class *field_reflect_class;       // java.lang.reflect.Field
static Class *MT_class;                  // java.lang.invoke.MethodType
static Class *MH_class;                  // java.lang.invoke.MethodHandle
static Class *MHN_class;                 // java.lang.invoke.MethodHandleNatives
static Class *MN_class;                  // java.lang.invoke.MemberName
static Class *RMN_class;                 // java.lang.invoke.ResolvedMethodName

static int MH_form_id; // final LambdaForm form;

// java.lang.invoke.LambdaForm
// static Field *LF_vmentry_field; // MemberName vmentry; // low-level behavior, or null if not yet prepared

static int MN_clazz_id;   // private Class<?> clazz;
static int MN_name_id;    // private String name;
static int MN_type_id;    // private Object type;
static int MN_flags_id;   // private int flags;
static int MN_method_id;  // private ResolvedMethodName method; // cached resolved method information
static int MN_vmindex_id; // @Injected intptr_t vmindex; // vtable index or offset of resolved member

// final class ResolvedMethodName {
//     // @Injected JVM_Method* vmtarget;
//     // @Injected Class<?>    vmholder;
// };
static int RMN_vmtarget_id;
static int RMN_vmholder_id;

//void init_MN();

#define MH_asType_method_desc "asType", "(Ljava/lang/invoke/MethodType;)Ljava/lang/invoke/MethodHandle;"

void init_invoke() {
    constructor_reflect_class = load_boot_class("java/lang/reflect/Constructor");
    method_reflect_class = load_boot_class("java/lang/reflect/Method");
    field_reflect_class = load_boot_class("java/lang/reflect/Field");
    MT_class = load_boot_class("java/lang/invoke/MethodType");
    MH_class = load_boot_class("java/lang/invoke/MethodHandle");
    MHN_class = load_boot_class("java/lang/invoke/MethodHandleNatives");
    MN_class = load_boot_class("java/lang/invoke/MemberName");
    RMN_class = load_boot_class("java/lang/invoke/ResolvedMethodName");

    MH_form_id = MH_class->get_field("form", "Ljava/lang/invoke/LambdaForm;")->id;

    MN_clazz_id = MN_class->get_field("clazz", "Ljava/lang/Class;")->id;
    MN_name_id = MN_class->get_field("name", "Ljava/lang/String;")->id;
    // type maybe a String or an Object[] or a MethodType
    // Object[]: (Class<?>) Object[0] is return type
    //           (Class<?>[]) Object[1] is parameter types
    MN_type_id = MN_class->get_field("type", "Ljava/lang/Object;")->id;
    MN_flags_id = MN_class->get_field("flags", "I")->id;
    MN_method_id = MN_class->get_field("method", "Ljava/lang/invoke/ResolvedMethodName;")->id;
    MN_vmindex_id = MN_class->get_field("vmindex", "I")->id;

    RMN_vmtarget_id = RMN_class->get_field("vmtarget", "Ljava/lang/Object;")->id;
    RMN_vmholder_id = RMN_class->get_field("vmholder", "Ljava/lang/Class;")->id;

//    init_MN();
}

jref findMethodType(jarrRef ptypes, jclsRef rtype) {
    assert(ptypes != nullptr && rtype != nullptr);

    // static MethodType findMethodHandleType(Class<?> rtype, Class<?>[] ptypes)
    Method *m = MHN_class->get_method("findMethodHandleType",
                "(Ljava/lang/Class;[Ljava/lang/Class;)Ljava/lang/invoke/MethodType;");
    return execJavaR(m, { rslot(rtype), rslot(ptypes) });
}

jref findMethodType(const utf8_t *desc, jref loader) {
    assert(desc != nullptr);

    // public static MethodType fromMethodDescriptorString(String descriptor, ClassLoader loader)
    Method *m = MT_class->get_method("fromMethodDescriptorString",
                "(Ljava/lang/String;Ljava/lang/ClassLoader;)Ljava/lang/invoke/MethodType;");
    return execJavaR(m, { rslot(Allocator::string(desc)), rslot(loader) });

    // pair<jarrRef, jclsRef> p = parseMethodDescriptor(desc, loader);
    // return findMethodType(p.first, p.second);
}

jref linkMethodHandleConstant(Class *caller_class, int ref_kind,
                              Class *defining_class, const char *name, Object *type) {
    jref name_str = Allocator::string(name);
    // static MethodHandle linkMethodHandleConstant(Class<?> callerClass, int refKind,
    //                                                 Class<?> defc, String name, Object type)
    Method *m = MHN_class->get_method("linkMethodHandleConstant",
        "(Ljava/lang/Class;ILjava/lang/Class;Ljava/lang/String;Ljava/lang/Object;)"
                    "Ljava/lang/invoke/MethodHandle;");
    return execJavaR(m,
                     { rslot(caller_class->java_mirror), islot(ref_kind),
                        rslot(defining_class->java_mirror), rslot(name_str), rslot(type) });
}
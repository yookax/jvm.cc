﻿module;
#include "../../../../vmdef.h"

module native;

import slot;
import object;
import classfile;
import constants;
import encoding;
import exception;
import runtime;
import class_loader;
import interpreter;

using namespace slot;
using namespace utf8;
using namespace java_lang_String;


/* Method flags */

#define MB_LAMBDA_HIDDEN        1
#define MB_LAMBDA_COMPILED      2
#define MB_CALLER_SENSITIVE     4
#define MB_DEFAULT_CONFLICT     8


#define IS_METHOD        0x010000
#define IS_CONSTRUCTOR   0x020000
#define IS_FIELD         0x040000
#define IS_TYPE          0x080000
#define CALLER_SENSITIVE 0x100000

#define SEARCH_SUPERCLASSES 0x100000
#define SEARCH_INTERFACES   0x200000

#define ALL_KINDS (IS_METHOD | IS_CONSTRUCTOR | IS_FIELD | IS_TYPE)

#define REFERENCE_KIND_SHIFT 24
#define REFERENCE_KIND_MASK  (0xf000000 >> REFERENCE_KIND_SHIFT)

static int methodFlags(Method *m) {
    int flags = m->access_flags.get();

    if(flags & MB_CALLER_SENSITIVE)
        flags |= CALLER_SENSITIVE;

    return flags;
}

static Class *constructor_reflect_class;
static Class *method_reflect_class;
static Class *field_reflect_class;
static Class *MT_class; // java/lang/invoke/MethodType
static Class *MH_class; // java/lang/invoke/MethodHandle

static Class *MN_class;  // java/lang/invoke/MemberName
static Field *MN_clazz_field;
static Field *MN_name_field;
static Field *MN_type_field;
static Field *MN_flags_field;
static Field *MN_method_field;
static Field *MN_vmindex_field;

// in java\lang\invoke\MemberName.java
// final class ResolvedMethodName {
//     //@Injected JVM_Method* vmtarget;
//     //@Injected Class<?>    vmholder;
// };
static Class *RMN_class; // java/lang/invoke/ResolvedMethodName
static Field *RMN_vmtarget_field;
static Field *RMN_vmholder_field;

static Class *CS_class;  // java/lang/invoke/CallSite
static Field *CS_target_field;

static void __init__() {
    constructor_reflect_class = load_boot_class("java/lang/reflect/Constructor");
    method_reflect_class = load_boot_class("java/lang/reflect/Method");
    field_reflect_class = load_boot_class("java/lang/reflect/Field");
    MN_class = load_boot_class("java/lang/invoke/MemberName");
    RMN_class = load_boot_class("java/lang/invoke/ResolvedMethodName");
    MH_class = load_boot_class("java/lang/invoke/MethodHandle");
    MT_class = load_boot_class("java/lang/invoke/MethodType");
    CS_class = load_boot_class("java/lang/invoke/CallSite");

    // private Class<?> clazz;
    MN_clazz_field = MN_class->get_field("clazz", "Ljava/lang/Class;");
    // private String name;
    MN_name_field = MN_class->get_field("name", "Ljava/lang/String;");
    // type maybe a String or an Object[] or a MethodType
    // Object[]: (Class<?>) Object[0] is return type
    //           (Class<?>[]) Object[1] is parameter types
    // private Object type;
    MN_type_field = MN_class->get_field("type", "Ljava/lang/Object;");
    // private int flags;
    MN_flags_field = MN_class->get_field("flags", "I");
    // private ResolvedMethodName method;    // cached resolved method information
    MN_method_field = MN_class->get_field("method", "Ljava/lang/invoke/ResolvedMethodName;");
    //@Injected intptr_t vmindex;   // vtable index or offset of resolved member
    MN_vmindex_field = MN_class->get_field("vmindex", "I");
    // public String getSignature();
    // MN_getSignature_method = MN_class->getMethod("getSignature", S(___java_lang_String));

    RMN_vmtarget_field = RMN_class->get_field("vmtarget", "Ljava/lang/Object;");
    RMN_vmholder_field = RMN_class->get_field("vmholder", "Ljava/lang/Class;");

    // The actual payload of this call site.
    // Can be modified using {@link MethodHandleNatives#setCallSiteTargetNormal} or {@link MethodHandleNatives#setCallSiteTargetVolatile}.
    /*package-private*/
    // final MethodHandle target;  // Note: This field is known to the JVM.
    CS_target_field = CS_class->get_field("target", "Ljava/lang/invoke/MethodHandle;");
}

static jref new_resolved_method_name(jref vmtarget, jref vmholder) {
    jref resolved_method_name = Allocator::object(RMN_class);
    resolved_method_name->set_field_value<jref>(RMN_vmtarget_field, vmtarget);
    resolved_method_name->set_field_value<jref>(RMN_vmholder_field, vmholder);
    return resolved_method_name;
}

// 这些native函数全部定义在 java\lang\invoke\MethodHandleNatives.java

// static native void init(MemberName self, Object ref);
static void init(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args++);
    auto target = slot::get<jref>(args);

    // todo 要不要判断一下resolved_method_name是否为空
    // jref resolved_method_name = get_ref_field0(member_name, MN_method_field);

    /*
     * in fact, `target` will be one of these three:
     * 1. java/lang/reflect/Field.
     * 2. java/lang/reflect/Constructor.
     * 3. java/lang/reflect/Method.
     */

    if (target->clazz == method_reflect_class) {
        // fill in vmtarget, vmindex while we have method in hand:

        Method *m = get_method_from_reflect_object(target);
        jref resolved = new_resolved_method_name((jref) (void *) m, nullptr);

        self->set_field_value<jref>(MN_clazz_field, m->clazz->java_mirror);
        self->set_field_value<jref>(MN_method_field, resolved);
        return;
    }

    if (target->clazz == constructor_reflect_class) {
        unimplemented
        return;
    }

    if (target->clazz == field_reflect_class) {
        // fill in vmtarget, vmindex while we have field in hand:

        // Field *f = get_field_from_reflect_object(target);

        unimplemented
        return;
    }

    assert(self != nullptr && target != nullptr);
    UNREACHABLE("Wrong target: %s\n", target->clazz->name);
}

// static native void expand(MemberName self);
static void expand(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args);

    jref resolved = self->get_field_value<jref>(MN_method_field);
    if (resolved == nullptr) {
        panic("java_lang_IllegalArgumentException");   // todo
    }

    auto vmtarget = (Method *) resolved->get_field_value<jref>(RMN_vmtarget_field);
    jclsRef vmholder = resolved->get_field_value<jref>(RMN_vmholder_field);
    assert(vmtarget != nullptr && vmholder != nullptr);

    jstrRef name = self->get_field_value<jref>(MN_name_field);
    jref type = self->get_field_value<jref>(MN_type_field);
    jint flags = self->get_field_value<jint>(MN_flags_field);

    switch (flags & ALL_KINDS) {
        case IS_METHOD:
        case IS_CONSTRUCTOR:
            if (name == nullptr) {
                self->set_field_value<jref>(MN_name_field, intern(Allocator::string(vmtarget->name)));
            }
            if (type == nullptr) {
                // todo type的类型？
                self->set_field_value<jref>(MN_type_field, intern(Allocator::string(vmtarget->descriptor)));
            }
            break;
        case IS_FIELD:
        unimplemented
        default:
            //  signalException(java_lang_InternalError, "flags kind");
            panic("java_lang_InternalError");   // todo
    }
}

/*
 * static native MemberName resolve(MemberName self, Class<?> caller, int lookupMode,
            boolean speculativeResolve) throws LinkageError, ClassNotFoundException;
 */
static void resolve(Frame *frame) {
    slot_t *args = frame->lvars;
    auto self = slot::get<jref>(args++); /*MemberName*/
    auto caller = slot::get<jref>(args++);
    auto lookupMode = slot::get<jint>(args++);
    auto speculativeResolve = slot::get<jbool>(args);

    // todo speculative_resolve
    // return java_lang_invoke_MemberName::resolve(self, caller != nullptr ? caller->jvm_mirror : nullptr);
    assert(self != nullptr);

    jstrRef name_str = self->get_field_value<jref>(MN_name_field);
    Class *clazz = self->get_field_value<jref>(MN_clazz_field)->jvm_mirror;
    jref type = self->get_field_value<jref>(MN_type_field);
    jint flags = self->get_field_value<jint>(MN_flags_field);

    if(clazz == nullptr || name_str == nullptr || type == nullptr) {
        UNREACHABLE(" "); // todo msg
    }

    //const utf8_t *name = utf8_pool::save(java_lang_String::to_utf8(name_str));
    const utf8_t *name = java_lang_String::to_utf8(name_str);
    if (strcmp(name, "<clinit>") == 0) {
        panic("11111111111"); // todo
    }

    switch(flags & ALL_KINDS) {
        case IS_METHOD: {
            // java.lang.invoke.MethodType
            // public String toMethodDescriptorString()
            Method *d = MT_class->lookup_method("toMethodDescriptorString", "()Ljava/lang/String;");
            jstrRef sig_str = execJavaR(d, { rslot(type) });
            const utf8_t *sig = java_lang_String::to_utf8(sig_str);

            Method *m = clazz->lookup_method(name, sig);
            if (m == nullptr) {
                m = clazz->generate_poly_method(name, sig);
            }
            if (m == nullptr) {
                frame->pushr(nullptr); // todo
                return;
                // raise_exception0(S(java_lang_NoSuchMethodError), "%s~%s~%s", clazz->class_name, name, sig);  // todo msg
            }

            flags |= methodFlags(m);
            self->set_field_value<jint>(MN_flags_field, flags);
            // printf("--- %p, %s, %s, %s\n", member_name, m->clazz->name, m->name, m->descriptor); //////////////////////////////////////////////////////////////////////////////////////
            jref resolved = new_resolved_method_name((jref) (void *) m, nullptr); // todo RMN_vmholder_field怎么设置
            self->set_field_value<jref>(MN_method_field, resolved);
            frame->pushr(self);
            return;
        }
        case IS_CONSTRUCTOR: {
            // java.lang.invoke.MethodType
            // public String toMethodDescriptorString()
            Method *d = MT_class->lookup_method("toMethodDescriptorString", "()Ljava/lang/String;");
            jstrRef sig_str = execJavaR(d, { rslot(type) });
            const utf8_t *sig = java_lang_String::to_utf8(sig_str);

            Method *m = clazz->get_method(name, sig);
            if (m == nullptr) {
                // todo
                throw java_lang_NoSuchMethodError("resolve member name, CONSTRUCTOR");
            }

            flags |= methodFlags(m);
            self->set_field_value<jint>(MN_flags_field, flags);

            jref resolved = new_resolved_method_name((jref) (void *) m, nullptr); // todo RMN_vmholder_field怎么设置
            self->set_field_value<jref>(MN_method_field, resolved);
            frame->pushr(self);
            return;
        }
        case IS_FIELD: {
            Field *f = clazz->lookup_field(name);
            if (f == nullptr) {
                // todo
                throw java_lang_NoSuchFieldError("resolve member name, FIELD");
            }

            flags |= f->access_flags.get();
            self->set_field_value<jint>(MN_flags_field, flags);
            jref resolved = new_resolved_method_name((jref) (void *) f, nullptr); // todo RMN_vmholder_field怎么设置
            self->set_field_value<jref>(MN_method_field, resolved);
            frame->pushr(self);
            return;
        }
        default:
            UNREACHABLE("%d", flags & ALL_KINDS); // todo
            // throw java_lang_LinkageError("resolve member name");
            // raise_exception(S(java_lang_LinkageError), nullptr);
    }
}

// static native int getMembers(Class<?> defc, String matchName, String matchSig,
//                       int matchFlags, Class<?> caller, int skip, MemberName[] results);
static void getMembers(Frame *f) {
    slot_t *args = f->lvars;
    auto defc = slot::get<jref>(args++);
    auto match_name = slot::get<jref>(args++);
    auto match_sig = slot::get<jref>(args++);
    auto match_flags = slot::get<jint>(args++);
    auto caller = slot::get<jref>(args++);
    auto skip = slot::get<jint>(args++);
    auto _results = slot::get<jref>(args);

    auto results = (jarrRef)(_results);
    int search_super = (match_flags & SEARCH_SUPERCLASSES) != 0;
    int search_intf = (match_flags & SEARCH_INTERFACES) != 0;
    int local = !(search_super || search_intf);
//    char *name_sym = nullptr;
//    char *sig_sym = nullptr;

    if (match_name != nullptr) {
        // utf8_t *x = string_to_utf8(match_name);
        unimplemented
    }

    if (match_sig != nullptr) {
        // utf8_t *x = string_to_utf8(match_sig);
        unimplemented
    }

    if(match_flags & IS_FIELD)
    unimplemented

    if(!local)
    unimplemented

    if(match_flags & (IS_METHOD | IS_CONSTRUCTOR)) {
        int count = 0;

        for (Method *m: defc->jvm_mirror->methods) {
            if(strcmp(m->name, "<clinit>") == 0)
                continue;
            if(strcmp(m->name, "<init>") == 0)
                continue;
            if(skip-- > 0)
                continue;

            if(count < results->arr_len) {
                auto member_name = results->getElt<jref>(count);
                count++;
                int flags = methodFlags(m) | IS_METHOD;

                flags |= (m->access_flags.is_static() ? JVM_REF_invokeStatic : JVM_REF_invokeVirtual) << REFERENCE_KIND_SHIFT;

                member_name->set_field_value<jint>(MN_flags_field, flags);
                member_name->set_field_value<jref>(MN_clazz_field, m->clazz->java_mirror);
                member_name->set_field_value<jref>(MN_name_field, intern(Allocator::string(m->name)));
                member_name->set_field_value<jref>(MN_type_field, Allocator::string(m->descriptor));
            }
        }

        f->pushi(count);
        return;
    }

    unimplemented
}

// static native long objectFieldOffset(MemberName self);  // e.g., returns vmindex
static void objectFieldOffset(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args);

    // return java_lang_invoke_MemberName::objectFieldOffset(self);
    jref resolved = self->get_field_value<jref>(MN_method_field);
    assert(resolved != nullptr);
    auto field = (Field *)(void *) resolved->get_field_value<jref>(RMN_vmtarget_field);
    f->pushl(field_offset(field));
}

// static native long staticFieldOffset(MemberName self);  // e.g., returns vmindex
static void staticFieldOffset(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args);

    jref resolved = self->get_field_value<jref>(MN_method_field);
    assert(resolved != nullptr);
    auto field = (Field *)(void *) resolved->get_field_value<jref>(RMN_vmtarget_field);
    f->pushl(field_offset(field));
}

// static native Object staticFieldBase(MemberName self);  // e.g., returns clazz
static void staticFieldBase(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args);

    jref resolved = self->get_field_value<jref>(MN_method_field);
    assert(resolved != nullptr);
    auto field = (Field *)(void *) resolved->get_field_value<jref>(RMN_vmtarget_field);
    f->pushr(field->clazz->java_mirror);
}

// static native Object getMemberVMInfo(MemberName self);  // returns {vmindex,vmtarget}
static void getMemberVMInfo(Frame *f) {
    slot_t *args = f->lvars;
    auto self = slot::get<jref>(args);

    // return java_lang_invoke_MemberName::getMemberVMInfo(self);
    /*
     * return Object[2];
     *
     * 使用实例：
     *   Object vmInfo = MethodHandleNatives.getMemberVMInfo(this); // `this` is a MemberName
     *   assert(vmInfo instanceof Object[]);
     *   long vmindex = (Long) ((Object[])vmInfo)[0];
     *   Object vmtarget = ((Object[])vmInfo)[1];
     */

    // vmindex and vmtarget 都是注入的属性
    // 参考 class.cpp 的 Class::Class(jref class_loader, const u1 *bytecode, size_t len) 函数
    // inject fields 部分

    jint vmindex = self->get_field_value<jint>(MN_vmindex_field);
    Method *vmtarget = nullptr;

    jref resolved = self->get_field_value<jref>(MN_method_field);
    if (resolved != nullptr) {
        vmtarget = (Method *) resolved->get_field_value<jref>(RMN_vmtarget_field);
    }

    auto o = Allocator::object_array({long_box(vmindex), (jref) vmtarget });
    f->pushr(o);
}

// static native void setCallSiteTargetNormal(CallSite site, MethodHandle target);
static void setCallSiteTargetNormal(Frame *f) {
    slot_t *args = f->lvars;
    auto site = slot::get<jref>(args++);
    auto target = slot::get<jref>(args);

    if (!equals(site->clazz->name, "java/lang/invoke/CallSite")) {
        UNREACHABLE("%s\n", site->clazz->name);
    }
    if (!equals(target->clazz->name, "java/lang/invoke/MethodHandle")) {
        UNREACHABLE("%s\n", target->clazz->name);
    }

    unimplemented
}

// static native void setCallSiteTargetVolatile(CallSite site, MethodHandle target);
static void setCallSiteTargetVolatile(Frame *f) {
    slot_t *args = f->lvars;
    auto site = slot::get<jref>(args++);
    auto target = slot::get<jref>(args);

    if (!equals(site->clazz->name, "java/lang/invoke/CallSite")) {
        UNREACHABLE("%s\n", site->clazz->name);
    }
    if (!equals(target->clazz->name, "java/lang/invoke/MethodHandle")) {
        UNREACHABLE("%s\n", target->clazz->name);
    }

    unimplemented
}

// static native void copyOutBootstrapArguments(Class<?> caller, int[] indexInfo,
//                                              int start, int end,
//                                              Object[] buf, int pos,
//                                              boolean resolve,
//                                              Object ifNotAvailable);
static void copyOutBootstrapArguments(Frame *f) {
    unimplemented
}


// Invalidate all recorded nmethods.
// private static native void clearCallSiteContext(CallSiteContext context);
static void clearCallSiteContext(Frame *f) {
    unimplemented
}

#define OBJ   "Ljava/lang/Object;"
#define OBJ_  "Ljava/lang/Object;)"
#define CLS   "Ljava/lang/Class;"
#define _CLS  "(Ljava/lang/Class;"
#define STR   "Ljava/lang/String;"
#undef MN
#undef _MN_
#define MN "Ljava/lang/invoke/MemberName;"
#define _MN_ "(Ljava/lang/invoke/MemberName;)"
#define _CS "(Ljava/lang/invoke/CallSite;"
#define MH_ "Ljava/lang/invoke/MethodHandle;)"

void java_lang_invoke_MethodHandleNatives_registerNatives(Frame *f) {
    __init__();

#undef R
#define R(method, method_descriptor) \
    registry("java/lang/invoke/MethodHandleNatives", #method, method_descriptor, method)

    R(init, "(" MN OBJ_ "V");
    R(expand, _MN_ "V");
    R(resolve, "(" MN CLS "IZ)" MN);
    R(getMembers, _CLS STR STR "I" CLS "I[" MN ")I");

    // Field layout queries parallel to sun.misc.Unsafe:

    R(objectFieldOffset, _MN_ "J");
    R(staticFieldOffset, _MN_ "J");
    R(staticFieldBase, _MN_ OBJ);
    R(getMemberVMInfo, _MN_ OBJ);

    // CallSite support
    // Tell the JVM that we need to change the target of a CallSite.

    R(setCallSiteTargetNormal, _CS MH_ "V");
    R(setCallSiteTargetVolatile, _CS MH_ "V");
    R(copyOutBootstrapArguments, _CLS "III" OBJ "IZ" OBJ ")V");
    R(clearCallSiteContext, "(Ljava/lang/invoke/MethodHandleNatives$CallSiteContext)V");
}
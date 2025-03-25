module;
#include "../../../../vmdef.h"

module native;

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

void __init_invoke__() {
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

//Array *method_type::parameterTypes(jref methodType)
//{
//    assert(methodType != nullptr);
//
//    // private final Class<?>[] ptypes;
//    auto ptypes = methodType->getRefField<Array>("ptypes", "[Ljava/lang/Class;");
//    return ptypes;
//}

//jstrRef method_type::toMethodDescriptor(jref methodType)
//{
//    assert(methodType != nullptr);
//
//    Class *mt = loadBootClass("java/lang/invoke/MethodType");
//    // public String toMethodDescriptorString();
//    Method *m = mt->getDeclaredInstMethod("toMethodDescriptorString", "()Ljava/lang/String;");
//    return (jstrRef) RSLOT(execJavaFunc(m, {methodType}));
//}

/* ----------------------------------------------------------------------------------------- */

// jref method_handles::getCaller()
// jref getCaller()
// {
//     // public static Lookup lookup();
//     Class *mh = loadBootClass("java/lang/invoke/MethodHandles");
//     Method *lookup = mh->getMethod("lookup", "()Ljava/lang/invoke/MethodHandles$Lookup;");
//     return execJavaR(lookup);
// }

static void push_return_value(Frame *f, slot_t *v) {
    switch (f->method->ret_type) {
        case Method::RET_VOID:      break;
        case Method::RET_BYTE:      f->pushi(slot::get<jbyte>(v));   break;
        case Method::RET_BOOL:      f->pushi(slot::get<jbool>(v));   break;
        case Method::RET_CHAR:      f->pushi(slot::get<jchar>(v));   break;
        case Method::RET_SHORT:     f->pushi(slot::get<jshort>(v));  break;
        case Method::RET_INT:       f->pushi(slot::get<jint>(v));    break;
        case Method::RET_FLOAT:     f->pushf(slot::get<jfloat>(v));  break;
        case Method::RET_LONG:      f->pushl(slot::get<jlong>(v));   break;
        case Method::RET_DOUBLE:    f->pushd(slot::get<jdouble>(v)); break;
        case Method::RET_REFERENCE: f->pushr(slot::get<jref>(v));    break;
        case Method::RET_INVALID:
        default:                    UNREACHABLE("%d", f->method->ret_type);  break;
    }
}

static slot_t* __invokeExact__(const slot_t *args) {
    jref _this = slot::get<jref>(args);

    jref form = _this->get_field_value<jref>(MH_form_id);
    jref entry = form->get_field_value<jref>("vmentry", "Ljava/lang/invoke/MemberName;");
    jref resolved = entry->get_field_value<jref>(MN_method_id);
    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);

    // printvm("+++ %d\n", args->arr_len);
    // printvm("+++ %p\n", entry);
    // printvm("+++ %s\n", target->toString().c_str());

    return execJava(target, args);
}

/**
 * Invokes the method handle, allowing any caller type type, but requiring an exact type match.
 * The symbolic type type at the call site of {@code invokeExact} must
 * exactly match this method handle's {@link #type type}.
 * No conversions are allowed on arguments or return values.
 * <p>
 * When this method is observed via the Core Reflection API,
 * it will appear as a single native method, taking an object array and returning an object.
 * If this native method is invoked directly via
 * {@link java.lang.reflect.Method#invoke java.lang.reflect.Method.invoke}, via JNI,
 * or indirectly via {@link java.lang.invoke.MethodHandles.Lookup#unreflect Lookup.unreflect},
 * it will throw an {@code UnsupportedOperationException}.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 * @throws WrongMethodTypeException if the target's type is not identical with the caller's symbolic type type
 * @throws Throwable anything thrown by the underlying method propagates unchanged through the method handle call
 */
// public final native @PolymorphicSignature Object invokeExact(Object... args) throws Throwable;
static void invokeExact(Frame *f) {
//    slot_t *args = f->lvars;
//    jref _this = slot::get<jref>(args);
//
//    jref form = _this->get_field_value<jref>(MH_form_id);
//    jref entry = form->get_field_value<jref>("vmentry", "Ljava/lang/invoke/MemberName;");
//    jref resolved = entry->get_field_value<jref>(MN_method_id);
//    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);
//
//    // printvm("+++ %d\n", args->arr_len);
//    // printvm("+++ %p\n", entry);
//    // printvm("+++ %s\n", target->toString().c_str());
//
//    slot_t *x = execJava(target, args);
//    push_return_value(f, x);

    slot_t *x = __invokeExact__(f->lvars);
    push_return_value(f, x);
}
/**
 * Invokes the method handle, allowing any caller type type,
 * and optionally performing conversions on arguments and return values.
 * <p>
 * If the call site's symbolic type type exactly matches this method handle's {@link #type type},
 * the call proceeds as if by {@link #invokeExact invokeExact}.
 * <p>
 * Otherwise, the call proceeds as if this method handle were first
 * adjusted by calling {@link #asType asType} to adjust this method handle
 * to the required type, and then the call proceeds as if by
 * {@link #invokeExact invokeExact} on the adjusted method handle.
 * <p>
 * There is no guarantee that the {@code asType} call is actually made.
 * If the JVM can predict the results of making the call, it may perform
 * adaptations directly on the caller's arguments,
 * and call the target method handle according to its own exact type.
 * <p>
 * The resolved type type at the call site of {@code invoke} must
 * be a valid argument to the receivers {@code asType} method.
 * In particular, the caller must specify the same argument arity
 * as the callee's type,
 * if the callee is not a {@linkplain #asVarargsCollector variable arity collector}.
 * <p>
 * When _this method is observed via the Core Reflection API,
 * it will appear as a single native method, taking an object array and returning an object.
 * If _this native method is invoked directly via
 * {@link java.lang.reflect.Method#invoke java.lang.reflect.Method.invoke}, via JNI,
 * or indirectly via {@link java.lang.invoke.MethodHandles.Lookup#unreflect Lookup.unreflect},
 * it will throw an {@code UnsupportedOperationException}.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 * @throws WrongMethodTypeException if the target's type cannot be adjusted to the caller's symbolic type type
 * @throws ClassCastException if the target's type can be adjusted to the caller, but a reference cast fails
 * @throws Throwable anything thrown by the underlying method propagates unchanged through the method handle call
 */
// public final native @PolymorphicSignature Object invoke(Object... args) throws Throwable;
static void invoke(Frame *f) {
    slot_t *args = f->lvars;
    auto len = f->method->arg_slots_count;
    jref _this = slot::get<jref>(args);

    Method *m = get_current_thread()->top_frame->method;
    jref new_type = findMethodType(m->descriptor, m->clazz->loader);
    Method *as_type = _this->clazz->lookup_method(MH_asType_method_desc);
    jref new_handler = execJavaR(as_type, {rslot(_this), rslot(new_type)});

//    slot_t exact_args[len];
    auto exact_args = new slot_t[len];
    slot::set<jref>(exact_args, new_handler);
    for (int i = 1; i < len; i++)
        exact_args[i] = args[i];

    slot_t *x = __invokeExact__(exact_args);
    push_return_value(f, x);
}

/**
 * Private method for trusted invocation of a method handle respecting simplified signatures.
 * Type mismatches will not throw {@code WrongMethodTypeException}, but could crash the JVM.
 * <p>
 * The caller signature is restricted to the following basic types:
 * Object, int, long, float, double, and void return.
 * <p>
 * The caller is responsible for maintaining type correctness by ensuring
 * that the each outgoing argument value is a member of the range of the corresponding
 * callee argument type.
 * (The caller should therefore issue appropriate casts and integer narrowing
 * operations on outgoing argument values.)
 * The caller can assume that the incoming result value is part of the range
 * of the callee's return type.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 */
// final native @PolymorphicSignature Object invokeBasic(Object... args) throws Throwable;
static void invokeBasic(Frame *f) {
    slot_t *args = f->lvars;
    jref _this = slot::get<jref>(args);
    jref form = _this->get_field_value<jref>(MH_form_id);
    jref entry = form->get_field_value<jref>("vmentry", "Ljava/lang/invoke/MemberName;");
    jref resolved = entry->get_field_value<jref>(MN_method_id);
    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);
    slot_t *x = execJava(target, args);
    push_return_value(f, x);
}

/**
 * Private method for trusted invocation of a MemberName of kind {@code REF_invokeVirtual}.
 * The caller signature is restricted to basic types as with {@code invokeBasic}.
 * The trailing (not leading) argument must be a MemberName.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 */
// static native @PolymorphicSignature Object linkToVirtual(Object... args) throws Throwable;
static void linkToVirtual(Frame *f) {
    slot_t *args = f->lvars;
    auto len = f->method->arg_slots_count;
    auto member_name = slot::get<jref>(args + len - 1);
    jref resolved = member_name->get_field_value<jref>(MN_method_id);
    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);

    auto _this = slot::get<jref>(args);
    target = _this->clazz->lookup_method(target->name, target->descriptor);
    slot_t *x = execJava(target, args);
    push_return_value(f, x);
}

/**
 * Private method for trusted invocation of a MemberName of kind {@code REF_invokeStatic}.
 * The caller signature is restricted to basic types as with {@code invokeBasic}.
 * The trailing (not leading) argument must be a MemberName.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 */
// static native @PolymorphicSignature Object linkToStatic(Object... args) throws Throwable;
static void linkToStatic(Frame *f) {
    slot_t *args = f->lvars;
    auto len = f->method->arg_slots_count;
    auto member_name = slot::get<jref>(args + len - 1);
    jref resolved = member_name->get_field_value<jref>(MN_method_id);
    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);
    slot_t *x = execJava(target, args);
    push_return_value(f, x);
}

/**
 * Private method for trusted invocation of a MemberName of kind {@code REF_invokeSpecial}.
 * The caller signature is restricted to basic types as with {@code invokeBasic}.
 * The trailing (not leading) argument must be a MemberName.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 */
// static native @PolymorphicSignature Object linkToSpecial(Object... args) throws Throwable;
static void linkToSpecial(Frame *f) {
    slot_t *args = f->lvars;
    auto len = f->method->arg_slots_count;
    auto member_name = slot::get<jref>(args + len - 1);
    jref resolved = member_name->get_field_value<jref>(MN_method_id);
    auto target = (Method *) (void *) resolved->get_field_value<jref>(RMN_vmtarget_id);
    slot_t *x = execJava(target, args);
    push_return_value(f, x);
}

/**
 * Private method for trusted invocation of a MemberName of kind {@code REF_invokeInterface}.
 * The caller signature is restricted to basic types as with {@code invokeBasic}.
 * The trailing (not leading) argument must be a MemberName.
 * @param args the signature-polymorphic parameter list, statically represented using varargs
 * @return the signature-polymorphic result, statically represented using {@code Object}
 */
// static native @PolymorphicSignature Object linkToInterface(Object... args) throws Throwable;
static void linkToInterface(Frame *f) {
    unimplemented
}

static void linkToNative(Frame *f) {
    unimplemented
}


void java_lang_invoke_MethodHandle_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/invoke/MethodHandle", #method, method_descriptor, method)

    R(invokeExact, "");
    R(invoke, "");
    R(invokeBasic, "");

    R(linkToVirtual, "");
    R(linkToStatic, "");
    R(linkToSpecial, "");
    R(linkToInterface, "");
    R(linkToNative, "");
}
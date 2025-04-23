module;
#include <cassert>
#include "../vmdef.h"

module constant_pool;

import std.core;
import slot;
import object;
import invoke;
import convert;
import class_loader;
import interpreter;

using namespace std;

ConstantPool::ConstantPool(Class *c): owner(c), size(1) {
    assert(owner != nullptr);
    types = new u1[size];
    types[0] = JVM_CONSTANT_Invalid; // constant pool 从 1 开始计数，第0位无效
}

ConstantPool::ConstantPool(Class *c, BytesReader &r): owner(c) {
    assert(owner != nullptr);

    size = r.readu2();

    types = new u1[size];
    types[0] = JVM_CONSTANT_Invalid; // constant pool 从 1 开始计数，第0位无效
    values = new Value[size];

    for (u2 i = 1; i < size; i++) {
        u1 tag = r.readu1();
        set_type(i, tag);
        switch (tag) {
            case JVM_CONSTANT_Class:
            case JVM_CONSTANT_String:
            case JVM_CONSTANT_MethodType:
            case JVM_CONSTANT_Module:
            case JVM_CONSTANT_Package:
                values[i].index = r.readu2();
                break;
            case JVM_CONSTANT_NameAndType:
            case JVM_CONSTANT_Fieldref:
            case JVM_CONSTANT_Methodref:
            case JVM_CONSTANT_InterfaceMethodref:
            case JVM_CONSTANT_Dynamic:
            case JVM_CONSTANT_InvokeDynamic:
                values[i].double_u2._1 = r.readu2();
                values[i].double_u2._2 = r.readu2();
                break;
            case JVM_CONSTANT_Integer: {
                u1 bytes[4];
                r.read_bytes(bytes, 4);
                values[i].i32 = bytes_to_int32(bytes, std::endian::big);
                break;
            }
            case JVM_CONSTANT_Float: {
                u1 bytes[4];
                r.read_bytes(bytes, 4);
                values[i].f32 = bytes_to_float(bytes, std::endian::big);
                break;
            }
            case JVM_CONSTANT_Long: {
                u1 bytes[8];
                r.read_bytes(bytes, 8);
                values[i].i64 = bytes_to_int64(bytes, std::endian::big);
                set_type(++i, JVM_CONSTANT_Placeholder);
                break;
            }
            case JVM_CONSTANT_Double: {
                u1 bytes[8];
                r.read_bytes(bytes, 8);
                values[i].f64 = bytes_to_double(bytes, std::endian::big);
                set_type(++i, JVM_CONSTANT_Placeholder);
                break;
            }
            case JVM_CONSTANT_Utf8: {
                u2 utf8_len = r.readu2();
                values[i].mutf8.s = r.curr_pos();
                values[i].mutf8.len_by_byte = utf8_len;

                auto buf = new utf8_t[utf8_len + 1];
                r.read_bytes((u1 *) buf, utf8_len);
                buf[utf8_len] = 0;

//                const char *utf8 = utf8_pool::find(buf);
//                if (utf8 == nullptr) {
//                    utf8 = utf8_pool::save(buf);
//                } else {
//                    delete[] buf;
//                }
                values[i].buf = buf;
                break;
            }
            case JVM_CONSTANT_MethodHandle:
                values[i].method_handle.reference_kind = r.readu1(); // 这里确实是 readu1
                values[i].method_handle.reference_index = r.readu2();
                break;
            default:
                UNREACHABLE("bad constant tag: %d", tag);
        }
    }
}

ConstantPool::~ConstantPool() {
    delete[] types;
    delete[] values;
}

u2 ConstantPool::get_size() const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return size;
}

u1 ConstantPool::get_type(u2 i) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    return types[i];
}

void ConstantPool::set_type(u2 i, u1 new_type) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    types[i] = new_type;
}

utf8_t *ConstantPool::utf8(u2 i) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Utf8);
    return values[i].buf;
}

MUTF8& ConstantPool::mutf8(u2 i) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Utf8);
    return values[i].mutf8;
}

MUTF8& ConstantPool::string(u2 i) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_String);
    return mutf8(values[i].index);
}

utf8_t *ConstantPool::class_name(u2 i) const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Class);
    return utf8(values[i].index);
}

utf8_t *ConstantPool::module_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Module);
    return utf8(values[i].index);
}

utf8_t *ConstantPool::package_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Package);
    return utf8(values[i].index);
}

utf8_t *ConstantPool::name_of_name_and_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_NameAndType);
    return utf8(values[i].name_and_type.name_index);
}

utf8_t *ConstantPool::type_of_name_and_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_NameAndType);
    return utf8(values[i].name_and_type.descriptor_index);
}

utf8_t *ConstantPool::field_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Fieldref);
    return name_of_name_and_type(values[i].field.name_and_type_index);
}

utf8_t *ConstantPool::field_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Fieldref);
    return type_of_name_and_type(values[i].field.name_and_type_index);
}

utf8_t *ConstantPool::method_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Methodref);
    return name_of_name_and_type(values[i].method.name_and_type_index);
}

utf8_t *ConstantPool::method_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Methodref);
    return type_of_name_and_type(values[i].method.name_and_type_index);
}

utf8_t *ConstantPool::interface_method_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InterfaceMethodref);
    return name_of_name_and_type(values[i].interface_method.name_and_type_index);
}

utf8_t *ConstantPool::interface_method_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InterfaceMethodref);
    return type_of_name_and_type(values[i].interface_method.name_and_type_index);
}

utf8_t *ConstantPool::method_type_descriptor(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_MethodType);
    return utf8(values[i].index);
}

u2 ConstantPool::invoke_dynamic_bootstrap_method_index(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InvokeDynamic);
    return values[i].invoke_dynamic.bootstrap_method_attr_index;
}

utf8_t *ConstantPool::invoke_dynamic_method_name(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InvokeDynamic);
    return name_of_name_and_type(values[i].invoke_dynamic.name_and_type_index);
}

utf8_t *ConstantPool::invoke_dynamic_method_type(u2 i) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InvokeDynamic);
    return type_of_name_and_type(values[i].invoke_dynamic.name_and_type_index);
}

jint ConstantPool::get_int(u2 i) const {
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Integer);
    return values[i].i32;
}

jfloat ConstantPool::get_float(u2 i) const {
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Float);
    return values[i].f32;
}

jlong ConstantPool::get_long(u2 i) const {
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Long);
    return values[i].i64;
}

jdouble ConstantPool::get_double(u2 i) const {
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Double);
    return values[i].f64;
}

Class *ConstantPool::resolve_class(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Class or types[i] == JVM_CONSTANT_ResolvedClass);

    if (types[i] == JVM_CONSTANT_ResolvedClass) {
        return values[i].resolved_class;
    }
    
    Class *c = load_class(owner->loader, class_name(i));
    set_type(i, JVM_CONSTANT_ResolvedClass);
    values[i].resolved_class = c;

    assert(c != nullptr);
    return c;
}

Method *ConstantPool::resolve_method(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Methodref || types[i] == JVM_CONSTANT_ResolvedMethod);

    if (types[i] == JVM_CONSTANT_ResolvedMethod) {
        return values[i].resolved_method;
    }

    Class *c = resolve_class(values[i].method.class_index);
    auto name = method_name(i);
    auto descriptor = method_type(i);
    Method *m = c->lookup_method(name, descriptor);
    if (m == nullptr) {
        m = c->generate_poly_method(name, descriptor);
    }

    set_type(i, JVM_CONSTANT_ResolvedMethod);
    values[i].resolved_method = m;
    return m;
}

Method* ConstantPool::resolve_interface_method(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InterfaceMethodref
           || types[i] == JVM_CONSTANT_ResolvedInterfaceMethod);

    if (types[i] == JVM_CONSTANT_ResolvedInterfaceMethod) {
        return values[i].resolved_method;
    }

    Class *c = resolve_class(values[i].interface_method.class_index);
    Method *m = c->lookup_method(interface_method_name(i), interface_method_type(i));

    set_type(i, JVM_CONSTANT_ResolvedInterfaceMethod);
    values[i].resolved_method = m;
    return m;
}

Method *ConstantPool::resolve_method_or_interface_method(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);

    if (types[i] == JVM_CONSTANT_Methodref || types[i] == JVM_CONSTANT_ResolvedMethod)
        return resolve_method(i);
    if (types[i] == JVM_CONSTANT_InterfaceMethodref || types[i] == JVM_CONSTANT_ResolvedInterfaceMethod)
        return resolve_interface_method(i);

    UNREACHABLE("%d\n", types[i]);
}

Field *ConstantPool::resolve_field(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_Fieldref or types[i] == JVM_CONSTANT_ResolvedField);

    if (types[i] == JVM_CONSTANT_ResolvedField) {
        return values[i].resolved_field;
    }

    Class *c = resolve_class(values[i].field.class_index);
    Field *f = c->lookup_field(field_name(i), field_type(i));

    set_type(i, JVM_CONSTANT_ResolvedField);
    values[i].resolved_field = f;
    return f;
}

Object *ConstantPool::resolve_string(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_String or types[i] == JVM_CONSTANT_ResolvedString);

    if (types[i] == JVM_CONSTANT_ResolvedString) {
        return values[i].resolved_string;
    }

    const MUTF8& str = string(i);
    Object *so = java_lang_String::intern(Allocator::string(str));

    set_type(i, JVM_CONSTANT_ResolvedString);
    values[i].resolved_string = so;
    return so;
}

Object *ConstantPool::resolve_method_type(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_MethodType);
    return findMethodType(method_type_descriptor(i), owner->loader);
}

Object *ConstantPool::resolve_method_handle(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_MethodHandle);

    u2 kind = values[i].method_handle.reference_kind;//method_handle_reference_kind(i);
    u2 index = values[i].method_handle.reference_index;//method_handle_reference_index(i);

//    auto caller = getCaller();
//    const utf8_t *d1 = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;)"
//                       "Ljava/lang/invoke/MethodHandle;";
//    const utf8_t *d2 = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/invoke/MethodType;)"
//                       "Ljava/lang/invoke/MethodHandle;";
//    const utf8_t *d3 = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/invoke/MethodType;Ljava/lang/Class;)"
//                       "Ljava/lang/invoke/MethodHandle;";
//    const utf8_t *d4 = "(Ljava/lang/Class;ILjava/lang/Class;Ljava/lang/String;Ljava/lang/Object;)"
//                       "Ljava/lang/invoke/MethodHandle;";

    const char *name;
    Class *resolved_class;
    Object *type_obj;

    switch (kind) {
        case JVM_REF_getField:
        case JVM_REF_getStatic:
        case JVM_REF_putField:
        case JVM_REF_putStatic: {
            Field *f = resolve_field(index);
            name = f->name;
            resolved_class = f->clazz;
            type_obj = f->get_type();
            break;
        }
        case JVM_REF_invokeVirtual:
        case JVM_REF_invokeStatic:
        case JVM_REF_invokeSpecial:
        case JVM_REF_newInvokeSpecial:
        case JVM_REF_invokeInterface: {
            Method *m = resolve_method_or_interface_method(index);
            name = m->name;
            auto desc = m->descriptor;
            resolved_class = m->clazz;
            type_obj = findMethodType(desc, resolved_class->loader);
            break;
        }
        default:
            UNREACHABLE("wrong reference kind: %d.\n", kind);
    }

    assert(type_obj != nullptr);
    jref method_handle = linkMethodHandleConstant(owner, kind, resolved_class, name, type_obj);
    return method_handle;

//    switch (kind) {
//        case JVM_REF_getField: {
//            Field *f = resolveField(index);
//
//            // public MethodHandle findGetter(Class<?> refc, String name, Class<?> type)
//            //                      throws NoSuchFieldException, IllegalAccessException;
//            Method *m = caller->clazz->getDeclaredInstMethod("findGetter", d1);
//            return getRef(execJavaFunc(m, { caller, f->clazz->java_mirror, newString(f->name), f->getType() }));
//        }
//        case JVM_REF_getStatic: {
//            Field *f = resolveField(index);
//
//            // public MethodHandle findStaticGetter(Class<?> refc, String name, Class<?> type)
//            //                      throws NoSuchFieldException, IllegalAccessException;
//            Method *m = caller->clazz->getDeclaredInstMethod("findStaticGetter", d1);
//            return getRef(execJavaFunc(m, { caller, f->clazz->java_mirror, newString(f->name), f->getType() }));
//        }
//        case JVM_REF_putField: {
//            Field *f = resolveField(index);
//
//            // public MethodHandle findSetter(Class<?> refc, String name, Class<?> type)
//            //                      throws NoSuchFieldException, IllegalAccessException;
//            Method *m = caller->clazz->getDeclaredInstMethod("findSetter", d1);
//            return getRef(execJavaFunc(m, { caller, f->clazz->java_mirror, newString(f->name), f->getType() }));
//        }
//        case JVM_REF_putStatic: {
//            Field *f = resolveField(index);
//
//            // public MethodHandle findStaticSetter(Class<?> refc, String name, Class<?> type)
//            //                      throws NoSuchFieldException, IllegalAccessException;
//            Method *m = caller->clazz->getDeclaredInstMethod("findStaticSetter", d1);
//            return getRef(execJavaFunc(m, { caller, f->clazz->java_mirror, newString(f->name), f->getType() }));
//        }
//        case JVM_REF_invokeVirtual :{
//            // public MethodHandle findVirtual(Class<?> refc, String name, MethodType type)
//            //                      throws NoSuchMethodException, IllegalAccessException;
//            JVM_PANIC("not implement.");
//        }
//        case JVM_REF_invokeStatic: {
//            Method *m = resolveMethod(index);
//            assert(m->isStatic());
//
//            jref mt = findMethodType(m->descriptor, m->clazz->loader);
//
//            Class *c = loadBootClass(S(java_lang_invoke_MethodHandleNatives));
//            // static MethodHandle linkMethodHandleConstant(Class<?> callerClass, int refKind,
//            //                                                  Class<?> defc, String name, Object type)
//            Method *m0 = c->getDeclaredStaticMethod("linkMethodHandleConstant", d4);
//            return getRef(execJavaFunc(m0, { rslot(clazz->java_mirror), islot(kind), rslot(m->clazz->java_mirror), rslot(newString(m->name)), rslot(mt) }));
//            // // public MethodHandle findStatic(Class<?> refc, String name, MethodType type)
//            // //                      throws NoSuchMethodException, IllegalAccessException;
//            // Method *m0 = caller->clazz->getDeclaredInstMethod("findStatic", d2);
//            // return RSLOT(execJavaFunc(m0, { caller, m->clazz->java_mirror, newString(m->name), mt }));
//        }
//        case JVM_REF_invokeSpecial: {
//            // public MethodHandle findSpecial(Class<?> refc, String name, MethodType type, Class<?> specialCaller)
//            //                      throws NoSuchMethodException, IllegalAccessException;
//            JVM_PANIC("not implement.");
//        }
//        case JVM_REF_newInvokeSpecial: {
//            // public MethodHandle findConstructor(Class<?> refc, MethodType type)
//            //                      throws NoSuchMethodException, IllegalAccessException;
//
//            // public MethodHandle findSpecial(Class<?> refc, String name, MethodType type, Class<?> specialCaller)
//            //                      throws NoSuchMethodException, IllegalAccessException;
//           JVM_PANIC("not implement.");
//        }
//        case JVM_REF_invokeInterface: {
//            // public MethodHandle findVirtual(Class<?> refc, String name, MethodType type)
//            //                      throws NoSuchMethodException, IllegalAccessException;
//            JVM_PANIC("not implement.");
//        }
//        default:
//            JVM_PANIC("wrong reference kind: %d.\n", kind);
//    }
}

ConstantPool::ResolvedInvDyn *ConstantPool::resolve_invoke_dynamic(u2 i) {
    lock_guard<recursive_mutex> lock(mutex);
    assert(0 < i && i < size);
    assert(types[i] == JVM_CONSTANT_InvokeDynamic || types[i] == JVM_CONSTANT_ResolvedInvokeDynamic);

    if (types[i] == JVM_CONSTANT_ResolvedInvokeDynamic) {
        return values[i].resolved_inv_dyn;
    }

    const utf8_t *name = invoke_dynamic_method_name(i);
    const utf8_t *descriptor = invoke_dynamic_method_type(i);
    u2 index = invoke_dynamic_bootstrap_method_index(i);

    auto x = new ResolvedInvDyn(name, descriptor, index);
    set_type(i, JVM_CONSTANT_ResolvedInvokeDynamic);
    values[i].resolved_inv_dyn = x;
    return x;
}

string ConstantPool::toString() const {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    ostringstream oss;
    oss << "size: " << size << endl;

    for (decltype(size) i = 1; i < size; i++) {
        oss << "\t" << (int) i;
        u1 _type = get_type(i);
        switch (_type) {
            case JVM_CONSTANT_Class:
                oss << ". [class name] " << class_name(i) << endl;
                break;
            case JVM_CONSTANT_ResolvedClass:
                oss << ". [class name] " << values[i].resolved_class->name << endl;
                break;
            case JVM_CONSTANT_String:
                //oss << ". [string] " << string(i) << endl;
                break;
            case JVM_CONSTANT_ResolvedString:
                //oss << ". [string] " << java_lang_String::to_utf8(values[i].resolved_string) << endl;
                break;
            case JVM_CONSTANT_MethodType:
            case JVM_CONSTANT_Module:
            case JVM_CONSTANT_Package:
            case JVM_CONSTANT_NameAndType:
            case JVM_CONSTANT_Fieldref:
            case JVM_CONSTANT_ResolvedField:
            case JVM_CONSTANT_Methodref:
            case JVM_CONSTANT_ResolvedMethod:
            case JVM_CONSTANT_InterfaceMethodref:
            case JVM_CONSTANT_ResolvedInterfaceMethod:
            case JVM_CONSTANT_Dynamic:
            case JVM_CONSTANT_InvokeDynamic:
                // todo
                break;
            case JVM_CONSTANT_Integer:
                oss << ". [int] " << values[i].i32 << endl;
                break;
            case JVM_CONSTANT_Float:
                oss << ". [float] " << values[i].f32 << endl;
                break;
            case JVM_CONSTANT_Long: 
                oss << ". [long] " << values[i].i64 << endl;
                i++;
                break;
            case JVM_CONSTANT_Double:
                oss << ". [double] " << values[i].f64 << endl;
                i++;
                break;
            case JVM_CONSTANT_Utf8:
                oss << ". [utf8] " << utf8(i) << endl;
                break;
            case JVM_CONSTANT_MethodHandle:
                break;
            default:
                UNREACHABLE("bad constant type: %d", _type);
        }
    }

    return oss.str();
}

Method *find_invoke_dynamic_invoker(
                Class *c, ConstantPool::ResolvedInvDyn *inv_dyn, Object *&appendix) {
    ConstantPool *cp = c->cp;
    ArrayClass *obj_array_class = load_array_class(BOOT_CLASS_LOADER, "[Ljava/lang/Object;");
    Object *name_str = java_lang_String::intern(Allocator::string(inv_dyn->name));
    
    BootstrapMethod &bm = c->bootstrap_methods.at(inv_dyn->boot_method_index);

    jobjArrRef args = Allocator::array(obj_array_class, bm.bootstrap_arguments.size());
    bm.resolve_args(args);

    jref appendix_result = Allocator::array(obj_array_class, 1);
    jref method_type = findMethodType(inv_dyn->descriptor, c->loader);
    jref boot_mthd = cp->resolve_method_handle(bm.bootstrap_method_ref);

    Class *mhn_class = load_boot_class("java/lang/invoke/MethodHandleNatives");

    // static MemberName linkCallSite(Object callerObj,
    //                                int indexInCP,
    //                                Object bootstrapMethodObj,
    //                                Object nameObj, Object typeObj,
    //                                Object staticArguments,
    //                                Object[] appendixResult)
    Method *link_call_site = mhn_class->lookup_method("linkCallSite",
                                    "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;"
                                    "Ljava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)"
                                    "Ljava/lang/invoke/MemberName;");
    assert(link_call_site != nullptr);

    jref member_name = execJavaR(link_call_site, { slot::rslot(c->java_mirror), // caller
                                                    slot::rslot(boot_mthd), 
                                                    slot::rslot(name_str), slot::rslot(method_type), 
                                                    slot::rslot(args),
                                                    slot::rslot(appendix_result)});

    appendix = appendix_result->getElt<jref>(0);

    assert(member_name != nullptr);
    jref resolved = member_name->get_field_value<jref>("method", "Ljava/lang/invoke/ResolvedMethodName;");
    return (Method *) (void *) resolved->get_field_value<jref>("vmtarget", "Ljava/lang/Object;");
}

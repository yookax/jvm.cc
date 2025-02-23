#ifndef CABIN_CONSTANT_POOL_H
#define CABIN_CONSTANT_POOL_H

#include <cassert>
#include <mutex>
#include <string>
#include "../cabin.h"
#include "../slot.h"
#include "constants.h"
#include "bytecode_reader.h"

class Class;
class Method;
class Field;
class Object;

// 从 1 开始计数，第0位无效
class ConstantPool {
    Class *owner = nullptr;
public:
    u1 *type = nullptr;
    slot_t *info = nullptr;
    u2 size = 0;
    
private:
    mutable std::recursive_mutex mutex;

    // Empty ConstantPool
    explicit ConstantPool(Class *c);

    ConstantPool(Class *c, BytecodeReader &r);

public:
    ~ConstantPool();

    u2 get_size() const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        return size;
    }

    u1 get_type(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        return type[i];
    }

    void set_type(u2 i, u1 new_type) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        type[i] = new_type;
    }

    void set_info(u2 i, slot_t new_info) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        info[i] = new_info;
    }

    utf8_t *utf8(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Utf8);
        return (utf8_t *)(info[i]);
    }

    utf8_t *string(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_String);
        return utf8((u2)info[i]);
    }

    utf8_t *class_name(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Class);
        return utf8((u2)info[i]);
    }

    utf8_t *module_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Module);
        return utf8((u2)info[i]);
    }

    utf8_t *package_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Package);
        return utf8((u2)info[i]);
    }

    utf8_t *name_of_name_and_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_NameAndType);
        return utf8((u2)info[i]);
    }

    utf8_t *type_of_name_and_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_NameAndType);
        return utf8((u2) (info[i] >> 16));
    }

    u2 field_class_index(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Fieldref);
        return (u2)info[i];
    }

    utf8_t *field_class_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Fieldref);
        return class_name((u2)info[i]);
    }

    utf8_t *field_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Fieldref);
        return name_of_name_and_type((u2) (info[i] >> 16));
    }

    utf8_t *field_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Fieldref);
        return type_of_name_and_type((u2) (info[i] >> 16));
    }

    u2 method_class_index(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Methodref);
        return (u2)info[i];
    }

    utf8_t *method_class_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Methodref);
        return class_name((u2)info[i]);
    }

    utf8_t *method_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Methodref);
        return name_of_name_and_type((u2) (info[i] >> 16));
    }

    utf8_t *method_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Methodref);
        return type_of_name_and_type((u2) (info[i] >> 16));
    }

    u2 interface_method_class_index(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InterfaceMethodref);
        return (u2)info[i];
    }

    utf8_t *interface_method_class_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InterfaceMethodref);
        return class_name((u2)info[i]);
    }

    utf8_t *interface_method_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InterfaceMethodref);
        return name_of_name_and_type((u2) (info[i] >> 16));
    }

    utf8_t *interface_method_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InterfaceMethodref);
        return type_of_name_and_type((u2) (info[i] >> 16));
    }

    utf8_t *method_type_descriptor(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_MethodType);
        return utf8((u2)info[i]);
    }

    u2 method_handle_reference_kind(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_MethodHandle);
        return (u2) info[i];
    }

    u2 method_handle_reference_index(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_MethodHandle);
        return (u2) (info[i] >> 16);
    }

    u2 invoke_dynamic_bootstrap_method_index(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InvokeDynamic);
        return (u2) info[i];
    }

    utf8_t *invoke_dynamic_method_name(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InvokeDynamic);
        return name_of_name_and_type((u2) (info[i] >> 16));
    }

    utf8_t *invoke_dynamic_method_type(u2 i) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_InvokeDynamic);
        return type_of_name_and_type((u2) (info[i] >> 16));
    }

    jint get_int(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Integer);
        return slot::get<jint>(info + i);
    }

    void set_int(u2 i, jint new_int) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Integer);
        slot::set<jint>(info + i, new_int);
    }

    jfloat get_float(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Float);
        return slot::get<jfloat>(info + i);
    }

    void set_float(u2 i, jfloat new_float) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Float);
        slot::set<jfloat>(info + i, new_float);
    }

    jlong get_long(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Long);
        return slot::get<jlong>(info + i);
    }

    void set_long(u2 i, jlong new_long) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Long);
        slot::set<jlong>(info + i, new_long);
    }

    jdouble get_double(u2 i) const {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Double);
        return slot::get<jdouble>(info + i);
    }

    void set_double(u2 i, jdouble new_double) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        assert(0 < i && i < size);
        assert(type[i] == JVM_CONSTANT_Double);
        slot::set<jdouble>(info + i, new_double);
    }

    Class  *resolve_class(u2 i);
    Method *resolve_method(u2 i);
    Method *resolve_interface_method(u2 i);
    Method *resolve_method_or_interface_method(u2 i);
    Field  *resolve_field(u2 i);
    Object *resolve_string(u2 i);
    Object *resolve_method_type(u2 i);
    Object *resolve_method_handle(u2 i);

    struct ResolvedInvDyn {
        const utf8_t *name;
        const utf8_t *descriptor;
        u2 boot_method_index;

        ResolvedInvDyn(const utf8_t *name0, const utf8_t *descriptor0, u2 boot_method_index0)
            :name(name0), descriptor(descriptor0), boot_method_index(boot_method_index0) { }
    };
    ResolvedInvDyn *resolve_invoke_dynamic(u2 i);

    std::string toString() const;

    friend class Class;
    friend class ArrayClass;
};

Method *find_invoke_dynamic_invoker(
                Class *c, ConstantPool::ResolvedInvDyn *inv_dyn, Object *&appendix);

#endif
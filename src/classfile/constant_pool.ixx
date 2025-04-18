module;
#include "../vmdef.h"

export module constant_pool;

import std.core;
import slot;
import constants;
import encoding;
import bytes_reader;

// 从 1 开始计数，第0位无效
export class ConstantPool {
    Class *owner = nullptr;
public:
    u1 *type = nullptr;
    slot_t *info = nullptr;

    union Value {
        uint16_t index;
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;

        MUTF8 mutf8;
        utf8_t *buf;

        struct {
            uint16_t _1;
            uint16_t _2;
        } double_u2;

        struct {
            uint16_t name_index;
            uint16_t descriptor_index;
        } name_and_type;

        struct {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } field, method, interface_method;

        struct {
            uint16_t bootstrap_method_attr_index;
            uint16_t name_and_type_index;
        } dynamic, invoke_dynamic;

        struct {
            uint8_t reference_kind;
            uint16_t reference_index;
        } method_handle;

        Class *resolved_class;
        Method *resolved_method;
        Field *resolved_field;
        Object *resolved_string;
    } *values = nullptr;

    u2 size = 0;

private:
    mutable std::recursive_mutex mutex;

    // Empty ConstantPool
    explicit ConstantPool(Class *c);

    ConstantPool(Class *c, BytesReader &r);

public:
    ~ConstantPool();

    u2 get_size() const;
    u1 get_type(u2 i) const;
    void set_type(u2 i, u1 new_type);
    void set_info(u2 i, slot_t new_info);

    utf8_t *utf8(u2 i) const;
    utf8_t *string(u2 i) const;
    utf8_t *class_name(u2 i) const;
    utf8_t *module_name(u2 i);
    utf8_t *package_name(u2 i);
    utf8_t *name_of_name_and_type(u2 i);
    utf8_t *type_of_name_and_type(u2 i);
    u2 field_class_index(u2 i);
    utf8_t *field_class_name(u2 i);
    utf8_t *field_name(u2 i);
    utf8_t *field_type(u2 i);
    u2 method_class_index(u2 i);
    utf8_t *method_class_name(u2 i);
    utf8_t *method_name(u2 i);
    utf8_t *method_type(u2 i);
    u2 interface_method_class_index(u2 i);
    utf8_t *interface_method_class_name(u2 i);
    utf8_t *interface_method_name(u2 i);
    utf8_t *interface_method_type(u2 i);
    utf8_t *method_type_descriptor(u2 i);
    u2 method_handle_reference_kind(u2 i);
    u2 method_handle_reference_index(u2 i);
    u2 invoke_dynamic_bootstrap_method_index(u2 i);
    utf8_t *invoke_dynamic_method_name(u2 i);
    utf8_t *invoke_dynamic_method_type(u2 i);
    jint get_int(u2 i) const;
    void set_int(u2 i, jint new_int);
    jfloat get_float(u2 i) const;
    void set_float(u2 i, jfloat new_float);
    jlong get_long(u2 i) const;
    void set_long(u2 i, jlong new_long);
    jdouble get_double(u2 i) const;
    void set_double(u2 i, jdouble new_double);

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

export Method *find_invoke_dynamic_invoker(
        Class *c, ConstantPool::ResolvedInvDyn *inv_dyn, Object *&appendix);
module;
#include "cabin.h"

export module vmstd;

import std.core;

export int processor_number();
export int page_size();
// 返回操作系统的名称。e.g. window 10
export const char *os_name();
// 返回操作系统的架构。e.g. amd64
export const char *os_arch();
export const char *file_separator();
export const char *path_separator();
export const char *line_separator();
export char *get_current_working_directory();


export bool is_prim_class_name(const char *class_name);
export bool is_prim_descriptor(char descriptor);
export bool is_prim_wrapper_class_name(const char *class_name);
export const char *get_prim_array_class_name(const char *class_name);
export const char *get_prim_class_name(char descriptor);
export const char *getPrimDescriptor(const char *wrapper_class_name);
export const char *get_prim_descriptor_by_class_name(const char *class_name);
export const char *get_prim_descriptor_by_wrapper_class_name(const char *wrapper_class_name);


/*
 * 将字节数组转换为32位整形.
 * 字节数组bytes按大端存储，长度4.
 */
export int32_t bytes_to_int32(const uint8_t *bytes);

/*
 * 将字节数组转换为64位整形.
 * 字节数组bytes按大端存储，长度8.
 */
export int64_t bytes_to_int64(const uint8_t *bytes);

/*
 * 将字节数组转换为32位浮点数.
 * 字节数组bytes按大端存储，长度4.
 */
export jfloat bytes_to_float(const uint8_t *bytes);

/*
 * 将字节数组转换为64位浮点数.
 * 字节数组bytes按大端存储，长度8.
 */
export jdouble bytes_to_double(const uint8_t *bytes);


// 一个slot_t类型必须可以容纳 jbool, jbyte, jchar, jshort，jint，jfloat, jref 称为类型一
// jlong, jdouble 称为类型二，占两个slot
export using slot_t = intptr_t;

static_assert(sizeof(slot_t) >= sizeof(jbool), ""); // todo msg
static_assert(sizeof(slot_t) >= sizeof(jbyte), "");
static_assert(sizeof(slot_t) >= sizeof(jchar), "");
static_assert(sizeof(slot_t) >= sizeof(jshort), "");
static_assert(sizeof(slot_t) >= sizeof(jint), "");
static_assert(sizeof(slot_t) >= sizeof(jfloat), "");
static_assert(sizeof(slot_t) >= sizeof(jref), "");
static_assert(2*sizeof(slot_t) >= sizeof(jlong), "");
static_assert(2*sizeof(slot_t) >= sizeof(jdouble), "");

export namespace slot {
    /* setter */
    template<JavaValueType  T>
    void set(slot_t *slots, T v) {
        assert(slots != nullptr);
        * (T *) slots = v;
    }

    template <> void set<jbyte>(slot_t *, jbyte);
    template <> void set<jchar>(slot_t *, jchar);
    template <> void set<jshort>(slot_t *, jshort);

    /* getter */
    template<JavaValueType  T>
    T get(const slot_t *slots) {
        assert(slots != nullptr);
        return (* (const T *) (slots));
    }

    template <> jbyte get<jbyte>(const slot_t *);
    template <> jchar get<jchar>(const slot_t *);
    template <> jshort get<jshort>(const slot_t *);

    /* builder */
    slot_t islot(jint v);
    slot_t fslot(jfloat v);
    slot_t rslot(jref v);
}


// Java Function Return Type
// using jfrt = std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>;

// 此方法用于虚拟机主动调用函数，数调用指令（invokestatic, invokespecial, ...）中不能使用

export slot_t *execJava(Method *, const slot_t *args = nullptr);
export jref execJavaR(Method *, const slot_t *args = nullptr);

// std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>
// exec_java_method(Method *method, const slot_t *args = nullptr);

export slot_t *execJava(Method *, std::initializer_list<slot_t> args);
export jref execJavaR(Method *m, std::initializer_list<slot_t> args);

// Object[] args;
export slot_t *execJava(Method *, jref _this, jarrRef args);
export jref execJavaR(Method *, jref _this, jarrRef args);
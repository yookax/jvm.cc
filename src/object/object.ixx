module;
#include "../cabin.h"
#include "../slot.h"
#include "../classfile/class_loader.h"
#include "../encoding.h"

export module object;

import std.core;
import std.threading;
import classfile;

export class Object {
public:
    bool reachable; // gc时判断对象是否可达

    Class *clazz = nullptr;

private:
    explicit Object(Class *c);
    Object(ArrayClass *arr_cls, jint arr_len);
    Object(ArrayClass *, jint dim, const jint lens[]);

    std::recursive_mutex mutex;

// private:
//     Field *lookupField(const char *name, const char *descriptor);
public:
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }
    void wait(jlong ms) { /* todo "unimplemented" */ }
    void notify() { /* todo "unimplemented" */ }
    void notify_all() { /* todo "unimplemented" */ }

    union {
        Class *jvm_mirror; // present only if Object of java.lang.Class

        jsize arr_len; // present only if Object of Array

        // present only if object of java/lang/ClassLoader
        // save the all loaded classes by this ClassLoader
        std::unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *classes;
    };

    jref protection_domain; // present only if Object of java.lang.Class

    std::string toString() const;

    bool is_array_object() const    { return clazz->name[0] == '['; }
    bool is_class_object() const    { return clazz == g_class_class; }
    bool is_string_object() const   { return clazz == g_string_class; }
    bool is_reference_array() const { return clazz->is_ref_array_class(); }
    bool is_type_array() const      { return clazz->is_type_array_class(); }
    bool is_bool_array() const      { return clazz->is_bool_array_class(); }
    bool is_byte_array() const      { return clazz->is_byte_array_class(); }
    bool is_char_array() const      { return clazz->is_char_array_class(); }
    bool is_short_array() const     { return clazz->is_short_array_class(); }
    bool is_int_array() const       { return clazz->is_int_array_class(); }
    bool is_float_array() const     { return clazz->is_float_array_class(); }
    bool is_long_array() const      { return clazz->is_long_array_class(); }
    bool is_double_array() const    { return clazz->is_double_array_class(); }

    /* Set field value */

    template<JavaValueType T>
    void set_field_value(int field_id, T v) {
        assert(field_id >= 0);
        slot::set<T>(data + field_id, v);
    }

    template<JavaValueType T>
    void set_field_value(Field *f, T v) {
        assert(f != nullptr);
        set_field_value(f->id, v);
    }

    template<JavaValueType T>
    void set_field_value(const char *name, const char *descriptor, T v) {
        assert(name != nullptr);
        set_field_value(clazz->lookup_field(name, descriptor), v);
    }

    template<JavaValueType T>
    void set_field_value(const char *name, T v) {
        assert(name != nullptr);
        set_field_value(name, nullptr, v);
    }

    void set_field_value_raw(Field *f, const slot_t *value);
    void set_field_value_unbox_if_necessary(int field_id, jref value);

    /* Get field value */

    template<JavaValueType T>
    T get_field_value(int field_id) {
        assert(field_id >= 0);
        return slot::get<T>(data + field_id);
    }

    template<JavaValueType T>
    T get_field_value(Field *f) {
        assert(f != nullptr);
        return get_field_value<T>(f->id);
    }

    template<JavaValueType T>
    T get_field_value(const char *name, const char *descriptor = nullptr) {
        assert(name != nullptr);
        assert(clazz != nullptr);
        auto f = clazz->lookup_field(name, descriptor);
        assert(f != nullptr);
        return get_field_value<T>(f->id);
    }

    bool is_instance_of(Class *c) const {
        if (c == nullptr)  // todo
            return false;
        return clazz->is_subclass_of(c);
    }

    Object *clone();
    size_t size() const;

    /* for primitive wrapper object */
    const slot_t *unbox() const;

    std::variant<std::monostate, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble>
    box_value() const;

    //-------- Array's interface

    jsize array_len() const { return arr_len; }
    jsize array_dimension() const { return ((ArrayClass *)clazz)->dimension; }
    void *index(jint i) const;
    bool check_bounds(jint i) const { return 0 <= i && i < arr_len; };

#define setT(jtype, Type, t) \
    void set##Type##Elt(jint i, jtype v) \
    { \
        assert(clazz->is_##t##_array_class()); \
        assert(0 <= i && i < arr_len); \
        *(jtype *) index(i) = v; \
    }

    setT(jbyte, Byte, byte)
    setT(jboolean, Bool, bool)
    setT(jchar, Char, char)
    setT(jshort, Short, short)
    setT(jint, Int,int)
    setT(jlong, Long, long)
    setT(jfloat, Float, float)
    setT(jdouble, Double, double)
#undef setT

    void setRefElt(int i, jref value);

    template <typename T> T getElt(jint index0) const { return *(T *) index(index0); }

    void display(std::string prefix_space = "");

    //--------

    friend class Class;
    friend class ArrayClass;
    friend struct Allocator;

    // 保存所有实例变量的值
    // 包括此Object中定义的和继承来的。
    // 特殊的，对于数组对象，保存数组的元素
    slot_t *data;
};

/*
 * 所有从堆上申请对象的函数都在这里声明
 */
export struct Allocator {
    // alloc non array object
    static Object *object(Class *c);

    // 一维数组
    static Object *array(ArrayClass *ac, jint arr_len);
    // alloc 一维数组
    static jarrRef array(jref class_loader, const char *arr_class_name, jint arr_len);
    // alloc 一维数组，使用 'BOOT_CLASS_LOADER'
    static jarrRef array(const char *arr_class_name, jint arr_len) {
        return array(BOOT_CLASS_LOADER, arr_class_name, arr_len);
    }

    static jarrRef string_array(jint arr_len) { return array("[Ljava/lang/String;", arr_len); }
    static jarrRef class_array(jint arr_len) { return array("[Ljava/lang/Class;", arr_len); }
    static jarrRef object_array(jint arr_len) { return array("[Ljava/lang/Object;", arr_len); }
    static jarrRef object_array(std::initializer_list<jref> os);

    // 多维数组
    static Object *multi_array(ArrayClass *ac, jint dim, const jint lens[]);

    static jstrRef string(const utf8_t *str);
    static jstrRef string(const unicode_t *str, jsize len);
};

/*-------- Array --------*/

// [[[I -> int
// [Ljava/lang/Object; -> java/lang/Object
export const char *arr_class_name_2_elt_class_name(const utf8_t *arr_class_name);

export void array_copy(jarrRef dst, jint dst_pos, const jarrRef src, jint src_pos, jint len);

/*-------- String --------*/

export namespace java_lang_String {
    utf8_t *to_utf8(jstrRef);
    unicode_t *to_unicode(jstrRef);
    bool equals(jstrRef, jstrRef);
    size_t hash(jstrRef);
    jsize length(jstrRef);
    jsize uft_length(jstrRef);
    jstrRef intern(jstrRef);
}

/*-------- Primary Box --------*/

export jref void_box();
export jref byte_box(jbyte x);
export jref bool_box(jbool x);
export jref char_box(jchar x);
export jref short_box(jshort x);
export jref int_box(jint x);
export jref float_box(jfloat x);
export jref long_box(jlong x);
export jref double_box(jdouble x);

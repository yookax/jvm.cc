module;
#include "cabin.h"

export module vmstd;

export import constants;

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

// -----------------------------------------------------------------------

export bool is_prim_class_name(const char *class_name);
export bool is_prim_descriptor(char descriptor);
export bool is_prim_wrapper_class_name(const char *class_name);
export const char *get_prim_array_class_name(const char *class_name);
export const char *get_prim_class_name(char descriptor);
export const char *getPrimDescriptor(const char *wrapper_class_name);
export const char *get_prim_descriptor_by_class_name(const char *class_name);
export const char *get_prim_descriptor_by_wrapper_class_name(const char *wrapper_class_name);

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

/*
 * jvm内部使用的utf8是一种改进过的utf8，与标准的utf8有所不同，
 * 具体参考 jvms。
 *
 * this vm 操作的utf8字符串，要求以'\0'结尾并且不包含utf8的结束符.
 */

export namespace utf8_pool {
    // save a utf8 string to pool.
    // 如不存在，返回新插入的值
    // 如果已存在，则返回池中的值。
    const utf8_t *save(const utf8_t *utf8);

    // get utf8 from pool, return null if not exist.
    const utf8_t *find(const utf8_t *utf8);
}

export namespace utf8 {
    size_t hash(const utf8_t *utf8);

    size_t length(const utf8_t *utf8);

    bool equals(const utf8_t *p1, const utf8_t *p2);

    utf8_t *dup(const utf8_t *utf8);

    utf8_t *dot_2_slash(utf8_t *utf8);
    utf8_t *dot_2_slash_dup(const utf8_t *utf8);

    utf8_t *slash_2_dot(utf8_t *utf8);
    utf8_t *slash_2_dot_dup(const utf8_t *utf8);

    unicode_t *toUnicode(const utf8_t *utf8, size_t unicode_len);

    struct Hash {
        size_t operator()(const utf8_t *utf8) const {
            return hash(utf8);
        }
    };

    struct Comparator {
        bool operator()(const utf8_t *s1, const utf8_t *s2) const {
            assert(s1 != nullptr && s2 != nullptr);
            return equals(s1, s2);
        }
    };
}

export namespace unicode {
    // 由调用者 delete[] utf8 string
    utf8_t *to_utf8(const unicode_t *unicode, size_t len);
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

export struct UncaughtJavaException: public std::exception {
    Object *java_excep;
    explicit UncaughtJavaException(Object *java_excep0): java_excep(java_excep0) { }
};

export struct JavaException: public std::exception {
    const char *excep_class_name = nullptr;
    Object *excep = nullptr;
    std::string msg;

    explicit JavaException(const char *excep_class_name0): excep_class_name(excep_class_name0) {
        assert(excep_class_name != nullptr);
    }

    explicit JavaException(const char *excep_class_name0, std::string msg0)
            : excep_class_name(excep_class_name0), msg(msg0) {
        assert(excep_class_name != nullptr);
    }

    Object *get_excep();
};

#define DEF_EXCEP_CLASS(ClassName, ClassNameStr) \
export struct ClassName: public JavaException { \
    ClassName(): JavaException(ClassNameStr) { } \
    explicit ClassName(std::string msg0): JavaException(ClassNameStr, msg0) { } \
}

DEF_EXCEP_CLASS(java_lang_ArrayStoreException, "java/lang/ArrayStoreException");
DEF_EXCEP_CLASS(java_lang_UnknownError, "java/lang/UnknownError");
DEF_EXCEP_CLASS(java_lang_ArrayIndexOutOfBoundsException, "java/lang/ArrayIndexOutOfBoundsException");
DEF_EXCEP_CLASS(java_lang_ArithmeticException, "java/lang/ArithmeticException");
DEF_EXCEP_CLASS(java_lang_ClassNotFoundException, "java/lang/ClassNotFoundException");
DEF_EXCEP_CLASS(java_lang_InternalError, "java/lang/InternalError");
DEF_EXCEP_CLASS(java_lang_IncompatibleClassChangeError, "java/lang/IncompatibleClassChangeError");
DEF_EXCEP_CLASS(java_lang_IllegalAccessError, "java/lang/IllegalAccessError");
DEF_EXCEP_CLASS(java_lang_AbstractMethodError, "java/lang/AbstractMethodError");
DEF_EXCEP_CLASS(java_lang_InstantiationException, "java/lang/InstantiationException");
DEF_EXCEP_CLASS(java_lang_NegativeArraySizeException, "java/lang/NegativeArraySizeException");
DEF_EXCEP_CLASS(java_lang_NullPointerException, "java/lang/NullPointerException");
DEF_EXCEP_CLASS(java_lang_ClassCastException, "java/lang/ClassCastException");
DEF_EXCEP_CLASS(java_lang_ClassFormatError, "java/lang/ClassFormatError");
DEF_EXCEP_CLASS(java_lang_LinkageError, "java/lang/LinkageError");
DEF_EXCEP_CLASS(java_lang_NoSuchFieldError, "java/lang/NoSuchFieldError");
DEF_EXCEP_CLASS(java_lang_NoSuchMethodError, "java/lang/NoSuchMethodError");
DEF_EXCEP_CLASS(java_lang_IllegalArgumentException, "java/lang/IllegalArgumentException");
DEF_EXCEP_CLASS(java_lang_CloneNotSupportedException, "java/lang/CloneNotSupportedException");
DEF_EXCEP_CLASS(java_lang_VirtualMachineError, "java/lang/VirtualMachineError");
DEF_EXCEP_CLASS(java_io_IOException, "java/io/IOException");
DEF_EXCEP_CLASS(java_io_FileNotFoundException, "java/io/FileNotFoundException");

#undef DEF_EXCEP_CLASS

export void print_stack_trace(Object *e);

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
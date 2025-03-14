module;
#include <cstdint>
#include "vmdef.h"

export module vmstd;

export import constants;
export import convert;
export import sysinfo;
export import primitive;

import std.core;

#if 0
/*
 * Java虚拟机中的整型类型的取值范围如下：
 * 1. 对于byte类型， 取值范围[-2e7, 2e7 - 1]。
 * 2. 对于short类型，取值范围[-2e15, 2e15 - 1]。
 * 3. 对于int类型，  取值范围[-2e31, 2e31 - 1]。
 * 4. 对于long类型， 取值范围[-2e63, 2e63 - 1]。
 * 5. 对于char类型， 取值范围[0, 65535]。
 */
export using jbyte    = int8_t;
export using jboolean = jbyte; // 本虚拟机实现，byte 和 boolean 用同一类型
export using jbool    = jboolean;
export using jchar    = uint16_t;
export using jshort   = int16_t;
export using jint     = int32_t;
export using jlong    = int64_t;
export using jfloat   = float;
export using jdouble  = double;

export using jsize = jint;

// s: signed
export using s1 = int8_t;
export using s2 = int16_t;
export using s4 = int32_t;

// u: unsigned
export using u1 = uint8_t;
export using u2 = uint16_t;
export using u4 = uint32_t;
export using u8 = uint64_t;

export using jref       = Object*; // JVM 中的引用类型
export using jstrRef    = jref;    // java.lang.String 的引用
export using jarrRef    = jref;    // Array 的引用
export using jobjArrRef = jref;    // java.lang.Object Array 的引用
export using jclsRef    = jref;    // java.lang.Class 的引用

#endif


export template <typename T> concept
JavaValueType = std::is_same_v<T, jint>
          || std::is_same_v<T, jbyte> || std::is_same_v<T, jbool>
          || std::is_same_v<T, jchar> || std::is_same_v<T, jshort>
          || std::is_same_v<T, jfloat> || std::is_same_v<T, jlong>
          || std::is_same_v<T, jdouble> || std::is_same_v<T, jref>;

// -----------------------------------------------------------------------

export struct Property {
    const utf8_t *name;
    const utf8_t *value;
    Property(const utf8_t *name0, const utf8_t *value0): name(name0), value(value0) {
        assert(name != nullptr);
        assert(value != nullptr);
    }
};

export std::vector<Property> g_properties;

export struct InitArgs {
    bool asyncgc = false;
    bool verbosegc = false;
    bool verbosedll = false;
    bool verboseclass = false;

    // Whether compaction has been given on the command line, and the value if it has
    bool compact_specified = false;
    int do_compact = false;
    bool trace_jni_sigs = false;

    char *classpath = nullptr;

    char *bootpath = nullptr;
    char *bootpath_a = nullptr;
    char *bootpath_p = nullptr;
    char *bootpath_c = nullptr;
    char *bootpath_v = nullptr;

    int java_stack = VM_STACK_SIZE;
    unsigned long min_heap = VM_HEAP_SIZE;
    unsigned long max_heap = VM_HEAP_SIZE;

    Property *commandline_props;
    int props_count = 0;

    void *main_stack_base;

    /* JNI invocation API hooks */

    int (* vfprintf)(FILE *stream, const char *fmt, va_list ap) = std::vfprintf;
    void (* exit)(int status) = std::exit;
    void (* abort)() = std::abort;
};

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
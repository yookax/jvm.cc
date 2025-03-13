#include <cassert>
#include "../vmdef.h"
#include "../jni.h"

import std.core;
import std.threading;
import vmstd;
import slot;
import object;
import classfile;
import class_loader;

using namespace slot;

#define OBJ   "Ljava/lang/Object;"
#define _OBJ  "(Ljava/lang/Object;"
#define OBJ_  "Ljava/lang/Object;)"

#define CLS   "Ljava/lang/Class;"
#define _CLS  "(Ljava/lang/Class;"
#define _CLS_ "(Ljava/lang/Class;)"

#define STR   "Ljava/lang/String;"

// 这些native函数全部定义在 jdk\internal\misc\Unsafe.java

/* todo
http://www.docjar.com/docs/api/sun/misc/Unsafe.html#park%28boolean,%20long%29
Block current Thread, returning when a balancing
unpark occurs, or a balancing unpark has
already occurred, or the Thread is interrupted, or, if not
absolute and time is not zero, the given time nanoseconds have
elapsed, or if absolute, the given deadline in milliseconds
since Epoch has passed, or spuriously (i.e., returning for no
"reason"). Note: This operation is in the Unsafe class only
because unpark is, so it would be strange to place it
elsewhere.
*/

// public native void park(boolean isAbsolute, long time);
static void park(JNIEnv *env, jref _this, jboolean isAbsolute, jlong time) {
    unimplemented
}

// public native void unpark(Object thread);
static void unpark(JNIEnv *env, jref _this, jref thread) {
    unimplemented
}

/*************************************    compare and swap    ************************************/

/*
 * 第一个参数为需要改变的对象，
 * 第二个为偏移量(参见函数 objectFieldOffset)，
 * 第三个参数为期待的值，
 * 第四个为更新后的值。
 *
 * 整个方法的作用即为若调用该方法时，value的值与expect这个值相等，那么则将value修改为update这个值，并返回一个true，
 * 如果调用该方法时，value的值与expect这个值不相等，那么不做任何操作，并范围一个false。
 *
 * public final native boolean compareAndSwapInt(Object o, long offset, int expected, int x);
 */

static std::mutex mtx;

static jboolean compareAndSwapInt(JNIEnv *env, jref _this,
                                jref o, jlong offset, jint expected, jint x) {
    jint *old;

    if (o == nullptr) {
        // offset is an address
        old = (jint *) offset;
    } else if (o->is_array_object()) {
        old = (jint *) (o->index(offset));
    } else {
        assert(0 <= offset && offset < o->clazz->inst_fields_count);
        old = (jint *) (o->data + offset);
    }
    assert(old != nullptr);

//#ifdef __GNUC__
//    bool b = __sync_bool_compare_and_swap(old, expected, x);
//#else
    std::lock_guard<std::mutex> lock(mtx);
    bool b = (*old == expected);
    if (b) {
        *old = x;
    }
//#endif

    return b ? jtrue : jfalse;
}

static jboolean compareAndSwapLong(JNIEnv *env, jref _this,
                                jref o, jlong offset, jlong expected, jlong x) {
    jlong *old;

    if (o == nullptr) {
        // offset is an address
        old = (jlong *) offset;
    } else if ((o)->is_array_object()) {
        old = (jlong *) ((o)->index(offset));
    } else {
        assert(0 <= offset && offset < o->clazz->inst_fields_count);
        old = (jlong *) (o->data + offset);
    }
    assert(old != nullptr);

//#ifdef __GNUC__
//    bool b = __sync_bool_compare_and_swap(old, expected, x);
//#else
    std::lock_guard<std::mutex> lock(mtx);
    bool b = (*old == expected);
    if (b) {
        *old = x;
    }
//#endif

    return b ? jtrue : jfalse;
}

static jboolean compareAndSwapObject(JNIEnv *env, jref _this,
                                jref o, jlong offset, jref expected, jref x) {
    jref *old;

    if (o == nullptr) {
        // offset is an address
        old = (jref *) offset;
    } else if (o->is_array_object()) {
        old = (jref *) (o->index(offset));
    } else {
        assert(0 <= offset && offset < o->clazz->inst_fields_count);
        old = (jref *) (o->data + offset);
    }
    assert(old != nullptr);

//#ifdef __GNUC__
//    bool b = __sync_bool_compare_and_swap(old, expected, x);
//#else
    std::lock_guard<std::mutex> lock(mtx);
    bool b = (*old == expected);
    if (b) {
        *old = x;
    }
//#endif

    return b ? jtrue : jfalse;
}

/*************************************    class    ************************************/
/** Allocate an instance but do not run any constructor. Initializes the class if it has not yet been. */
// public native Object allocateInstance(Class<?> type) throws InstantiationException;
static jref allocateInstance(JNIEnv *env, jref _this, jclsRef type) {
    Class *c = type->jvm_mirror;
    init_class(c);
    jref o = Allocator::object(c);
    return o;
}

// public native Class defineClass(String name, byte[] b, int off, int len,
//                                  ClassLoader loader, ProtectionDomain protectionDomain)
static jclsRef defineClass_(JNIEnv *env, jref _this, jstrRef name,
                           jref b, jint off, jint len, jref loader, jref protection_domain) {
    return define_class(loader, name, b, off, len, protection_domain)->java_mirror;
}

// public native void ensureClassInitialized(Class<?> c);
static void ensureClassInitialized(JNIEnv *env, jref _this, jclsRef c) {
    init_class(c->jvm_mirror);
}

// public native long staticFieldOffset(Field f);
static jlong staticFieldOffset(JNIEnv *env, jref _this, jref f) {
    return field_offset(f);
    // private String name;
    // utf8_t *name = java_lang_String::toUtf8(f->getRefField("name", "Ljava/lang/String;"));

    // // private Class<?> clazz;
    // Class *c = f->getRefField("clazz", "Ljava/lang/Class;")->jvm_mirror;
    // for (size_t i = 0; i < c->fields.size(); i++) {
    //     if (utf8::equals(c->fields[i]->name, name))
    //         return i;
    // }

    // ShouldNotReachHere("%s", name);
    // return -1; 
}

// public native Object staticFieldBase(Field f); 
static jref staticFieldBase(JNIEnv *env, jref _this, jref f) {
    // private Class<?> clazz;
    jclsRef co = f->get_field_value<jref>("clazz", "Ljava/lang/Class;");
    return co;
}

/*************************************    object    ************************************/

// public native int arrayBaseOffset(Class<?> type)
static jint arrayBaseOffset(JNIEnv *env, jref _this, jclsRef type) {
    return 0; // todo
}

// public native int arrayIndexScale(Class<?> type)
static jint arrayIndexScale(JNIEnv *env, jref _this, jclsRef type) {
    return 1; // todo
}

// public native long objectFieldOffset(Field field)
static jlong objectFieldOffset(JNIEnv *env, jref _this, jref field) {
    // jint offset = field->getIntField("slot");
    // return offset;
    return field_offset(field);
}

// private native long objectFieldOffset1(Class<?> c, String name);
static jlong objectFieldOffset1(JNIEnv *env, jref _this, jclsRef c, jstrRef name) {
    Field *f = c->jvm_mirror->get_field(java_lang_String::to_utf8(name));
    return field_offset(f);
}

#define OBJ_SETTER_AND_GETTER(jtype, type, Type, t) \
static j##type _obj_get_##type##_(JNIEnv *env, jref _this, jref o, jlong offset) \
{ \
    /* o == nullptr 时get内存中的值， offset就是地址 */ \
    if (o == nullptr) { \
        return *(j##type *) (intptr_t) offset; \
    } \
    if ((o)->is_array_object()) { /* get value from array */ \
        assert(0 <= offset && offset < o->arr_len); \
        return (o)->getElt<j##type>(offset); \
    } else if ((o)->is_class_object()) { /* get static filed value */ \
        Class *c = o->jvm_mirror; \
        init_class(c);  \
        return c->fields.at(offset)->static_value.t; \
    } else { \
        assert(0 <= offset && offset < o->clazz->inst_fields_count); \
        return slot::get<jtype>(o->data + offset); \
    } \
} \
\
static void _obj_put_##type##_(JNIEnv *env, jref _this, jref o, jlong offset, j##type x) \
{ \
    /* o == nullptr 时put值到内存中， offset就是地址 */ \
    if (o == nullptr) { \
        *(j##type *) (intptr_t) offset = x; \
        return; \
    } \
    if ((o)->is_array_object()) { /* set value to array */ \
        assert(0 <= offset && offset < o->arr_len); \
        (o)->set##Type##Elt(offset, x); \
    } else if ((o)->is_class_object()) { /* set static filed value */ \
        Class *c = o->jvm_mirror; \
        init_class(c); \
        c->fields.at(offset)->static_value.t = x; \
    } else { \
        assert(0 <= offset && offset < o->clazz->inst_fields_count); \
        slot::set<jtype>(o->data + offset, x); \
    } \
}

OBJ_SETTER_AND_GETTER(jbool, boolean, Bool, z)
OBJ_SETTER_AND_GETTER(jbyte, byte, Byte, b)
OBJ_SETTER_AND_GETTER(jchar, char, Char, c)
OBJ_SETTER_AND_GETTER(jshort, short, Short, s)
OBJ_SETTER_AND_GETTER(jint, int, Int, i)
OBJ_SETTER_AND_GETTER(jlong, long, Long, j)
OBJ_SETTER_AND_GETTER(jfloat, float, Float, f)
OBJ_SETTER_AND_GETTER(jdouble, double, Double, d)
OBJ_SETTER_AND_GETTER(jref, ref, Ref, r)

#undef OBJ_SETTER_AND_GETTER


#define OBJ_SETTER_AND_GETTER_VOLATILE(type) \
static j##type _obj_get_##type##_volatile(JNIEnv *env, jref _this, jref o, jlong offset) \
{ \
    /* todo Volatile */ \
    return _obj_get_##type##_(env, _this, o, offset);  \
} \
 \
static void _obj_put_##type##_volatile(JNIEnv *env, jref _this, jref o, jlong offset, j##type x) \
{ \
    /* todo Volatile */ \
    _obj_put_##type##_(env, _this, o, offset, x); \
}

OBJ_SETTER_AND_GETTER_VOLATILE(boolean)
OBJ_SETTER_AND_GETTER_VOLATILE(byte)
OBJ_SETTER_AND_GETTER_VOLATILE(char)
OBJ_SETTER_AND_GETTER_VOLATILE(short)
OBJ_SETTER_AND_GETTER_VOLATILE(int)
OBJ_SETTER_AND_GETTER_VOLATILE(long)
OBJ_SETTER_AND_GETTER_VOLATILE(float)
OBJ_SETTER_AND_GETTER_VOLATILE(double)

#undef OBJ_SETTER_AND_GETTER_VOLATILE

// -------------------------
// public native Object getObjectVolatile(Object o, long offset);
static jref getObjectVolatile(JNIEnv *env, jref _this, jref o, jlong offset) {
    // todo Volatile

    if (o->is_array_object()) {
        return o->getElt<jref>(offset);
    } else if (o->is_class_object()) {
        Class *c = o->jvm_mirror;
        return c->fields.at(offset)->static_value.r;
    } else {
        assert(0 <= offset && offset < o->clazz->inst_fields_count);
        return *(jref *)(o->data + offset);//o->getInstFieldValue<jref>(offset);  // todo
    }
}

// public native void putObjectVolatile(Object o, long offset, Object x);
static void putObjectVolatile(JNIEnv *env, jref _this, jref o, jlong offset, jref x) {
    // todo Volatile
    if (o->is_array_object()) {
        o->setRefElt(offset, x);
    } else {
        o->set_field_value_unbox_if_necessary(offset, x);
    }
}

/*************************************    unsafe memory    ************************************/
// todo 说明 unsafe memory

/*
 * todo
 * 分配内存方法还有重分配内存方法都是分配的堆外内存，
 * 返回的是一个long类型的地址偏移量。这个偏移量在你的Java程序中每块内存都是唯一的。
 */
// public native long allocateMemory(long bytes);
static jlong allocateMemory(JNIEnv *env, jref _this, jlong bytes) {
    u1 *p = (u1 *) malloc(sizeof(char)*bytes);
    if (p == nullptr) {
        // todo error
    }
    return (jlong) (intptr_t) p;
}

// public native long reallocateMemory(long address, long bytes);
static jlong reallocateMemory(JNIEnv *env, jref _this, jlong address, jlong bytes) {
    return (jlong) (intptr_t) realloc((void *) (intptr_t) address, (size_t) bytes); // 有内存泄漏  todo
}

// public native void freeMemory(long address);
static void freeMemory(JNIEnv *env, jref _this, jlong address) {
    free((void *) (intptr_t) address);
}

static void *getPoint__(jref o, jlong offset) {
    void *p;

    if (o == nullptr) {
        p = (void *) (intptr_t) offset;
    } else if (o->is_array_object()) {
        // offset 在这里表示数组下标(index)
        p = o->index(offset);
    } else {
        // offset 在这里表示 slot id.
        p = o->data;
    }

    return p;
}

/**
 * Sets all bytes in a given block of memory to a fixed value (usually zero).
 * 
 * This method determines a block's base address by means of two parameters,
 * and so it provides (in effect) a <em>double-register</em> addressing mode,
 * as discussed in {@link #getInt(Object,long)}.  When the object reference is null,
 * the offset supplies an absolute base address.
 *
 * The stores are in coherent (atomic) units of a size determined
 * by the address and length parameters.  If the effective address and
 * length are all even modulo 8, the stores take place in 'long' units.
 * If the effective address and length are (resp.) even modulo 4 or 2,
 * the stores take place in units of 'int' or 'short'.
 */
// public native void setMemory(Object o, long offset, long bytes, byte value);
static void setMemory(JNIEnv *env, jref _this, jref o, jlong offset, jlong bytes, jbyte value) {
    void *p = getPoint__(o, offset);
    assert(p != nullptr);
    memset(p, value, bytes);
}

/**
 * Sets all bytes in a given block of memory to a copy of another block.
 *
 * This method determines each block's base address by means of two parameters,
 * and so it provides (in effect) a <em>double-register</em> addressing mode,
 * as discussed in {@link #getInt(Object,long)}.  When the object reference is null,
 * the offset supplies an absolute base address.
 *
 * The transfers are in coherent (atomic) units of a size determined
 * by the address and length parameters.  If the effective addresses and
 * length are all even modulo 8, the transfer takes place in 'long' units.
 * If the effective addresses and length are (resp.) even modulo 4 or 2,
 * the transfer takes place in units of 'int' or 'short'.
 */
// public native void copyMemory(Object srcBase, long srcOffset, Object destBase, long destOffset, long bytes);
static void copyMemory(JNIEnv *env, jref _this, 
                    jref src_base, jlong src_offset, 
                    jref dest_base, jlong dest_offset, jlong bytes) {
    void *src_p = getPoint__(src_base, src_offset);
    void *dest_p = getPoint__(dest_base, dest_offset);

    assert(src_p != nullptr);
    assert(dest_p != nullptr);
    memcpy(dest_p, src_p, bytes);
}

// private native void copySwapMemory0(Object srcBase, long srcOffset, Object destBase, 
//                                long destOffset, long bytes, long elemSize);
static void copySwapMemory(JNIEnv *env, jref _this, 
                    jref src_base, jlong src_offset, 
                    jref dest_base, jlong dest_offset, jlong bytes, jlong elemSize) {
    unimplemented
}

/**
* Gets the load average in the system run queue assigned
* to the available processors averaged over various periods of time.
* This method retrieves the given <tt>nelem</tt> samples and
* assigns to the elements of the given <tt>loadavg</tt> array.
* The system imposes a maximum of 3 samples, representing
* averages over the last 1,  5,  and  15 minutes, respectively.
*
* @params loadavg an array of double of size nelems
* @params nelems the number of samples to be retrieved and
*         must be 1 to 3.
*
* @return the number of samples actually retrieved; or -1
*         if the load average is unobtainable.
*/
// public native int getLoadAverage(double[] loadavg, int nelems);
static jint getLoadAverage(JNIEnv *env, jref _this, jref loadavg, jint nelems) {
    unimplemented
}

// (Ljava/lang/Class;)Z
static jboolean shouldBeInitialized(JNIEnv *env, jref _this, jclsRef c) {
    // todo
    return c->jvm_mirror->state >= Class::State::INITED ? jtrue : jfalse;
}

/**
 * Define a class but do not make it known to the class loader or system dictionary.
 *
 * For each CP entry, the corresponding CP patch must either be null or have
 * the a format that matches its tag:
 * 1. Integer, Long, Float, Double: the corresponding wrapper object type from java.lang
 * 2. Utf8: a string (must have suitable syntax if used as signature or name)
 * 3. Class: any java.lang.Class object
 * 4. String: any object (not just a java.lang.String)
 * 5. InterfaceMethodRef: (NYI) a method handle to invoke on that call site's arguments
 *
 * @hostClass context for linkage, access control, protection domain, and class loader
 * @data      bytes of a class file
 * @cpPatches where non-null entries exist, they replace corresponding CP entries in data
 */
// public native Class defineAnonymousClass(Class hostClass, byte[] data, Object[] cpPatches);
static jclsRef defineAnonymousClass(JNIEnv *env, jref _this, jclsRef host_class, jref data, jref cp_patches)
{
    assert(host_class != nullptr);
    assert(data != nullptr && data->is_array_object());
    assert(cp_patches == nullptr || cp_patches->is_array_object());

    Class *c = define_class(host_class->jvm_mirror->loader, (u1 *) data->data, data->arr_len);
    if (c == nullptr)
        return nullptr; // todo

    int cp_patches_len = cp_patches == nullptr ? 0 : cp_patches->arr_len;
    for (int i = 0; i < cp_patches_len; i++) {
        jref o = cp_patches->getElt<jref>(i);
        if (o != nullptr) {
            u1 type = c->cp->get_type(i);
            if (type == JVM_CONSTANT_String) {
                c->cp->set_info(i, (slot_t) o);
                c->cp->set_type(i, JVM_CONSTANT_ResolvedString);
            } else {
                unimplemented // todo
            }
        }
    }

    // c->nest_host = host_class->jvm_mirror;
    c->set_nest_host(host_class->jvm_mirror);
    link_class(c);

    return c->java_mirror;
}

/** Throw the exception without telling the verifier. */
// public native void throwException(Throwable ee);
static void throwException(JNIEnv *env, jref _this, jref ee) {
    unimplemented
}

// "()V"
static void loadFence(JNIEnv *env, jref _this) {
    unimplemented
}

// "()V"
static void storeFence(JNIEnv *env, jref _this) {
    // unimplemented // todo
}

// "()V"
static void fullFence(JNIEnv *env, jref _this) {
    unimplemented
}

#undef CLD
#define CLD "Ljava/lang/ClassLoader;"

static JNINativeMethod Unsafe_natives[] = {
        { "park", "(ZJ)V", (void *) park },
        { "unpark", _OBJ ")V", (void *) unpark },

        // compare and swap
        { "compareAndSetInt", _OBJ "JII)Z", (void *) compareAndSwapInt },
        { "compareAndSetLong", _OBJ "JJJ)Z", (void *) compareAndSwapLong },
        { "compareAndSetReference", _OBJ "J" OBJ OBJ_ "Z", (void *) compareAndSwapObject },

        // class
        { "allocateInstance", _CLS_ OBJ, (void *) allocateInstance },
        { "defineClass0", "(" STR "[BII" CLD "Ljava/security/ProtectionDomain;)" CLS, (void *) defineClass_ },
        { "ensureClassInitialized0", _CLS_ "V", (void *) ensureClassInitialized },
        { "staticFieldOffset0", "(Ljava/lang/reflect/Field;)J", (void *) staticFieldOffset },
        { "staticFieldBase0", "(Ljava/lang/reflect/Field;)" OBJ, (void *) staticFieldBase },

        // Object
        { "arrayBaseOffset0", _CLS_"I", (void *) arrayBaseOffset },
        { "arrayIndexScale0", _CLS_"I", (void *) arrayIndexScale },
        { "objectFieldOffset0", "(Ljava/lang/reflect/Field;)J", (void *) objectFieldOffset },
        { "objectFieldOffset1", _CLS STR ")J", (void *) objectFieldOffset1 },

        { "getBoolean", _OBJ "J)Z", (void *) _obj_get_boolean_ },
        { "putBoolean", _OBJ "JZ)V", (void *) _obj_put_boolean_ },
        { "getByte", _OBJ "J)B", (void *) _obj_get_byte_ },
        { "putByte", _OBJ "JB)V", (void *) _obj_put_byte_ },
        { "getChar", _OBJ "J)C", (void *) _obj_get_char_ },
        { "putChar", _OBJ "JC)V", (void *) _obj_put_char_ },
        { "getShort", _OBJ "J)S", (void *) _obj_get_short_ },
        { "putShort", _OBJ "JS)V", (void *) _obj_put_short_ },
        { "getInt", _OBJ "J)I", (void *) _obj_get_int_ },
        { "putInt", _OBJ "JI)V", (void *) _obj_put_int_ },
        { "getLong", _OBJ "J)J", (void *) _obj_get_long_ },
        { "putLong", _OBJ "JJ)V", (void *) _obj_put_long_ },
        { "getFloat", _OBJ "J)F", (void *) _obj_get_float_ },
        { "putFloat", _OBJ "JF)V", (void *) _obj_put_float_ },
        { "getDouble", _OBJ "J)D", (void *) _obj_get_double_ },
        { "putDouble", _OBJ "JD)V",(void *)  _obj_put_double_ },
        { "getReference", _OBJ "J)" OBJ, (void *) _obj_get_ref_ },
        { "putReference", _OBJ "J" OBJ_ "V", (void *) _obj_put_ref_ },

        { "putIntVolatile", _OBJ "JI)V", (void *) _obj_put_int_volatile },
        { "putBooleanVolatile", _OBJ "JZ)V", (void *) _obj_put_boolean_volatile },
        { "putByteVolatile", _OBJ "JB)V", (void *) _obj_put_byte_volatile },
        { "putShortVolatile", _OBJ "JS)V", (void *) _obj_put_short_volatile },
        { "putCharVolatile", _OBJ "JC)V", (void *) _obj_put_char_volatile },
        { "putLongVolatile", _OBJ "JJ)V", (void *) _obj_put_long_volatile },
        { "putFloatVolatile", _OBJ "JF)V", (void *) _obj_put_float_volatile },
        { "putDoubleVolatile", _OBJ "JD)V", (void *) _obj_put_double_volatile },
        { "putReferenceVolatile", _OBJ "J" OBJ_ "V", (void *) putObjectVolatile },

        { "getCharVolatile", _OBJ "J)C", (void *) _obj_get_char_volatile },
        { "getIntVolatile", _OBJ "J)I", (void *) _obj_get_int_volatile },
        { "getBooleanVolatile", _OBJ "J)Z", (void *) _obj_get_boolean_volatile },
        { "getByteVolatile", _OBJ "J)B", (void *) _obj_get_byte_volatile },
        { "getShortVolatile", _OBJ "J)S", (void *) _obj_get_short_volatile },
        { "getLongVolatile", _OBJ "J)J", (void *) _obj_get_long_volatile },
        { "getFloatVolatile", _OBJ "J)F", (void *) _obj_get_float_volatile },
        { "getDoubleVolatile", _OBJ "J)D", (void *) _obj_get_double_volatile },
        { "getReferenceVolatile", _OBJ "J)" OBJ, (void *) getObjectVolatile },

        // unsafe memory
        { "allocateMemory0", "(J)J", (void *) allocateMemory },
        { "reallocateMemory0", "(JJ)J", (void *) reallocateMemory },
        { "setMemory0", _OBJ "JJB)V", (void *) setMemory },
        { "copyMemory0", _OBJ "JLjava/lang/Object;JJ)V", (void *) copyMemory },
        { "copySwapMemory0", _OBJ "JLjava/lang/Object;JJJ)V", (void *) copySwapMemory },
        { "freeMemory0", "(J)V", (void *) freeMemory },

        { "shouldBeInitialized0", _CLS_ "Z", (void *) shouldBeInitialized },
        { "getLoadAverage0", "([DI)I", (void *) getLoadAverage },
        { "defineAnonymousClass0", _CLS "[B[" OBJ_ CLS, (void *) defineAnonymousClass },
        { "throwException", "(Ljava/lang/Throwable;)V", (void *) throwException },

        { "loadFence", "()V", (void *) loadFence },
        { "storeFence", "()V", (void *) storeFence },
        { "fullFence", "()V", (void *) fullFence },
};

void Unsafe_registerNatives(JNIEnv *env, jclass cls) {
    env->RegisterNatives(cls, Unsafe_natives, std::size(Unsafe_natives));
}


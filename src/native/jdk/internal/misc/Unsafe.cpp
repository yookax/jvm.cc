module;
#include <assert.h>
#include "../../../../vmdef.h"

module native;

import std;
// import std.threading;
import slot;
import object;
import classfile;
import constants;
import runtime;
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
static void park(Frame *f) {
    unimplemented
}

// public native void unpark(Object thread);
static void unpark(Frame *f) {
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

//public final native boolean compareAndSetInt(Object o, long offset, int expected, int x);
static void compareAndSetInt(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);
    args += 2;
    auto expected = slot::get<jint>(args++);
    auto x = slot::get<jint>(args);

    jint *old;

    if (o == nullptr) {
        // offset is an address
        old = (jint *) offset;
    } else if (o->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto i = offset / es;
        assert(0 <= i && i < o->arr_len);
        old = (jint *) (o->index(i));
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

    f->pushi(b ? jtrue : jfalse);
}

//public final native boolean compareAndSetLong(Object o, long offset, long expected, long x);
static void compareAndSetLong(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);
    args += 2;
    auto expected = slot::get<jlong>(args);
    args += 2;
    auto x = slot::get<jlong>(args);

    jlong *old;

    if (o == nullptr) {
        // offset is an address
        old = (jlong *) offset;
    } else if ((o)->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto i = offset / es;
        assert(0 <= i && i < o->arr_len);
        old = (jlong *) ((o)->index(i));
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

    f->pushi(b ? jtrue : jfalse);
}

//public final native boolean compareAndSetReference(Object o, long offset, Object expected, Object x);
static void compareAndSetReference(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);
    args += 2;
    auto expected = slot::get<jref>(args++);
    auto x = slot::get<jref>(args);

    jref *old;

    if (o == nullptr) {
        // offset is an address
        old = (jref *) offset;
    } else if (o->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto i = offset / es;
        assert(0 <= i && i < o->arr_len);
        old = (jref *) (o->index(i));
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

    f->pushi(b ? jtrue : jfalse);
}

/*************************************    class    ************************************/
// Allocate an instance but do not run any constructor. Initializes the class if it has not yet been.
// public native Object allocateInstance(Class<?> cls) throws InstantiationException;
static void allocateInstance(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto co = slot::get<jref>(args);

    Class *c = co->jvm_mirror;
    init_class(c);
    jref o = Allocator::object(c);
    f->pushr(o);
}

// public native Class<?> defineClass0(String name, byte[] b, int off, int len,
//                                        ClassLoader loader,
//                                        ProtectionDomain protectionDomain);
static void defineClass0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto name = slot::get<jref>(args++);
    auto b = slot::get<jref>(args++);
    auto off = slot::get<jint>(args++);
    auto len = slot::get<jint>(args++);
    auto loader = slot::get<jref>(args++);
    auto protection_domain = slot::get<jref>(args);

    Class *c = define_class(loader, name, b, off, len, protection_domain);
    f->pushr(c->java_mirror);
}

// private native void ensureClassInitialized0(Class<?> c);
static void ensureClassInitialized0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto co = slot::get<jref>(args);
    init_class(co->jvm_mirror);
}

// private native long staticFieldOffset0(Field f);
static void staticFieldOffset0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto fo = slot::get<jref>(args);
    f->pushl(field_offset(fo));
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

// private native Object staticFieldBase0(Field f);
static void staticFieldBase0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto fo = slot::get<jref>(args);

    // private Class<?> clazz;
    jclsRef co = fo->get_field_value<jref>("clazz", "Ljava/lang/Class;");
    f->pushr(co);
}

/*************************************    object    ************************************/

// private native int arrayBaseOffset0(Class<?> arrayClass);
static void arrayBaseOffset0(Frame *f) {
    f->pushi(0); // todo
}

// private native int arrayIndexScale0(Class<?> arrayClass);
static void arrayIndexScale0(Frame *f) {
//    f->pushi(1); // todo

    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto ao = slot::get<jref>(args);
    auto c = ao->jvm_mirror;
    assert(c->name[0] == '[');
    auto ac = (ArrayClass *) c;
    f->pushi(ac->get_element_size());
}

// private native long objectFieldOffset0(Field field);
static void objectFieldOffset0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto fo = slot::get<jref>(args);
    // jint offset = fo->getIntField("slot");
    // return offset;
    f->pushl(field_offset(fo));
}

// private native long objectFieldOffset1(Class<?> c, String name);
static void objectFieldOffset1(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto co = slot::get<jref>(args++);
    auto name = slot::get<jref>(args);
    Field *field = co->jvm_mirror->get_field(java_lang_String::to_utf8(name));
    f->pushl(field_offset(field));
}

#define OBJ_SETTER_AND_GETTER(jtype, type, Type, t, t0, jtype0) \
static void _obj_get_##type##_(Frame *f) { \
    slot_t *args = f->lvars; \
    args++; /* jump 'this' */ \
    auto o = slot::get<jref>(args++); \
    auto offset = slot::get<jlong>(args); \
    /* o == nullptr 时get内存中的值， offset就是地址 */ \
    if (o == nullptr) { \
        f->push##t0(*(j##type *) (intptr_t) offset); return;\
    } \
    if ((o)->is_array_object()) { /* get value from array */ \
        auto ac = (ArrayClass *) o->clazz; \
        auto es = ac->get_element_size();  \
        assert(offset % es == 0);  \
        auto index = offset / es; \
        assert(0 <= index && index < o->arr_len); \
        f->push##t0((o)->getElt<j##type>(index)); return;\
    } else if ((o)->is_class_object()) { /* get static filed value */ \
        Class *c = o->jvm_mirror; \
        init_class(c);  \
        f->push##t0(c->fields.at(offset)->static_value.t); return; \
    } else { \
        assert(0 <= offset && offset < o->clazz->inst_fields_count); \
        f->push##t0(slot::get<jtype>(o->data + offset)); return; \
    } \
} \
\
static void _obj_put_##type##_(Frame *f) { \
    slot_t *args = f->lvars; \
    args++; /* jump 'this' */  \
    auto o = slot::get<jref>(args++); \
    auto offset = slot::get<jlong>(args); \
    args += 2; \
    auto x = (jtype)slot::get<jtype0>(args); \
    /* o == nullptr 时put值到内存中， offset就是地址 */ \
    if (o == nullptr) { \
        *(j##type *) (intptr_t) offset = x; \
        return; \
    } \
    if ((o)->is_array_object()) { /* set value to array */ \
        auto ac = (ArrayClass *) o->clazz; \
        auto es = ac->get_element_size();  \
        assert(offset % es == 0);  \
        auto index = offset / es; \
        assert(0 <= index && index < o->arr_len); \
        (o)->set##Type##Elt(index, x); \
    } else if ((o)->is_class_object()) { /* set static filed value */ \
        Class *c = o->jvm_mirror; \
        init_class(c); \
        c->fields.at(offset)->static_value.t = x; \
    } else { \
        assert(0 <= offset && offset < o->clazz->inst_fields_count); \
        slot::set<jtype>(o->data + offset, x); \
    } \
}

OBJ_SETTER_AND_GETTER(jbool, boolean, Bool, z, i, jint)
OBJ_SETTER_AND_GETTER(jbyte, byte, Byte, b, i, jint)
OBJ_SETTER_AND_GETTER(jchar, char, Char, c, i, jint)
OBJ_SETTER_AND_GETTER(jshort, short, Short, s, i, jint)
OBJ_SETTER_AND_GETTER(jint, int, Int, i, i, jint)
OBJ_SETTER_AND_GETTER(jlong, long, Long, j, l, jlong)
OBJ_SETTER_AND_GETTER(jfloat, float, Float, f, f, jfloat)
OBJ_SETTER_AND_GETTER(jdouble, double, Double, d, d, jdouble)
OBJ_SETTER_AND_GETTER(jref, ref, Ref, r, r, jref)

#undef OBJ_SETTER_AND_GETTER

//public native int getInt(Object o, long offset);
//public native void putInt(Object o, long offset, int x);
//public native Object getReference(Object o, long offset);
//public native void putReference(Object o, long offset, Object x);
//public native boolean getBoolean(Object o, long offset);
//public native void putBoolean(Object o, long offset, boolean x);
//public native byte getByte(Object o, long offset);
//public native void putByte(Object o, long offset, byte x);
//public native short getShort(Object o, long offset);
//public native void putShort(Object o, long offset, short x);
//public native char getChar(Object o, long offset);
//public native void putChar(Object o, long offset, char x);
//public native long getLong(Object o, long offset);
//public native void putLong(Object o, long offset, long x);
//public native float getFloat(Object o, long offset);
//public native void putFloat(Object o, long offset, float x);
//public native double getDouble(Object o, long offset);
//public native void putDouble(Object o, long offset, double x);


#define OBJ_SETTER_AND_GETTER_VOLATILE(type) \
static void _obj_get_##type##_volatile(Frame *f) { \
    /* todo Volatile */ \
    _obj_get_##type##_(f);  \
} \
 \
static void _obj_put_##type##_volatile(Frame *f) { \
    /* todo Volatile */ \
    _obj_put_##type##_(f); \
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
static void getObjectVolatile(Frame *f) {
    // todo Volatile

    slot_t *args = f->lvars;
    args++; /* jump 'this' */
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);

    if (o->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto index = offset / es;
        assert(0 <= index && index < o->arr_len);
        f->pushr(o->getElt<jref>(index));
    } else if (o->is_class_object()) {
        Class *c = o->jvm_mirror;
        f->pushr(c->fields.at(offset)->static_value.r);
    } else {
        assert(0 <= offset && offset < o->clazz->inst_fields_count);
        f->pushr(*(jref *)(o->data + offset));//o->getInstFieldValue<jref>(offset);  // todo
    }
}

// public native void putObjectVolatile(Object o, long offset, Object x);
static void putObjectVolatile(Frame *f) {
    // todo Volatile

    slot_t *args = f->lvars;
    args++; /* jump 'this' */
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);
    args += 2;
    auto x = slot::get<jref>(args);

    if (o->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto index = offset / es;
        assert(0 <= index && index < o->arr_len);
        o->setRefElt(index, x);
    } else {
        o->set_field_value_unbox_if_necessary(offset, x);
    }
}


/*************************************    unsafe memory    ************************************/

/*
 * todo
 * 分配内存方法还有重分配内存方法都是分配的堆外内存，
 * 返回的是一个long类型的地址偏移量。这个偏移量在你的Java程序中每块内存都是唯一的。
 */
// public native long allocateMemory0(long bytes);
static void allocateMemory0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto bytes = slot::get<jlong>(args);

    u1 *p = (u1 *) malloc(sizeof(char)*bytes);
    if (p == nullptr) {
        // todo error
    }
    f->pushl((jlong) (intptr_t) p);
}

// public native long reallocateMemory0(long address, long bytes);
static void reallocateMemory0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto address = slot::get<jlong>(args);
    args += 2;
    auto bytes = slot::get<jlong>(args);
    f->pushl((jlong) (intptr_t) realloc((void *) (intptr_t) address, (size_t) bytes)); // 有内存泄漏  todo
}

// public native void freeMemory0(long address);
static void freeMemory0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto address = slot::get<jlong>(args);
    free((void *) (intptr_t) address);
}

static void *getPoint__(jref o, jlong offset) {
    void *p;

    if (o == nullptr) {
        p = (void *) (intptr_t) offset;
    } else if (o->is_array_object()) {
        auto ac = (ArrayClass *) o->clazz;
        auto es = ac->get_element_size();
        assert(offset % es == 0);
        auto index = offset / es;
        assert(0 <= index && index < o->arr_len);
        p = o->index(offset);
    } else {
        // offset 在这里表示 slot id.
        p = o->data;
    }

    return p;
}

/*
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
// public native void setMemory0(Object o, long offset, long bytes, byte value);
static void setMemory0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto o = slot::get<jref>(args++);
    auto offset = slot::get<jlong>(args);
    args += 2;
    auto bytes = slot::get<jlong>(args);
    args += 2;
    auto value = slot::get<jbyte>(args);

    void *p = getPoint__(o, offset);
    assert(p != nullptr);
    memset(p, value, bytes);
}

/*
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
// public native void copyMemory0(Object srcBase, long srcOffset, Object destBase, long destOffset, long bytes);
static void copyMemory0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto src_base = slot::get<jref>(args++);
    auto src_offset = slot::get<jlong>(args);
    args += 2;
    auto dest_base = slot::get<jref>(args++);
    auto dest_offset = slot::get<jlong>(args);
    args += 2;
    auto bytes = slot::get<jlong>(args);

    void *src_p = getPoint__(src_base, src_offset);
    void *dest_p = getPoint__(dest_base, dest_offset);

    assert(src_p != nullptr);
    assert(dest_p != nullptr);
    memcpy(dest_p, src_p, bytes);
}

// private native void copySwapMemory0(Object srcBase, long srcOffset, Object destBase,
//                                long destOffset, long bytes, long elemSize);
static void copySwapMemory0(Frame *f) {
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
// private native int getLoadAverage0(double[] loadavg, int nelems);
static void getLoadAverage0(Frame *f) {
    unimplemented
}

// private native boolean shouldBeInitialized0(Class<?> c);
static void shouldBeInitialized0(Frame *f) {
    // todo
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto co = slot::get<jref>(args);
    jbool b = co->jvm_mirror->state >= Class::State::INITED ? jtrue : jfalse;
    f->pushi(b);
}
#if 0
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

#endif

// Throw the exception without telling the verifier.
// public native void throwException(Throwable ee);
static void throwException(Frame *f) {
    unimplemented
}

// "()V"
static void loadFence(Frame *f) {
    // todo
}

// "()V"
static void storeFence(Frame *f) {
    // todo
}

// public native void fullFence();
static void fullFence(Frame *f) {
    // todo
}

#undef CLD
#define CLD "Ljava/lang/ClassLoader;"

void jdk_internal_misc_Unsafe_registerNatives(Frame *f) {
#undef R
#undef R0
#define R(method, method_descriptor) \
    registry("jdk/internal/misc/Unsafe", #method, method_descriptor, method)
#define R0(method_name, method_descriptor, method) \
    registry("jdk/internal/misc/Unsafe", method_name, method_descriptor, method)

    R(park, "(ZJ)V");
    R(unpark, _OBJ ")V");

    R(compareAndSetInt, _OBJ "JII)Z");
    R(compareAndSetLong, _OBJ "JJJ)Z");
    R(compareAndSetReference, _OBJ "J" OBJ OBJ_ "Z");

    // class
    R(allocateInstance, _CLS_ OBJ);
    R(defineClass0, "(" STR "[BII" CLD "Ljava/security/ProtectionDomain;)" CLS);
    R(ensureClassInitialized0, _CLS_ "V");
    R(staticFieldOffset0, "(Ljava/lang/reflect/Field;)J");
    R(staticFieldBase0, "(Ljava/lang/reflect/Field;)" OBJ);

    // Object
    R(arrayBaseOffset0, _CLS_"I");
    R(arrayIndexScale0, _CLS_"I");
    R(objectFieldOffset0, "(Ljava/lang/reflect/Field;)J");
    R(objectFieldOffset1, _CLS STR ")J");

    R0("getBoolean", _OBJ "J)Z", _obj_get_boolean_);
    R0("putBoolean", _OBJ "JZ)V",  _obj_put_boolean_);
    R0("getByte", _OBJ "J)B", _obj_get_byte_);
    R0("putByte", _OBJ "JB)V", _obj_put_byte_);
    R0("getChar", _OBJ "J)C", _obj_get_char_);
    R0("putChar", _OBJ "JC)V", _obj_put_char_);
    R0("getShort", _OBJ "J)S", _obj_get_short_);
    R0("putShort", _OBJ "JS)V", _obj_put_short_);
    R0("getInt", _OBJ "J)I", _obj_get_int_);
    R0("putInt", _OBJ "JI)V", _obj_put_int_);
    R0("getLong", _OBJ "J)J", _obj_get_long_);
    R0("putLong", _OBJ "JJ)V", _obj_put_long_);
    R0("getFloat", _OBJ "J)F", _obj_get_float_);
    R0("putFloat", _OBJ "JF)V", _obj_put_float_);
    R0("getDouble", _OBJ "J)D", _obj_get_double_);
    R0("putDouble", _OBJ "JD)V", _obj_put_double_);
    R0("getReference", _OBJ "J)" OBJ, _obj_get_ref_);
    R0("putReference", _OBJ "J" OBJ_ "V", _obj_put_ref_);

    R0("putIntVolatile", _OBJ "JI)V", _obj_put_int_volatile);
    R0("putBooleanVolatile", _OBJ "JZ)V", _obj_put_boolean_volatile);
    R0("putByteVolatile", _OBJ "JB)V", _obj_put_byte_volatile);
    R0("putShortVolatile", _OBJ "JS)V", _obj_put_short_volatile);
    R0("putCharVolatile", _OBJ "JC)V", _obj_put_char_volatile);
    R0("putLongVolatile", _OBJ "JJ)V", _obj_put_long_volatile);
    R0("putFloatVolatile", _OBJ "JF)V", _obj_put_float_volatile);
    R0("putDoubleVolatile", _OBJ "JD)V", _obj_put_double_volatile);
    R0("putReferenceVolatile", _OBJ "J" OBJ_ "V", putObjectVolatile);

    R0("getCharVolatile", _OBJ "J)C", _obj_get_char_volatile);
    R0("getIntVolatile", _OBJ "J)I", _obj_get_int_volatile);
    R0("getBooleanVolatile", _OBJ "J)Z",_obj_get_boolean_volatile);
    R0("getByteVolatile", _OBJ "J)B", _obj_get_byte_volatile);
    R0("getShortVolatile", _OBJ "J)S", _obj_get_short_volatile);
    R0("getLongVolatile", _OBJ "J)J", _obj_get_long_volatile);
    R0("getFloatVolatile", _OBJ "J)F", _obj_get_float_volatile);
    R0("getDoubleVolatile", _OBJ "J)D", _obj_get_double_volatile);
    R0("getReferenceVolatile", _OBJ "J)" OBJ, getObjectVolatile);

//    // unsafe memory
    R(allocateMemory0, "(J)J");
    R(reallocateMemory0, "(JJ)J");
    R(setMemory0, _OBJ "JJB)V");
    R(copyMemory0, _OBJ "JLjava/lang/Object;JJ)V");
    R(copySwapMemory0, _OBJ "JLjava/lang/Object;JJJ)V");
    R(freeMemory0, "(J)V");

    R(shouldBeInitialized0, _CLS_ "Z");
    R(getLoadAverage0, "([DI)I");
//    R("defineAnonymousClass0", _CLS "[B[" OBJ_ CLS, defineAnonymousClass);
    R(throwException, "(Ljava/lang/Throwable;)V");

    R(loadFence, "()V");
    R(storeFence, "()V");
    R(fullFence, "()V");
}
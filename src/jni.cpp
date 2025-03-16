#include "vmdef.h"
#include "jni.h"

import slot;
import runtime;
import object;
import classfile;
import class_loader;
import interpreter;

using namespace slot;

static Class *nio_Buffer_class;
static int nio_Buffer_capacity_id; // private int capacity;
static int nio_Buffer_address_id;  // long address;

static Class *nio_DirectByteBuffer_class;
static Method *nio_DirectByteBuffer_init_method; // private Direct$Type$Buffer(long addr, int cap)

void init_jni() {
    /* Cache class method and fields for JNI 1.4 NIO support */

    nio_Buffer_class = load_boot_class("java/nio/Buffer");
    
    nio_Buffer_capacity_id = nio_Buffer_class->get_field("capacity", "I")->id;
    nio_Buffer_address_id = nio_Buffer_class->get_field("address", "J")->id;
    nio_DirectByteBuffer_class = load_boot_class("java/nio/DirectByteBuffer");
    nio_DirectByteBuffer_init_method = nio_DirectByteBuffer_class->get_method("<init>", "(JI)V");
}

bool isSupportedJNIVersion(int version) {
    // todo 支持的jni版本都有哪些????
    return version == JNI_VERSION_1_6 ||
           version == JNI_VERSION_1_4 ||
           version == JNI_VERSION_1_2 || 
           version == JNI_VERSION_1_1;
}

#define JVM_MIRROR(jclass) ((jclsRef) jclass)->jvm_mirror

static jobject addJNIGlobalRef(Object *ref) {
    // todo 
    return (jobject) ref;
}

static void deleteJNIGlobalRef(Object *ref) {
    // todo
}

static slot_t *execJavaV(Method *m, jref _this, va_list args) {
    assert(m != nullptr);
    if (m->access_flags.is_static()) {
        init_class(m->clazz);
    }

    // Class[]
    jarrRef types = m->get_parameter_types();
    jsize args_count = types->arr_len;
    
    // 因为有 category two 的存在，result 的长度最大为 types_len * 2 + this_obj
    auto real_args = new slot_t[2*args_count + 1];
    int k = 0;
    if (_this != nullptr) {
        assert(!m->access_flags.is_static());
        set<jref>(real_args, _this);
        k++;
    }

    for (int i = 0; i < args_count; i++, k++) {
        Class *c = types->getElt<jref>(i)->jvm_mirror;

        // 可变长参数列表误区与陷阱——va_arg不可接受的类型：
        // https://www.cnblogs.com/shiweihappy/p/4246442.html
        if (c->check_class_name("boolean")) {
            set<jbool>(real_args + k, va_arg(args, jint));
        } else if (c->check_class_name("byte")) {
            set<jbyte>(real_args + k, va_arg(args, jint));
        } else if (c->check_class_name("char")) {
            set<jchar>(real_args + k, va_arg(args, jint));
        } else if (c->check_class_name("short")) {
            set<jshort>(real_args + k, va_arg(args, jint));
        } else if (c->check_class_name("int")) {
            set<jint>(real_args + k, va_arg(args, jint));
        } else if (c->check_class_name("float")) {
            set<jfloat>(real_args + k, va_arg(args, jdouble));
        } else if (c->check_class_name("long")) { // category_two
            set<jlong>(real_args + k++, va_arg(args, jlong));
        } else if (c->check_class_name("double")) { // category_two
            set<jdouble>(real_args + k++, va_arg(args, jdouble));
        } else {
            set<jref>(real_args + k, va_arg(args, jref));
        }
    }

    return execJava(m, real_args);
}

static slot_t *execJavaA(Method *m, jref _this, const jvalue *args) {
    assert(m != nullptr);
    if (m->access_flags.is_static()) {
        init_class(m->clazz);
    }

    // Class[]
    jarrRef types = m->get_parameter_types();
    jsize args_count = types->arr_len;
    
    // 因为有 category two 的存在，result 的长度最大为 types_len * 2 + this_obj
    auto real_args = new slot_t[2*args_count + 1];
    int k = 0;
    if (_this != nullptr) {
        assert(!m->access_flags.is_static());
        set<jref>(real_args, _this);
        k++;
    }

    for (int i = 0; i < args_count; i++, k++) {
        Class *c = types->getElt<jref>(i)->jvm_mirror;

        if (c->check_class_name("boolean")) {
            set<jbool>(real_args + k, args[i].z);
        } else if (c->check_class_name("byte")) {
            set<jbyte>(real_args + k, args[i].b);
        } else if (c->check_class_name("char")) {
            set<jchar>(real_args + k, args[i].c);
        } else if (c->check_class_name("short")) {
            set<jshort>(real_args + k, args[i].s);
        } else if (c->check_class_name("int")) {
            set<jint>(real_args + k, args[i].i);
        } else if (c->check_class_name("float")) {
            set<jfloat>(real_args + k, args[i].f);
        } else if (c->check_class_name("long")) { // category_two
            set<jlong>(real_args + k++, args[i].j);
        } else if (c->check_class_name("double")) { // category_two
            set<jdouble>(real_args + k++, args[i].d);
        } else {
            set<jref>(real_args + k, (jref) args[i].l);
        }
    }

    return execJava(m, real_args);
}

// GlobalRefTable global_refs;
// GlobalRefTable weak_global_refs;

jint JNICALL Jvmcc_GetVersion(JNIEnv *env) {
    TRACE("Jvmcc_GetVersion(env=%p)", env);
    return JNI_VERSION;
}

jclass JNICALL Jvmcc_DefineClass(JNIEnv *env,
                const char *name, jobject loader, const jbyte *buf, jsize len) {
    TRACE("Jvmcc_DefineClass(env=%p, name=%s, loader=%p, buf=%p, len=%d)", env, name, loader, buf, len);

    assert(name != nullptr && buf != nullptr && len >= 0);

    Class *c = define_class((jref) loader, (const u1 *) buf, len);
    if (c != nullptr)
        link_class(c);

    return (*env)->NewLocalRef(env, (jobject) c->java_mirror);
}

jclass JNICALL Jvmcc_FindClass(JNIEnv *env, const char *name) {
    // assert(env != nullptr && name != nullptr);
    // jref loader = ((Frame *) (*env)->functions->reserved3)->method->clazz->loader;
    // Class *c = loadClass(loader, name);
    // return to_jclass(c);

    Class *c = loadClass(nullptr, name);
    if (c != nullptr) {
        return (jclass) c->java_mirror;
    }

    c = loadClass(g_app_class_loader, name);
    return (jclass) c->java_mirror;
}

jmethodID JNICALL Jvmcc_FromReflectedMethod(JNIEnv *env, jobject method) {
    unimplemented
}

jfieldID JNICALL Jvmcc_FromReflectedField(JNIEnv *env, jobject field) {
    unimplemented
}

jobject JNICALL Jvmcc_ToReflectedMethod(JNIEnv *env, jclass cls,
                                        jmethodID methodID, jboolean isStatic) {
    unimplemented
}

/**
 * Returns the Class representing the superclass of the entity
 * (class, interface, primitive type or void) represented by this
 * Class.  If this Class represents either the Object class, 
 * an interface, a primitive type, or void, then null is returned.  
 * If this object represents an array class then the
 * Class object representing the Object class is returned.
 */
jclass JNICALL Jvmcc_GetSuperclass(JNIEnv *env, jclass sub) {
    Class *c = ((jclsRef) sub)->jvm_mirror;
    if (c->access_flags.is_interface() || c->is_prim_class() || c->check_class_name("void"))
        return nullptr;
    if (c->super_class == nullptr)
        return nullptr;
    return (jclass) c->super_class->java_mirror;
}

// 查看 static jboolean Class::isAssignableFrom(JNIEnv *env, jclsRef this, jclsRef cls);
jboolean JNICALL Jvmcc_IsAssignableFrom(JNIEnv *env, jclass sub, jclass sup) {
    return JVM_MIRROR(sub)->is_subclass_of(JVM_MIRROR(sup));
}

jobject JNICALL Jvmcc_ToReflectedField(JNIEnv *env, jclass cls,
                                       jfieldID fieldID, jboolean isStatic) {
    unimplemented
}

jint JNICALL Jvmcc_Throw(JNIEnv *env, jthrowable obj) {
    // set_exception((jref) obj);
    Thread::jniThrow((jref) obj);
    return JNI_TRUE;
}

jint JNICALL Jvmcc_ThrowNew(JNIEnv *env, jclass clazz, const char *msg) {
    Class *c = JVM_MIRROR(clazz);
    Thread::jniThrow(c, msg);
    return JNI_TRUE;
}

jthrowable JNICALL Jvmcc_ExceptionOccurred(JNIEnv *env) {
    return (jthrowable) Thread::jniExceptionOccurred();
}

void JNICALL Jvmcc_ExceptionDescribe(JNIEnv *env) {
    unimplemented
}

void JNICALL Jvmcc_ExceptionClear(JNIEnv *env) {
    Thread::jniExceptionClear();
}

void JNICALL Jvmcc_FatalError(JNIEnv *env, const char *msg) {
    unimplemented
}

jint JNICALL Jvmcc_PushLocalFrame(JNIEnv *env, jint capacity) {
    unimplemented
}

jobject JNICALL Jvmcc_PopLocalFrame(JNIEnv *env, jobject result) {
    unimplemented
}

/*
 * JNI局部引用、全局引用和弱全局引用：   
 * https://blog.csdn.net/xyang81/article/details/44657385
 */

jobject JNICALL Jvmcc_NewGlobalRef(JNIEnv *env, jobject gref) {
    // todo
    // UNIMPLEMENTED
    // assert(env != nullptr && gref != nullptr);
    return addJNIGlobalRef((jref) gref); // todo
}

void JNICALL Jvmcc_DeleteGlobalRef(JNIEnv *env, jobject gref) {
    // todo
    unimplemented
    // assert(env != nullptr && gref != nullptr);
    // auto o = to_object_ref(gref);
    // if (o->jni_obj_ref_type == JNIGlobalRefType)
    //     deleteJNIGlobalRef(o);
}

void JNICALL Jvmcc_DeleteLocalRef(JNIEnv *env, jobject obj) {
    assert(env != nullptr);
    if (obj == nullptr)
        return;

    Frame *f = get_current_thread()->top_frame;
    for (int i = 0; i < f->jni_local_ref_count; i++) {
        jref *t = f->jni_local_ref_table + i;
        if ((*env)->IsSameObject(env, (jobject) *t, obj)) {
            memmove(t, t + 1, (f->jni_local_ref_count - i)*sizeof(jref));
            f->jni_local_ref_count--;
            return;
        }
    }

    // WARN("Delete a absent local ref(%p)", obj); todo
}

jboolean JNICALL Jvmcc_IsSameObject(JNIEnv *env, jobject obj1, jobject obj2) {
    assert(env != nullptr);
    return obj1 == obj2;
}

jobject JNICALL Jvmcc_NewLocalRef(JNIEnv *env, jobject obj) {
    (void) env;
    if (obj == nullptr)
        return nullptr;

    Frame *f = get_current_thread()->top_frame;
    if (f == nullptr) {
        return obj;
    }

    if (f->jni_local_ref_count < JNI_LOCAL_REFERENCE_TABLE_MAX_CAPACITY) {
        f->jni_local_ref_table[f->jni_local_ref_count++] = (jref) obj;
        return obj;
    }

    // todo
    // JNI_THROW(env, S(java_lang_OutOfMemoryError), );
    panic("local reference table overflow (max=%d)", JNI_LOCAL_REFERENCE_TABLE_MAX_CAPACITY);
}

jint JNICALL Jvmcc_EnsureLocalCapacity(JNIEnv *env, jint capacity) {
    assert(env != nullptr);

    if (capacity > JNI_LOCAL_REFERENCE_TABLE_MAX_CAPACITY) {
        // 本jvm比支持局部引用表的扩展
        return JNI_ERR;
    }
    return JNI_OK;
}

jobject JNICALL Jvmcc_AllocObject(JNIEnv *env, jclass clazz) {
    Class *c = JVM_MIRROR(clazz);
    if (c->access_flags.is_abstract() || c->access_flags.is_interface()) {
        // Can not be instantiated
        JNI_THROW(env, "java.lang.InstantiationException", c->name);
    }

    // Make sure it is initialised
    init_class(c);
    return (*env)->NewLocalRef(env, (jobject) Allocator::object(c));
//    return (jobject) Allocator::object(c);;
    // return addJNILocalRef(newObject(c));
}

jobject JNICALL Jvmcc_NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    va_list args;
    va_start(args, methodID);
    jobject o = (*env)->NewObjectV(env, clazz, methodID, args);
    va_end(args);
    return o;
}

jobject JNICALL Jvmcc_NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    jobject o = (*env)->AllocObject(env, clazz);
    if (o == nullptr) {
        // todo error
    }

    slot_t *ret = execJavaV((Method *) methodID, (jref) o, args); 
    return (jobject) get<jref>(ret);
}

jobject JNICALL Jvmcc_NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    jobject o = (*env)->AllocObject(env, clazz);
    if (o == nullptr) {
        // todo error
    }

    slot_t *ret = execJavaA((Method *) methodID, (jref) o, args);
    return (jobject) get<jref>(ret);
}

jclass JNICALL Jvmcc_GetObjectClass(JNIEnv *env, jobject obj) {
    assert(env != nullptr && obj != nullptr);
    return (jclass) ((jref)obj)->clazz->java_mirror;
}

jboolean JNICALL Jvmcc_IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
    // assert(env != nullptr && obj != nullptr && clazz != nullptr);
    // auto o = to_object_ref(obj);
    // auto c = to_object_ref<Class>(clazz);
    // return o->isInstanceOf(c) ? JNI_TRUE : JNI_FALSE;
    
    return ((jref) obj)->is_instance_of(JVM_MIRROR(clazz));
}

jmethodID JNICALL Jvmcc_GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    Class *c = JVM_MIRROR(clazz);
    Method *m = c->lookup_method(name, sig);
    return (jmethodID) m;
}

/*
 * 定义 Call_T_Method 镞函数：
 *
 * T JNICALL JVM_Call_T_Method(JNIEnv *env, jobject obj, jmethodID methodID, ...);
 * T JNICALL JVM_Call_T_MethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
 * T JNICALL JVM_Call_T_MethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);
 */

#define DEFINE_CALL_T_METHOD_BODY \
    jref _this = (jref)(obj); \
    Method *_m0 = (Method *)(methodID); \
    Method *_m = _this->clazz->lookup_method(_m0->name, _m0->descriptor); \
    if (_m == nullptr) { \
        /* todo error */ \
    }

#define DEFINE_CALL_T_METHOD(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_Call##T##Method(JNIEnv *env, jobject obj, jmethodID methodID, ...) \
{                                                                                         \
    va_list args;                                                                         \
    va_start(args, methodID);                                                             \
    DEFINE_CALL_T_METHOD_BODY                                                             \
    slot_t *ret = execJavaV(_m, _this, args);                                             \
    (void) ret; \
    va_end(args); \
    return ret_value;                                                                     \
}

#define DEFINE_CALL_T_METHOD_V(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_Call##T##MethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) \
{ \
    DEFINE_CALL_T_METHOD_BODY \
    slot_t *ret = execJavaV(_m, _this, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_CALL_T_METHOD_A(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_Call##T##MethodA(JNIEnv *env, \
                                       jobject obj, jmethodID methodID, const jvalue *args) \
{ \
    DEFINE_CALL_T_METHOD_BODY \
    slot_t *ret = execJavaA(_m, _this, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_3_CALL_T_METHODS(T, ret_type, ret_value) \
    DEFINE_CALL_T_METHOD(T, ret_type, ret_value) \
    DEFINE_CALL_T_METHOD_V(T, ret_type, ret_value) \
    DEFINE_CALL_T_METHOD_A(T, ret_type, ret_value)


DEFINE_3_CALL_T_METHODS(Object, jobject, (*env)->NewLocalRef(env, (jobject) get<jref>(ret)))
DEFINE_3_CALL_T_METHODS(Boolean, jboolean, get<jbool>(ret))
DEFINE_3_CALL_T_METHODS(Byte, jbyte, get<jbyte>(ret))
DEFINE_3_CALL_T_METHODS(Char, jchar, get<jchar>(ret))
DEFINE_3_CALL_T_METHODS(Short, jshort, get<jshort>(ret))
DEFINE_3_CALL_T_METHODS(Int, jint, get<jint>(ret))
DEFINE_3_CALL_T_METHODS(Long, jlong, get<jlong>(ret))
DEFINE_3_CALL_T_METHODS(Float, jfloat, get<jfloat>(ret))
DEFINE_3_CALL_T_METHODS(Double, jdouble, get<jdouble>(ret))
DEFINE_3_CALL_T_METHODS(Void, void, )

#undef DEFINE_CALL_T_METHOD_BODY
#undef DEFINE_CALL_T_METHOD
#undef DEFINE_CALL_T_METHOD_V
#undef DEFINE_CALL_T_METHOD_A
#undef DEFINE_3_CALL_T_METHODS

/*
 * 定义 CallNonvirtual_T_Method 镞函数：
 *
 * T JNICALL JVM_CallNonvirtual_T_Method(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
 * T JNICALL JVM_CallNonvirtual_T_MethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);
 * T JNICALL JVM_CallNonvirtual_T_MethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args);
 */

#define DEFINE_CALL_NONVIRTUAL_T_METHOD(T, ret_type, ret_value)  \
ret_type JNICALL Jvmcc_CallNonvirtual##T##Method(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) \
{ \
    va_list args; \
    va_start(args, methodID); \
    slot_t *ret = execJavaV((Method *) methodID, (jref) obj, args); \
    (void) ret; \
    va_end(args); \
    return ret_value; \
}

#define DEFINE_CALL_NONVIRTUAL_T_METHOD_V(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_CallNonvirtual##T##MethodV(JNIEnv *env, jobject obj, \
                            jclass clazz, jmethodID methodID, va_list args) \
{ \
    slot_t *ret = execJavaV((Method *) methodID, (jref) obj, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_CALL_NONVIRTUAL_T_METHOD_A(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_CallNonvirtual##T##MethodA(JNIEnv *env, jobject obj, \
                            jclass clazz, jmethodID methodID, const jvalue *args) \
{ \
    slot_t *ret = execJavaA((Method *) methodID, (jref) obj, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_3_CALL_NONVIRTUAL_T_METHODS(T, ret_type, ret_value) \
    DEFINE_CALL_NONVIRTUAL_T_METHOD_A(T, ret_type, ret_value) \
    DEFINE_CALL_NONVIRTUAL_T_METHOD_V(T, ret_type, ret_value) \
    DEFINE_CALL_NONVIRTUAL_T_METHOD(T, ret_type, ret_value)


DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Object, jobject, (*env)->NewLocalRef(env, (jobject) get<jref>(ret)))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Boolean, jboolean, get<jbool>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Byte, jbyte, get<jbyte>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Char, jchar, get<jchar>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Short, jshort, get<jshort>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Int, jint, get<jint>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Long, jlong, get<jlong>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Float, jfloat, get<jfloat>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Double, jdouble, get<jdouble>(ret))
DEFINE_3_CALL_NONVIRTUAL_T_METHODS(Void, void, )

#undef DEFINE_CALL_NONVIRTUAL_T_METHOD
#undef DEFINE_CALL_NONVIRTUAL_T_METHOD_A
#undef DEFINE_CALL_NONVIRTUAL_T_METHOD_V
#undef DEFINE_3_CALL_NONVIRTUAL_T_METHODS

/*
* 定义 CallStatic_T_Method 镞函数：
*
* T JNICALL JVM_CallStatic_T_Method(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
* T JNICALL JVM_CallStatic_T_MethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args);
* T JNICALL JVM_CallStatic_T_MethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args);
*/

#define DEFINE_CALL_STATIC_T_METHOD(T, ret_type, ret_value)  \
ret_type JNICALL Jvmcc_CallStatic##T##Method(JNIEnv *env, jclass clazz, jmethodID methodID, ...) \
{ \
    va_list args; \
    va_start(args, methodID); \
    slot_t *ret = execJavaV((Method *) methodID, nullptr, args); \
    (void) ret; \
    va_end(args); \
    return ret_value; \
}

#define DEFINE_CALL_STATIC_T_METHOD_V(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_CallStatic##T##MethodV(JNIEnv *env, \
                            jclass clazz, jmethodID methodID, va_list args) \
{ \
    slot_t *ret = execJavaV((Method *) methodID, nullptr, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_CALL_STATIC_T_METHOD_A(T, ret_type, ret_value) \
ret_type JNICALL Jvmcc_CallStatic##T##MethodA(JNIEnv *env, \
                            jclass clazz, jmethodID methodID, const jvalue *args) \
{ \
    slot_t *ret = execJavaA((Method *) methodID, nullptr, args); \
    (void) ret; \
    return ret_value; \
}

#define DEFINE_3_CALL_STATIC_T_METHODS(T, ret_type, ret_value) \
    DEFINE_CALL_STATIC_T_METHOD(T, ret_type, ret_value) \
    DEFINE_CALL_STATIC_T_METHOD_V(T, ret_type, ret_value) \
    DEFINE_CALL_STATIC_T_METHOD_A(T, ret_type, ret_value)

DEFINE_3_CALL_STATIC_T_METHODS(Object, jobject, (*env)->NewLocalRef(env, (jobject) get<jref>(ret)))
DEFINE_3_CALL_STATIC_T_METHODS(Boolean, jboolean, get<jbool>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Byte, jbyte, get<jbyte>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Char, jchar, get<jchar>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Short, jshort, get<jshort>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Int, jint, get<jint>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Long, jlong, get<jlong>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Float, jfloat, get<jfloat>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Double, jdouble, get<jdouble>(ret))
DEFINE_3_CALL_STATIC_T_METHODS(Void, void, )

#undef DEFINE_CALL_STATIC_T_METHOD
#undef DEFINE_CALL_STATIC_T_METHOD_V
#undef DEFINE_CALL_STATIC_T_METHOD_A
#undef DEFINE_3_CALL_STATIC_T_METHODS

jfieldID JNICALL Jvmcc_GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    Class *c = JVM_MIRROR(clazz);
    init_class(c);

    Field *f = c->lookup_field(name, sig);
    if (f == nullptr) {
        // todo java_lang_NoSuchFieldError
    }
    return (jfieldID) f;
}

jobject JNICALL Jvmcc_GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID) {
    return (jobject) ((jref) obj)->get_field_value<jref>((Field *) fieldID);
}

#define GET_AND_SET_FIELD(Type, jtype, t) \
jtype JNICALL Jvmcc_Get##Type##Field(JNIEnv *env, jobject obj, jfieldID fieldID) {   \
    (void) env; \
    assert(obj != nullptr); \
    assert(fieldID != nullptr); \
    return ((jref) obj)->get_field_value<jtype>((Field *) fieldID); \
} \
\
void JNICALL Jvmcc_Set##Type##Field(JNIEnv *env, jobject obj, jfieldID fieldID, jtype val) { \
    (void) env; \
    assert(obj != nullptr); \
    assert(fieldID != nullptr); \
    ((jref) obj)->set_field_value<jtype>((Field *) fieldID, val); \
} \
\
jtype JNICALL Jvmcc_GetStatic##Type##Field(JNIEnv *env, jclass clazz, jfieldID fieldID) { \
    (void) env; \
    (void) clazz; \
    assert(fieldID != nullptr); \
    return ((Field *) fieldID)->static_value.t; \
} \
\
void JNICALL Jvmcc_SetStatic##Type##Field(JNIEnv *env, jclass clazz, jfieldID fieldID, jtype value) { \
    (void) env; \
    (void) clazz; \
    assert(fieldID != nullptr); \
    ((Field *) fieldID)->static_value.t = value; \
}

GET_AND_SET_FIELD(Bool, jboolean, z)
GET_AND_SET_FIELD(Byte, jbyte, b)
GET_AND_SET_FIELD(Char, jchar, c)
GET_AND_SET_FIELD(Short, jshort, s)
GET_AND_SET_FIELD(Int, jint, i)
GET_AND_SET_FIELD(Long, jlong, j)
GET_AND_SET_FIELD(Float, jfloat, f)
GET_AND_SET_FIELD(Double, jdouble, d)

#undef GET_AND_SET_FIELD

void JNICALL Jvmcc_SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject val) {
    ((jref) obj)->set_field_value<jref>((Field *) fieldID, (jref) val);
}

jmethodID JNICALL Jvmcc_GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    Method *m = JVM_MIRROR(clazz)->lookup_method(name, sig);
    if (m == nullptr || !m->access_flags.is_static())
        return nullptr;
    return (jmethodID) m;
}

jfieldID JNICALL Jvmcc_GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    Field *f = JVM_MIRROR(clazz)->lookup_field(name, sig);
    if (f == nullptr || !f->access_flags.is_static())
        return nullptr;
    return (jfieldID) f;
}

jobject JNICALL Jvmcc_GetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID) {
    assert(env != nullptr && clazz != nullptr && fieldID != nullptr);
    return (jobject) (((Field *) fieldID)->static_value.r);
}

void JNICALL Jvmcc_SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value) {
    ((Field *) fieldID)->static_value.r = (jref) value;
}

jstring JNICALL Jvmcc_NewString(JNIEnv *env, const jchar *unicode, jsize len) {
    jstrRef str = Allocator::string(unicode, len);
    return (*env)->NewLocalRef(env, (jobject) str);
}

jsize JNICALL Jvmcc_GetStringLength(JNIEnv *env, jstring str) {
    return java_lang_String::length((jstrRef) str);
}

const jchar *JNICALL Jvmcc_GetStringChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    // // addJNIGlobalRef(so); /* Pin the reference */
    if (isCopy != nullptr)
        *isCopy = JNI_TRUE;

    unicode_t *u = java_lang_String::to_unicode((jstrRef)str);
    return u;
}

void JNICALL Jvmcc_ReleaseStringChars(JNIEnv *env, jstring str, const jchar *chars) {
    // if (g_jdk_version_9_and_upper) {
    //     delete[] chars;
    // } else {
    //     deleteJNIGlobalRef(to_object_ref(str)); /* Unpin the reference */
    // }

    // todo

    // UNIMPLEMENTED // todo
}

jstring JNICALL Jvmcc_NewStringUTF(JNIEnv *env, const char *utf) {
    jstrRef str = Allocator::string(utf);
    return (*env)->NewLocalRef(env, (jobject) str);
}

jsize JNICALL Jvmcc_GetStringUTFLength(JNIEnv *env, jstring str) {
    return java_lang_String::uft_length((jstrRef) str);
}

const char* JNICALL Jvmcc_GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy) {
//    // addJNIGlobalRef(so); /* Pin the reference */
    if (isCopy != nullptr)
        *isCopy = JNI_TRUE;

    utf8_t *u = java_lang_String::to_utf8((jstrRef)str);
    return u;
}

void JNICALL Jvmcc_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *chars) {
    // if (g_jdk_version_9_and_upper) {
    //     deleteJNIGlobalRef(to_object_ref(str)); /* Unpin the reference */
    // } else {
    //     delete[] chars;
    // }

    // UNIMPLEMENTED // todo
}

jsize JNICALL Jvmcc_GetArrayLength(JNIEnv *env, jarray arr) {
    auto array = (jarrRef) arr;
    return array->arr_len;
}

jobjectArray JNICALL Jvmcc_NewObjectArray(JNIEnv *env, jsize len, jclass elementClass, jobject init) {
    if (len < 0) {
        JNI_THROW_NegativeArraySizeException(env, nullptr);
        return nullptr;
    }

    jarrRef arr = Allocator::array(JVM_MIRROR(elementClass)->generate_array_class(), len);
    if (init != nullptr) {
        for (int i = 0; i < len; ++i) {
            arr->setRefElt(i, (jref) init);
        }
    }

    return (jobjectArray) (*env)->NewLocalRef(env, (jobject) arr);
}

jobject JNICALL Jvmcc_GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index) {
    auto ao = (jarrRef) array;
    if (index <= 0 || index >= ao->arr_len) {
        // todo error
    }

    return (jobject) ao->getElt<jref>(index);
}

void JNICALL Jvmcc_SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject val) {
    auto ao = (jarrRef) array;
    if (index < 0 || index >= ao->arr_len) {
        JNI_THROW_ArrayIndexOutOfBoundsException(env, nullptr); // todo msg
        return;
    }

    ao->setRefElt(index, (jref) val);
}

#define NEW_TYPE_ARRAY(Type, class_name) \
jarray JNICALL Jvmcc_New##Type##Array(JNIEnv *env, jsize len) { \
    if (len < 0) { \
        JNI_THROW_NegativeArraySizeException(env, nullptr); \
        return nullptr; \
    } \
 \
    jarrRef arr = Allocator::array(BOOT_CLASS_LOADER, class_name, len); \
    return (*env)->NewLocalRef(env, (jobject) arr); \
}

NEW_TYPE_ARRAY(Byte, "[B")
NEW_TYPE_ARRAY(Boolean, "[Z")
NEW_TYPE_ARRAY(Char, "[C")
NEW_TYPE_ARRAY(Short, "[S")
NEW_TYPE_ARRAY(Int, "[I")
NEW_TYPE_ARRAY(Long, "[J")
NEW_TYPE_ARRAY(Float, "[F")
NEW_TYPE_ARRAY(Double, "[D")

#undef NEW_TYPE_ARRAY

/*
 * The GetBooleanArrayElements function is to obtain a pointer that can be directly
 * accessed in the native code (such as C or C++) for a Java boolean type array.
 * Through this pointer, the native code can read and write the elements of
 * the Java boolean array just like operating on an ordinary C array.
 *
 * @isCopy：This is an optional output parameter. If a valid pointer is passed in,
 *          the function will store the information about whether the obtained array
 *          elements are a copy of the original array at the location pointed to by this pointer.
 *          If the value is JNI_TRUE, it means that a copy of the original array is returned;
 *          if it is JNI_FALSE, it means that a direct pointer to the original array is returned.
 *          If you don't need this information, you can pass in NULL.
 */
#define GET_TYPE_ARRAY_ELEMENTS(Type, raw_type) \
raw_type* JNICALL Jvmcc_Get##Type##ArrayElements(JNIEnv *env, jarray array, jboolean *isCopy) \
{ \
    jarrRef arr = (jarrRef) array; \
    if (isCopy != nullptr) \
        *isCopy = JNI_FALSE; \
    addJNIGlobalRef(arr); \
    return (raw_type *) arr->data; \
}

GET_TYPE_ARRAY_ELEMENTS(Byte, jbyte)
GET_TYPE_ARRAY_ELEMENTS(Boolean, jboolean)
GET_TYPE_ARRAY_ELEMENTS(Char, jchar)
GET_TYPE_ARRAY_ELEMENTS(Short, jshort)
GET_TYPE_ARRAY_ELEMENTS(Int, jint)
GET_TYPE_ARRAY_ELEMENTS(Long, jlong)
GET_TYPE_ARRAY_ELEMENTS(Float, jfloat)
GET_TYPE_ARRAY_ELEMENTS(Double, jdouble)

#undef GET_TYPE_ARRAY_ELEMENTS


/*
 * In JNI, when you use the GetBooleanArrayElements function to obtain a pointer to a Java
 * boolean array that can be accessed in native code, the native code can then read from and
 * write to the array elements. After the operations are completed, to avoid memory leaks and
 * synchronize any possible modifications made to the array back to the Java side, you need
 * to call the ReleaseBooleanArrayElements function to release the relevant resources.
 *
 * jbooleanArray array: A reference to a Java boolean array, which is the array object originally
 *                      passed to the GetBooleanArrayElements function.
 * jboolean *elems: The pointer to the boolean array elements obtained through the
 *                      GetBooleanArrayElements function.
 * jint mode: This parameter specifies the behavior mode when releasing the array elements. There are
 *                      several common values:
 *          JNI_COMMIT: If elems is a copy of the original array, the modifications in the copy are
 *                      synchronized back to the original array, but the memory of the copy is not released.
 *                      You can continue to use the elems pointer later.
 *          JNI_ABORT: If elems is a copy of the original array, the copy is directly discarded, and the
 *                      modifications in the copy are not synchronized back to the original array. The memory
 *                      of the copy is released.
 *          0: If elems is a copy of the original array, the modifications in the copy are synchronized back
 *                      to the original array, and the memory of the copy is released. If elems is a direct
 *                      pointer to the original array, this parameter has no additional effect, and it only
 *                      unlocks the array.
 */
template<typename T>
void JNICALL Jvmcc_ReleaseTypeArrayElements(JNIEnv *env, jarray array, T *elems, jint mode) {
    deleteJNIGlobalRef((jref) array);
}

template<typename T>
void JNICALL Jvmcc_GetTypeArrayRegion(JNIEnv *env,
                                      jarray array, jsize start, jsize len, T *buf) {
    (void) env;
    assert(array != nullptr);
    assert(buf != nullptr);
    auto arr = (jarrRef) array;
    assert(start + len <= arr->arr_len);
    //assert(reinterpret_cast<ArrayClass *>(arr)->get_element_size() == sizeof(T));
    memcpy(buf, (arr)->index(start), len*sizeof(T));
}

template<typename T>
void JNICALL Jvmcc_SetTypeArrayRegion(JNIEnv *env,
                                      jarray array, jsize start, jsize len, const T *buf) {
    (void) env;
    assert(array != nullptr);
    assert(buf != nullptr);
    auto arr = (jarrRef) array;
    assert(start + len <= arr->arr_len);
    //assert(reinterpret_cast<ArrayClass *>(arr)->get_element_size() == sizeof(T));
    memcpy((arr)->index(start), buf, len*sizeof(T));
}

jint JNICALL Jvmcc_RegisterNatives(JNIEnv *env, jclass clazz,
                                const JNINativeMethod *methods, jint methods_count) {
    Class *c = JVM_MIRROR(clazz);
    for (jint i = 0; i < methods_count; i++) {
        Method *m = c->get_method(methods[i].name, methods[i].signature);
        if (m == nullptr) {
            // 在类中没有找到函数'm'，那么也就无需为它注册native方法了
            // 直接忽略就好了。
            continue;
        }
        if (!m->access_flags.is_native()) {
            UNREACHABLE("%s, %s, %s", c->name, m->name, m->signature);
        }
        m->native_method = methods[i].fnPtr;
    }

    return JNI_OK;
}

jint JNICALL Jvmcc_UnregisterNatives(JNIEnv *env, jclass clazz) {
    Class *c = JVM_MIRROR(clazz);
    for (Method *m: c->methods) {
        m->native_method = nullptr;
    }
    return JNI_OK;
}

jint JNICALL Jvmcc_MonitorEnter(JNIEnv *env, jobject obj) {
    // todo
//    jref o = to_object_ref(obj);
//    o->lock();
// return JNI_OK;
    unimplemented
}

jint JNICALL Jvmcc_MonitorExit(JNIEnv *env, jobject obj) {
    // todo
//    jref o = to_object_ref(obj);
//    o->unlock();
// return JNI_OK;
    unimplemented
}

static JavaVM java_vm;

jint JNICALL Jvmcc_GetJavaVM(JNIEnv *env, JavaVM **vm) {
    *vm = &java_vm;
    return JNI_OK;
}

void JNICALL Jvmcc_GetStringRegion(JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf) {
    unicode_t *u = java_lang_String::to_unicode((jstrRef) str);
    memcpy(buf, u + start, len * sizeof(unicode_t));
    buf[len] = 0;
}

void JNICALL Jvmcc_GetStringUTFRegion(JNIEnv *env, jstring str, jsize start, jsize len, char *buf) {
    utf8_t *u = java_lang_String::to_utf8((jstrRef)str);
    strncpy(buf, u + start, len);
    buf[len] = 0;
}

void* JNICALL Jvmcc_GetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy) {
    auto arr = (jarrRef) array;

    if(isCopy != nullptr)
        *isCopy = JNI_FALSE;

    /* Pin the array */ //  todo
    addJNIGlobalRef(arr);
    return arr->data;
}

void JNICALL Jvmcc_ReleasePrimitiveArrayCritical(JNIEnv *env,
                                                 jarray array, void *carray, jint mode) {
    // todo 'mode'
    deleteJNIGlobalRef((jref) array);
}

const jchar* JNICALL Jvmcc_GetStringCritical(JNIEnv *env,
                                             jstring string, jboolean *isCopy) {
    return Jvmcc_GetStringChars(env, string, isCopy);
}

void JNICALL Jvmcc_ReleaseStringCritical(JNIEnv *env,
                                         jstring string, const jchar *cstring) {
    Jvmcc_ReleaseStringChars(env, string, cstring);
}

jweak JNICALL Jvmcc_NewWeakGlobalRef(JNIEnv *env, jobject obj) {
    unimplemented
}

void JNICALL Jvmcc_DeleteWeakGlobalRef(JNIEnv *env, jweak ref) {
    unimplemented
}

jboolean JNICALL Jvmcc_ExceptionCheck(JNIEnv *env) {
    Thread *t = get_current_thread();
    return t->jniExceptionOccurred() != nullptr;
}

jobject JNICALL Jvmcc_NewDirectByteBuffer(JNIEnv *env, void *address, jlong capacity) {
    jref buffer = Allocator::object(nio_DirectByteBuffer_class);

    slot_t args[3];
    set<jlong>(args, (jlong)(uintptr_t) address);
    set<jlong>(args + 2, (jint) capacity);
    execJava(nio_DirectByteBuffer_init_method, args);

    return (jobject) buffer;
}

void* JNICALL Jvmcc_GetDirectBufferAddress(JNIEnv *env, jobject buf) {
    jref b = (jref) buf;
    return (void *) (uintptr_t) b->get_field_value<jlong>(nio_Buffer_address_id);
}

jlong JNICALL Jvmcc_GetDirectBufferCapacity(JNIEnv *env, jobject buf) {
    jref b = (jref) buf;
    return b->get_field_value<jint>(nio_Buffer_capacity_id);
}

jobjectRefType JNICALL Jvmcc_GetObjectRefType(JNIEnv* env, jobject obj) {
    // jref o = to_object_ref(obj);
    // return o != nullptr ? o->jni_obj_ref_type : JNIInvalidRefType;
    unimplemented
}

jobject JNICALL Jvmcc_GetModule(JNIEnv* env, jclass clazz) {
    unimplemented
}

static struct JNINativeInterface_ Jvmcc_JNINativeInterface = {
    .reserved0 = nullptr,
    .reserved1 = nullptr,
    .reserved2 = nullptr,
    .reserved3 = nullptr,

    .GetVersion = Jvmcc_GetVersion,

    .DefineClass = Jvmcc_DefineClass,
    .FindClass = Jvmcc_FindClass,

    .FromReflectedMethod = Jvmcc_FromReflectedMethod,
    .FromReflectedField = Jvmcc_FromReflectedField,

    .ToReflectedMethod = Jvmcc_ToReflectedMethod,

    .GetSuperclass = Jvmcc_GetSuperclass,
    .IsAssignableFrom = Jvmcc_IsAssignableFrom,

    .ToReflectedField = Jvmcc_ToReflectedField,

    .Throw = Jvmcc_Throw,
    .ThrowNew = Jvmcc_ThrowNew,
    .ExceptionOccurred = Jvmcc_ExceptionOccurred,
    .ExceptionDescribe = Jvmcc_ExceptionDescribe,
    .ExceptionClear = Jvmcc_ExceptionClear,
    .FatalError = Jvmcc_FatalError,

    .PushLocalFrame = Jvmcc_PushLocalFrame,
    .PopLocalFrame = Jvmcc_PopLocalFrame,

    .NewGlobalRef = Jvmcc_NewGlobalRef,
    .DeleteGlobalRef = Jvmcc_DeleteGlobalRef,
    .DeleteLocalRef = Jvmcc_DeleteLocalRef,
    .IsSameObject = Jvmcc_IsSameObject,
    .NewLocalRef = Jvmcc_NewLocalRef,
    .EnsureLocalCapacity = Jvmcc_EnsureLocalCapacity,

    .AllocObject = Jvmcc_AllocObject,
    .NewObject = Jvmcc_NewObject,
    .NewObjectV = Jvmcc_NewObjectV,
    .NewObjectA = Jvmcc_NewObjectA,

    .GetObjectClass = Jvmcc_GetObjectClass,
    .IsInstanceOf = Jvmcc_IsInstanceOf,

    .GetMethodID = Jvmcc_GetMethodID,

    .CallObjectMethod = Jvmcc_CallObjectMethod,
    .CallObjectMethodV = Jvmcc_CallObjectMethodV,
    .CallObjectMethodA = Jvmcc_CallObjectMethodA,

    .CallBooleanMethod = Jvmcc_CallBooleanMethod,
    .CallBooleanMethodV = Jvmcc_CallBooleanMethodV,
    .CallBooleanMethodA = Jvmcc_CallBooleanMethodA,

    .CallByteMethod = Jvmcc_CallByteMethod,
    .CallByteMethodV = Jvmcc_CallByteMethodV,
    .CallByteMethodA = Jvmcc_CallByteMethodA,

    .CallCharMethod = Jvmcc_CallCharMethod,
    .CallCharMethodV = Jvmcc_CallCharMethodV,
    .CallCharMethodA = Jvmcc_CallCharMethodA,

    .CallShortMethod = Jvmcc_CallShortMethod,
    .CallShortMethodV = Jvmcc_CallShortMethodV,
    .CallShortMethodA = Jvmcc_CallShortMethodA,

    .CallIntMethod = Jvmcc_CallIntMethod,
    .CallIntMethodV = Jvmcc_CallIntMethodV,
    .CallIntMethodA = Jvmcc_CallIntMethodA,

    .CallLongMethod = Jvmcc_CallLongMethod,
    .CallLongMethodV = Jvmcc_CallLongMethodV,
    .CallLongMethodA = Jvmcc_CallLongMethodA,

    .CallFloatMethod = Jvmcc_CallFloatMethod,
    .CallFloatMethodV = Jvmcc_CallFloatMethodV,
    .CallFloatMethodA = Jvmcc_CallFloatMethodA,

    .CallDoubleMethod = Jvmcc_CallDoubleMethod,
    .CallDoubleMethodV = Jvmcc_CallDoubleMethodV,
    .CallDoubleMethodA = Jvmcc_CallDoubleMethodA,

    .CallVoidMethod = Jvmcc_CallVoidMethod,
    .CallVoidMethodV = Jvmcc_CallVoidMethodV,
    .CallVoidMethodA = Jvmcc_CallVoidMethodA,

    .CallNonvirtualObjectMethod = Jvmcc_CallNonvirtualObjectMethod,
    .CallNonvirtualObjectMethodV = Jvmcc_CallNonvirtualObjectMethodV,
    .CallNonvirtualObjectMethodA = Jvmcc_CallNonvirtualObjectMethodA,

    .CallNonvirtualBooleanMethod = Jvmcc_CallNonvirtualBooleanMethod,
    .CallNonvirtualBooleanMethodV = Jvmcc_CallNonvirtualBooleanMethodV,
    .CallNonvirtualBooleanMethodA = Jvmcc_CallNonvirtualBooleanMethodA,

    .CallNonvirtualByteMethod = Jvmcc_CallNonvirtualByteMethod,
    .CallNonvirtualByteMethodV = Jvmcc_CallNonvirtualByteMethodV,
    .CallNonvirtualByteMethodA = Jvmcc_CallNonvirtualByteMethodA,

    .CallNonvirtualCharMethod = Jvmcc_CallNonvirtualCharMethod,
    .CallNonvirtualCharMethodV = Jvmcc_CallNonvirtualCharMethodV,
    .CallNonvirtualCharMethodA = Jvmcc_CallNonvirtualCharMethodA,

    .CallNonvirtualShortMethod = Jvmcc_CallNonvirtualShortMethod,
    .CallNonvirtualShortMethodV = Jvmcc_CallNonvirtualShortMethodV,
    .CallNonvirtualShortMethodA = Jvmcc_CallNonvirtualShortMethodA,

    .CallNonvirtualIntMethod = Jvmcc_CallNonvirtualIntMethod,
    .CallNonvirtualIntMethodV = Jvmcc_CallNonvirtualIntMethodV,
    .CallNonvirtualIntMethodA = Jvmcc_CallNonvirtualIntMethodA,

    .CallNonvirtualLongMethod = Jvmcc_CallNonvirtualLongMethod,
    .CallNonvirtualLongMethodV = Jvmcc_CallNonvirtualLongMethodV,
    .CallNonvirtualLongMethodA = Jvmcc_CallNonvirtualLongMethodA,

    .CallNonvirtualFloatMethod = Jvmcc_CallNonvirtualFloatMethod,
    .CallNonvirtualFloatMethodV = Jvmcc_CallNonvirtualFloatMethodV,
    .CallNonvirtualFloatMethodA = Jvmcc_CallNonvirtualFloatMethodA,

    .CallNonvirtualDoubleMethod = Jvmcc_CallNonvirtualDoubleMethod,
    .CallNonvirtualDoubleMethodV = Jvmcc_CallNonvirtualDoubleMethodV,
    .CallNonvirtualDoubleMethodA = Jvmcc_CallNonvirtualDoubleMethodA,

    .CallNonvirtualVoidMethod = Jvmcc_CallNonvirtualVoidMethod,
    .CallNonvirtualVoidMethodV = Jvmcc_CallNonvirtualVoidMethodV,
    .CallNonvirtualVoidMethodA = Jvmcc_CallNonvirtualVoidMethodA,

    .GetFieldID = Jvmcc_GetFieldID,

    .GetObjectField = Jvmcc_GetObjectField,
    .GetBooleanField = Jvmcc_GetBoolField,
    .GetByteField = Jvmcc_GetByteField,
    .GetCharField = Jvmcc_GetCharField,
    .GetShortField = Jvmcc_GetShortField,
    .GetIntField = Jvmcc_GetIntField,
    .GetLongField = Jvmcc_GetLongField,
    .GetFloatField = Jvmcc_GetFloatField,
    .GetDoubleField = Jvmcc_GetDoubleField,

    .SetObjectField = Jvmcc_SetObjectField,
    .SetBooleanField = Jvmcc_SetBoolField,
    .SetByteField = Jvmcc_SetByteField,
    .SetCharField = Jvmcc_SetCharField,
    .SetShortField = Jvmcc_SetShortField,
    .SetIntField = Jvmcc_SetIntField,
    .SetLongField = Jvmcc_SetLongField,
    .SetFloatField = Jvmcc_SetFloatField,
    .SetDoubleField = Jvmcc_SetDoubleField,

    .GetStaticMethodID = Jvmcc_GetStaticMethodID,

    .CallStaticObjectMethod = Jvmcc_CallStaticObjectMethod,
    .CallStaticObjectMethodV = Jvmcc_CallStaticObjectMethodV,
    .CallStaticObjectMethodA = Jvmcc_CallStaticObjectMethodA,

    .CallStaticBooleanMethod = Jvmcc_CallStaticBooleanMethod,
    .CallStaticBooleanMethodV = Jvmcc_CallStaticBooleanMethodV,
    .CallStaticBooleanMethodA = Jvmcc_CallStaticBooleanMethodA,

    .CallStaticByteMethod = Jvmcc_CallStaticByteMethod,
    .CallStaticByteMethodV = Jvmcc_CallStaticByteMethodV,
    .CallStaticByteMethodA = Jvmcc_CallStaticByteMethodA,

    .CallStaticCharMethod = Jvmcc_CallStaticCharMethod,
    .CallStaticCharMethodV = Jvmcc_CallStaticCharMethodV,
    .CallStaticCharMethodA = Jvmcc_CallStaticCharMethodA,

    .CallStaticShortMethod = Jvmcc_CallStaticShortMethod,
    .CallStaticShortMethodV = Jvmcc_CallStaticShortMethodV,
    .CallStaticShortMethodA = Jvmcc_CallStaticShortMethodA,

    .CallStaticIntMethod = Jvmcc_CallStaticIntMethod,
    .CallStaticIntMethodV = Jvmcc_CallStaticIntMethodV,
    .CallStaticIntMethodA = Jvmcc_CallStaticIntMethodA,

    .CallStaticLongMethod = Jvmcc_CallStaticLongMethod,
    .CallStaticLongMethodV = Jvmcc_CallStaticLongMethodV,
    .CallStaticLongMethodA = Jvmcc_CallStaticLongMethodA,

    .CallStaticFloatMethod = Jvmcc_CallStaticFloatMethod,
    .CallStaticFloatMethodV = Jvmcc_CallStaticFloatMethodV,
    .CallStaticFloatMethodA = Jvmcc_CallStaticFloatMethodA,

    .CallStaticDoubleMethod = Jvmcc_CallStaticDoubleMethod,
    .CallStaticDoubleMethodV = Jvmcc_CallStaticDoubleMethodV,
    .CallStaticDoubleMethodA = Jvmcc_CallStaticDoubleMethodA,

    .CallStaticVoidMethod = Jvmcc_CallStaticVoidMethod,
    .CallStaticVoidMethodV = Jvmcc_CallStaticVoidMethodV,
    .CallStaticVoidMethodA = Jvmcc_CallStaticVoidMethodA,

    .GetStaticFieldID = Jvmcc_GetStaticFieldID,
    .GetStaticObjectField = Jvmcc_GetStaticObjectField,
    .GetStaticBooleanField = Jvmcc_GetStaticBoolField,
    .GetStaticByteField = Jvmcc_GetStaticByteField,
    .GetStaticCharField = Jvmcc_GetStaticCharField,
    .GetStaticShortField = Jvmcc_GetStaticShortField,
    .GetStaticIntField = Jvmcc_GetStaticIntField,
    .GetStaticLongField = Jvmcc_GetStaticLongField,
    .GetStaticFloatField = Jvmcc_GetStaticFloatField,
    .GetStaticDoubleField = Jvmcc_GetStaticDoubleField,

    .SetStaticObjectField = Jvmcc_SetStaticObjectField,
    .SetStaticBooleanField = Jvmcc_SetStaticBoolField,
    .SetStaticByteField = Jvmcc_SetStaticByteField,
    .SetStaticCharField = Jvmcc_SetStaticCharField,
    .SetStaticShortField = Jvmcc_SetStaticShortField,
    .SetStaticIntField = Jvmcc_SetStaticIntField,
    .SetStaticLongField = Jvmcc_SetStaticLongField,
    .SetStaticFloatField = Jvmcc_SetStaticFloatField,
    .SetStaticDoubleField = Jvmcc_SetStaticDoubleField,

    .NewString = Jvmcc_NewString,
    .GetStringLength = Jvmcc_GetStringLength,
    .GetStringChars = Jvmcc_GetStringChars,
    .ReleaseStringChars = Jvmcc_ReleaseStringChars,

    .NewStringUTF = Jvmcc_NewStringUTF,
    .GetStringUTFLength = Jvmcc_GetStringUTFLength,
    .GetStringUTFChars = Jvmcc_GetStringUTFChars,
    .ReleaseStringUTFChars = Jvmcc_ReleaseStringUTFChars,

    .GetArrayLength = Jvmcc_GetArrayLength,

    .NewObjectArray = Jvmcc_NewObjectArray,
    .GetObjectArrayElement = Jvmcc_GetObjectArrayElement,
    .SetObjectArrayElement = Jvmcc_SetObjectArrayElement,
    .NewBooleanArray = Jvmcc_NewBooleanArray,
    .NewByteArray = Jvmcc_NewByteArray,
    .NewCharArray = Jvmcc_NewCharArray,
    .NewShortArray = Jvmcc_NewShortArray,
    .NewIntArray = Jvmcc_NewIntArray,
    .NewLongArray = Jvmcc_NewLongArray,
    .NewFloatArray = Jvmcc_NewFloatArray,
    .NewDoubleArray = Jvmcc_NewDoubleArray,
    .GetBooleanArrayElements = Jvmcc_GetBooleanArrayElements,
    .GetByteArrayElements = Jvmcc_GetByteArrayElements, // Jvmcc_GetTypeArrayElements<jbyte>,
    .GetCharArrayElements = Jvmcc_GetCharArrayElements,
    .GetShortArrayElements = Jvmcc_GetShortArrayElements,
    .GetIntArrayElements = Jvmcc_GetIntArrayElements,
    .GetLongArrayElements = Jvmcc_GetLongArrayElements,
    .GetFloatArrayElements = Jvmcc_GetFloatArrayElements,
    .GetDoubleArrayElements = Jvmcc_GetDoubleArrayElements,

    .ReleaseBooleanArrayElements = Jvmcc_ReleaseTypeArrayElements<jbool>,
    .ReleaseByteArrayElements = Jvmcc_ReleaseTypeArrayElements<jbyte>,
    .ReleaseCharArrayElements = Jvmcc_ReleaseTypeArrayElements<jchar>,
    .ReleaseShortArrayElements = Jvmcc_ReleaseTypeArrayElements<jshort>,
    .ReleaseIntArrayElements = Jvmcc_ReleaseTypeArrayElements<jint>,
    .ReleaseLongArrayElements = Jvmcc_ReleaseTypeArrayElements<jlong>,
    .ReleaseFloatArrayElements = Jvmcc_ReleaseTypeArrayElements<jfloat>,
    .ReleaseDoubleArrayElements = Jvmcc_ReleaseTypeArrayElements<jdouble>,

    .GetBooleanArrayRegion = Jvmcc_GetTypeArrayRegion<jboolean>,
    .GetByteArrayRegion = Jvmcc_GetTypeArrayRegion<jbyte>,
    .GetCharArrayRegion = Jvmcc_GetTypeArrayRegion<jchar>,
    .GetShortArrayRegion = Jvmcc_GetTypeArrayRegion<jshort >,
    .GetIntArrayRegion = Jvmcc_GetTypeArrayRegion<jint>,
    .GetLongArrayRegion = Jvmcc_GetTypeArrayRegion<jlong>,
    .GetFloatArrayRegion = Jvmcc_GetTypeArrayRegion<jfloat>,
    .GetDoubleArrayRegion = Jvmcc_GetTypeArrayRegion<jdouble>,

    .SetBooleanArrayRegion = Jvmcc_SetTypeArrayRegion<jboolean>,
    .SetByteArrayRegion = Jvmcc_SetTypeArrayRegion<jbyte>,
    .SetCharArrayRegion = Jvmcc_SetTypeArrayRegion<jchar>,
    .SetShortArrayRegion = Jvmcc_SetTypeArrayRegion<jshort>,
    .SetIntArrayRegion = Jvmcc_SetTypeArrayRegion<jint>,
    .SetLongArrayRegion = Jvmcc_SetTypeArrayRegion<jlong>,
    .SetFloatArrayRegion = Jvmcc_SetTypeArrayRegion<jfloat>,
    .SetDoubleArrayRegion = Jvmcc_SetTypeArrayRegion<jdouble>,

    .RegisterNatives = Jvmcc_RegisterNatives,
    .UnregisterNatives = Jvmcc_UnregisterNatives,

    .MonitorEnter = Jvmcc_MonitorEnter,
    .MonitorExit = Jvmcc_MonitorExit,

    .GetJavaVM = Jvmcc_GetJavaVM,

    .GetStringRegion = Jvmcc_GetStringRegion,
    .GetStringUTFRegion = Jvmcc_GetStringUTFRegion,

    .GetPrimitiveArrayCritical = Jvmcc_GetPrimitiveArrayCritical,
    .ReleasePrimitiveArrayCritical = Jvmcc_ReleasePrimitiveArrayCritical,

    .GetStringCritical = Jvmcc_GetStringCritical,
    .ReleaseStringCritical = Jvmcc_ReleaseStringCritical,

    .NewWeakGlobalRef = Jvmcc_NewWeakGlobalRef,
    .DeleteWeakGlobalRef = Jvmcc_DeleteWeakGlobalRef,

    .ExceptionCheck = Jvmcc_ExceptionCheck,

    .NewDirectByteBuffer = Jvmcc_NewDirectByteBuffer,
    .GetDirectBufferAddress = Jvmcc_GetDirectBufferAddress,
    .GetDirectBufferCapacity = Jvmcc_GetDirectBufferCapacity,

    .GetObjectRefType = Jvmcc_GetObjectRefType,

    .GetModule = Jvmcc_GetModule,
};

// -------------------------------------------------------

jint JNICALL Jvmcc_DestroyJavaVM(JavaVM *vm) {
    unimplemented  //  todo
    return JNI_OK;
}

jint JNICALL Jvmcc_AttachCurrentThread(JavaVM *vm, void **penv, void *args) {
    unimplemented  //  todo
    return JNI_OK;
}

jint JNICALL Jvmcc_DetachCurrentThread(JavaVM *vm) {
    unimplemented  //  todo
    return JNI_OK;
}

static JNIEnv jni_env;

jint JNICALL Jvmcc_GetEnv(JavaVM *vm, void **penv, jint version) {
    assert(penv != nullptr);

    if (get_current_thread() == nullptr) {
        *penv = nullptr;
        return JNI_EDETACHED;
    }

    *penv = &jni_env;
    return JNI_OK;
}

jint JNICALL Jvmcc_AttachCurrentThreadAsDaemon(JavaVM *vm, void **penv, void *args) {
    unimplemented  //  todo
    return JNI_OK;
}

const static struct JNIInvokeInterface_ Jvmcc_JNIInvokeInterface = {
    .reserved0 = nullptr,
    .reserved1 = nullptr,
    .reserved2 = nullptr,

    .DestroyJavaVM = Jvmcc_DestroyJavaVM,
    .AttachCurrentThread = Jvmcc_AttachCurrentThread,
    .DetachCurrentThread = Jvmcc_DetachCurrentThread,
    .GetEnv = Jvmcc_GetEnv,
    .AttachCurrentThreadAsDaemon = Jvmcc_AttachCurrentThreadAsDaemon,
};

// -------------------------------------------------------

jint parseJvmInitArgs(JavaVMInitArgs *vm_args, InitArgs *args) {
    // Property props[vm_args->nOptions];
    // args->commandline_props = &props[0];
#if 0
    for(int i = 0; i < vm_args->nOptions; i++) {
        char *string = vm_args->options[i].optionString;

        if(strcmp(string, "vfprintf") == 0)
            args->vfprintf = vm_args->options[i].extraInfo;

        else if(strcmp(string, "exit") == 0)
            args->exit = vm_args->options[i].extraInfo;

        else if(strcmp(string, "abort") == 0)
            args->abort = vm_args->options[i].extraInfo;

        else if(strcmp(string, "-verbose") == 0)
            args->verboseclass = true;

        else if(strncmp(string, "-verbose:", 9) == 0) {
            char *type = &string[8];

            do {
                type++;

                if(strncmp(type, "class", 5) == 0) {
                    args->verboseclass = true;
                    type += 5;
                }
                else if(strncmp(type, "gc", 2) == 0) {
                    args->verbosegc = true;
                    type += 2;
                }
                else if(strncmp(type, "jni", 3) == 0) {
                    args->verbosedll = true;
                    type += 3;
                }
            } while(*type == ',');

        } else if(!vm_args->ignoreUnrecognized) {
            ERR("Unrecognised option: %s\n", string);
            goto error;
        }
    }

    if(args->min_heap > args->max_heap) {
        ERR("Minimum heap size greater than max!\n");
        goto error;
    }
#endif

    // if(args->props_count) {
    //     args->commandline_props = sysMalloc(args->props_count *
    //                                         sizeof(Property));
    //     memcpy(args->commandline_props, &props[0], args->props_count *
    //                                                sizeof(Property));
    // }

    return JNI_OK;

//error:
//    return JNI_ERR;
}

void init_jvm(InitArgs *);

jint JNICALL JNI_CreateJavaVM(JavaVM **pvm, JNIEnv **penv, JavaVMInitArgs *args) {
    java_vm = &Jvmcc_JNIInvokeInterface;
    jni_env = &Jvmcc_JNINativeInterface;

    if (pvm != nullptr)
        *pvm = &java_vm;
    if (penv != nullptr)
        *penv = &jni_env;

    InitArgs init_args;

    if (args != nullptr) {
        if (parseJvmInitArgs(args, &init_args) == JNI_ERR)
            return JNI_ERR;
    }

    init_jvm(&init_args);
    return JNI_OK;
}

jint JNICALL JNI_GetDefaultJavaVMInitArgs(void *args) {
    // JavaVMInitArgs *vm_init_args = (JavaVMInitArgs*) args;

    // if(!isSupportedJNIVersion(vm_args->version))
    //     return JNI_EVERSION;

    return JNI_OK;
}

jint JNICALL JNI_GetCreatedJavaVMs(JavaVM **buff, jsize buff_len, jsize *num) {
    if(buff_len > 0) {
        *buff = &java_vm;
        *num = 1;
        return JNI_OK;
    }
    return JNI_ERR;
}

jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    unimplemented  //  todo

    // 好像应该返回 jni version
    // 参考
    // D:\jdk17\src\java.base\share\native\libjava\NativeLibraries.c
    // 对 JNI_OnLoad 的使用
    return JNI_OK;
}

void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    unimplemented  //  todo
}

JavaVM *getJavaVM() {
    return &java_vm;
}

JNIEnv *getJNIEnv() {
    return &jni_env;
}
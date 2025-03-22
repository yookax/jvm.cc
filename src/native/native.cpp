module;
#include "../vmdef.h"
#include "../jni.h"

module native;

import slot;
import classfile;
import class_loader;

class Frame;

static unordered_map<string, void (*)(Frame *)> all_natives;

//export void registry(const char *key, void (*method)(Frame *)) {
//    all_natives.insert(make_pair(key, method));
//}

void registry(const char *class_name, const char *method_name,
                     const char *method_descriptor, void (*method)(Frame *)) {
    string key(class_name);
    key.append("~").append(method_name).append("~").append(method_descriptor);

    all_natives.insert(make_pair(key, method));
}

void (* find_native(const char *class_name, const char *method_name, const char *method_descriptor))(Frame *) {
    string key(class_name);
    key.append("~").append(method_name).append("~").append(method_descriptor);

    auto iter = all_natives.find(key);
    if (iter == all_natives.end())
        return nullptr;
    return *(iter->second);
}

//void registry(const char *key, void (*method)(Frame *)) {
//    all_natives.insert(make_pair(key, method));
//}

using namespace slot;

#define OBJ   "Ljava/lang/Object;"

JNIEXPORT jint JNICALL JVM_IHashCode(JNIEnv *env, jobject obj);
JNIEXPORT jobject JNICALL JVM_Clone(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL JVM_MonitorWait(JNIEnv *env, jobject obj, jlong ms);
JNIEXPORT void JNICALL JVM_MonitorNotify(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL JVM_MonitorNotifyAll(JNIEnv *env, jobject obj);

void Unsafe_registerNatives(JNIEnv *, jclass);
void MethodHandleNatives_registerNatives(JNIEnv *, jclass);
void ScopedMemoryAccess_registerNatives(JNIEnv *, jclass );

#define REGISTRY(class_name, method_name, method_descriptor, method) \
    do { \
        void method(Frame *); \
        registry(class_name, method_name, method_descriptor, method); \
    } while(false)

#define REGISTRY_NOW(method) \
    do { \
        void method(); \
        method(); \
    } while(false)

void init_native() {
    Class *c = load_boot_class("java/lang/Object");
//    c->get_method("hashCode", "()I")->native_method = (void *) JVM_IHashCode;
//    c->get_method("clone", "()" OBJ)->native_method = (void *) JVM_Clone;
//    c->get_method("notifyAll", "()V")->native_method = (void *) JVM_MonitorNotifyAll;
//    c->get_method("notify", "()V")->native_method = (void *) JVM_MonitorNotify;
//    c->get_method("wait", "(J)V")->native_method = (void *) JVM_MonitorWait;

    c = load_boot_class("jdk/internal/misc/Unsafe");
    c->get_method("registerNatives", "()V")->native_method = (void *) Unsafe_registerNatives;

    c = load_boot_class("java/lang/invoke/MethodHandleNatives");
    c->get_method("registerNatives", "()V")->native_method = (void *) MethodHandleNatives_registerNatives;

    c = load_boot_class("jdk/internal/misc/ScopedMemoryAccess");
    c->get_method("registerNatives", "()V")->native_method = (void *) ScopedMemoryAccess_registerNatives;

    REGISTRY("java/lang/Class", "registerNatives", "()V", java_lang_Class_registerNatives);
    REGISTRY("java/lang/Thread", "registerNatives", "()V", java_lang_Thread_registerNatives);
    REGISTRY("java/lang/System", "registerNatives", "()V", java_lang_System_registerNatives);
    REGISTRY("jdk/internal/misc/Unsafe", "registerNatives", "()V", jdk_internal_misc_Unsafe_registerNatives);

    REGISTRY_NOW(java_lang_Object_registerNatives);
    REGISTRY_NOW(java_lang_Float_registerNatives);
    REGISTRY_NOW(java_lang_reflect_Array_registerNatives);
    REGISTRY_NOW(jdk_internal_misc_CDS_registerNatives);
}

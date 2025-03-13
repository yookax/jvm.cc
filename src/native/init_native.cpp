#include "../vmdef.h"
#include "../jni.h"

import vmstd;
import classfile;
import class_loader;

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

void init_native() {
    Class *c = load_boot_class("java/lang/Object");
    c->get_method("hashCode", "()I")->native_method = (void *) JVM_IHashCode;
    c->get_method("clone", "()" OBJ)->native_method = (void *) JVM_Clone;
    c->get_method("notifyAll", "()V")->native_method = (void *) JVM_MonitorNotifyAll;
    c->get_method("notify", "()V")->native_method = (void *) JVM_MonitorNotify;
    c->get_method("wait", "(J)V")->native_method = (void *) JVM_MonitorWait;

    c = load_boot_class("jdk/internal/misc/Unsafe");
    c->get_method("registerNatives", "()V")->native_method = (void *) Unsafe_registerNatives;

    c = load_boot_class("java/lang/invoke/MethodHandleNatives");
    c->get_method("registerNatives", "()V")->native_method = (void *) MethodHandleNatives_registerNatives;

    c = load_boot_class("jdk/internal/misc/ScopedMemoryAccess");
    c->get_method("registerNatives", "()V")->native_method = (void *) ScopedMemoryAccess_registerNatives;
}

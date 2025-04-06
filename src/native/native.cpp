module;
#include "../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import class_loader;

using namespace std;

static unordered_map<string, void (*)(Frame *)> all_natives;

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

#define REGISTRY_ONE(class_name, method_name, method_descriptor, method) \
    do { \
        void method(Frame *); \
        registry(class_name, method_name, method_descriptor, method); \
    } while(false)

#define REGISTRY_ALL(method) \
    do { \
        void method(); \
        method(); \
    } while(false)

void init_native() {
    REGISTRY_ONE("java/lang/Class", "registerNatives", "()V", java_lang_Class_registerNatives);
    REGISTRY_ONE("java/lang/ClassLoader", "registerNatives", "()V", java_lang_ClassLoader_registerNatives);
    REGISTRY_ONE("java/lang/Thread", "registerNatives", "()V", java_lang_Thread_registerNatives);
    REGISTRY_ONE("java/lang/System", "registerNatives", "()V", java_lang_System_registerNatives);
    REGISTRY_ONE("jdk/internal/misc/Unsafe", "registerNatives", "()V", jdk_internal_misc_Unsafe_registerNatives);
    REGISTRY_ONE("jdk/internal/misc/ScopedMemoryAccess", "registerNatives", "()V", jdk_internal_misc_ScopedMemoryAccess_registerNatives);
    REGISTRY_ONE("java/lang/invoke/MethodHandleNatives", "registerNatives", "()V", java_lang_invoke_MethodHandleNatives_registerNatives);

    REGISTRY_ALL(java_lang_Object_registerNatives);
    REGISTRY_ALL(java_lang_String_registerNatives);
    REGISTRY_ALL(java_lang_Float_registerNatives);
    REGISTRY_ALL(java_lang_Double_registerNatives);
    REGISTRY_ALL(java_lang_Runtime_registerNatives);
    REGISTRY_ALL(java_lang_Module_registerNatives);
    REGISTRY_ALL(java_lang_Throwable_registerNatives);
    REGISTRY_ALL(java_lang_ref_Reference_registerNatives);
    REGISTRY_ALL(java_lang_ref_Finalizer_registerNatives);
    REGISTRY_ALL(java_lang_ref_PhantomReference_registerNatives);
    REGISTRY_ALL(java_lang_reflect_Array_registerNatives);
    REGISTRY_ALL(java_lang_invoke_MethodHandle_registerNatives);
    REGISTRY_ALL(java_io_FileDescriptor_registerNatives);
    REGISTRY_ALL(java_io_FileInputStream_registerNatives);
    REGISTRY_ALL(java_io_FileOutputStream_registerNatives);
    REGISTRY_ALL(jdk_internal_misc_VM_registerNatives);
    REGISTRY_ALL(jdk_internal_misc_CDS_registerNatives);
    REGISTRY_ALL(jdk_internal_misc_Signal_registerNatives);
    REGISTRY_ALL(jdk_internal_reflect_Reflection_registerNatives);
    REGISTRY_ALL(jdk_internal_loader_BootLoader_registerNatives);
    REGISTRY_ALL(jdk_internal_loader_NativeLibraries_registerNatives);
    REGISTRY_ALL(jdk_internal_util_SystemProps$Raw_registerNatives);
    REGISTRY_ALL(jdk_internal_jimage_NativeImageBuffer_registerNatives);
    REGISTRY_ALL(sun_nio_ch_IOUtil_registerNatives);
#ifdef _WIN64
    REGISTRY_ALL(java_io_WinNTFileSystem_registerNatives);
    REGISTRY_ALL(sun_io_Win32ErrorMode_registerNatives);
    REGISTRY_ALL(sun_nio_fs_WindowsNativeDispatcher_registerNatives);
#endif
}

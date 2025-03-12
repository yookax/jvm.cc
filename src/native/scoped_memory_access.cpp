#include "../cabin.h"
#include "../jni.h"

// 这些native函数全部定义在 jdk\internal\misc\ScopedMemoryAccess.java

// native boolean closeScope0(Scope scope, Scope.ScopedAccessError exception);
static jbool closeScope(JNIEnv *env, jobject _this, jobject scope, jobject exception) {
    // todo
    return true;
}

static JNINativeMethod ScopedMemoryAccess_natives[] = {
    { "closeScope0", "(Ljdk/internal/misc/ScopedMemoryAccess$Scope;"
                     "Ljdk/internal/misc/ScopedMemoryAccess$Scope$ScopedAccessError;)Z",
                     (void *) closeScope },
};

void ScopedMemoryAccess_registerNatives(JNIEnv *env, jclass cls) {
    env->RegisterNatives(cls, ScopedMemoryAccess_natives, std::size(ScopedMemoryAccess_natives));
}

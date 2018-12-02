/*
 * Author: Jia Yang
 */

#include "../../registry.h"
#include "../../../interpreter/stack_frame.h"
#include "../../../slot.h"
#include "../../../rtda/heap/JObject.h"

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedAction<T> action);
static void doPrivileged(struct stack_frame *frame)
{
    // todo 这个函数干什么用的。。。。
    struct jobject *this_obj = slot_getr(frame->local_vars);

    /*
     * run 函数返回 T类型 的对象
     *
     * public interface PrivilegedAction<T> {
     *     T run();
     * }
     */
    struct jmethod *m = jclass_get_method(this_obj->jclass, "run", "()Ljava/lang/Object;"); // todo getInstanceMethod
    struct slot args[] = { rslot(this_obj) };
    jthread_invoke_method(frame->thread, m, args);
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedAction<T> action, AccessControlContext context);
static void doPrivileged1(struct stack_frame *frame)
{
    // todo
    doPrivileged(frame);
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedExceptionAction<T> action) throws PrivilegedActionException;
static void doPrivileged2(struct stack_frame *frame)
{
    // todo
    doPrivileged(frame);
}

// @CallerSensitive
// public static native <T> T doPrivileged(PrivilegedExceptionAction<T> action, AccessControlContext context)
//      throws PrivilegedActionException;
static void doPrivileged3(struct stack_frame *frame)
{
    // todo
    doPrivileged(frame);
}

// private static native AccessControlContext getStackAccessControlContext();
static void getStackAccessControlContext(struct stack_frame *frame)
{
    // todo
    os_pushr(frame->operand_stack, NULL);
}

void java_security_AccessController_registerNatives()
{
    register_native_method("java/security/AccessController", "registerNatives", "()V", empty_method);
    register_native_method("java/security/AccessController", "getStackAccessControlContext",
                         "()Ljava/security/AccessControlContext;", getStackAccessControlContext);

    register_native_method("java/security/AccessController", "doPrivileged",
                         "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;",
                         doPrivileged);

    register_native_method("java/security/AccessController", "doPrivileged",
                         "(Ljava/security/PrivilegedAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;",
                         doPrivileged1);

    register_native_method("java/security/AccessController", "doPrivileged",
                         "(Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object;",
                         doPrivileged2);

    register_native_method("java/security/AccessController", "doPrivileged",
                         "(Ljava/security/PrivilegedExceptionAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;",
                         doPrivileged3);

}
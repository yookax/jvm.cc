module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

/*
 * Fill in the current stack trace in this exception.  This is
 * usually called automatically when the exception is created but it
 * may also be called explicitly by the user.  This routine returns
 * `this' so you can write 'throw e.fillInStackTrace();'
 */
// private native Throwable fillInStackTrace(int dummy);
void fillInStackTrace(Frame *f0) {
    slot_t *args = f0->lvars;
    auto _this = slot::get<jref>(args);

    jref throwable = _this;
    Thread *thread = get_current_thread();

    Frame *frame = thread->top_frame;
    int num = thread->count_stack_frames();
    /*
     * 栈顶两帧正在执行 fillInStackTrace(int) 和 fillInStackTrace() 方法，所以需要跳过这两帧。
     * 这两帧下面的几帧正在执行异常类的构造函数，所以也要跳过。
     * 具体要跳过多少帧数则要看异常类的继承层次。
     *
     * (RuntimeException extends Exception extends Throwable extends Object)
     *
     * 比如一个异常抛出示例
     * java.lang.RuntimeException: BAD!
     * at exception/UncaughtTest.main(UncaughtTest.java:6)
     * at exception/UncaughtTest.foo(UncaughtTest.java:10)
     * at exception/UncaughtTest.bar(UncaughtTest.java:14)
     * at exception/UncaughtTest.bad(UncaughtTest.java:18)
     * at java/lang/RuntimeException.<init>(RuntimeException.java:62)
     * at java/lang/Exception.<init>(Exception.java:66)
     * at java/lang/Throwable.<init>(Throwable.java:265)
     * at java/lang/Throwable.fillInStackTrace(Throwable.java:783)
     * at java/lang/Throwable.fillInStackTrace(Native Method)
     */
    Frame *f = frame->prev->prev;
    num -= 2;

    for (Class *c = throwable->clazz; c != nullptr; c = c->super_class) {
        f = f->prev; // jump 执行异常类的构造函数的frame
        num--;
        if (strcmp(c->name, "java/lang/Throwable") == 0) {
            break; // 可以了，遍历到 Throwable 就行了，因为现在在执行 Throwable 的 fillInStackTrace 方法。
        }
    }

    jarrRef backtrace = Allocator::object_array(num);
    auto trace = (Object **) backtrace->data;

    Class *c = load_boot_class("java/lang/StackTraceElement");
    for (int i = 0; f != nullptr; f = f->prev) {
        Object *o = Allocator::object(c);
        assert(i < num);
        trace[i++] = o;

        // public StackTraceElement(String declaringClass, String methodName, String fileName, int lineNumber)
        // may be should call <init>, but 直接赋值 is also ok. todo

        jstrRef file_name = f->method->clazz->source_file_name != nullptr
                            ? Allocator::string(f->method->clazz->source_file_name)
                            : nullptr;
        jstrRef class_name = Allocator::string(f->method->clazz->name);
        jstrRef method_name = Allocator::string(f->method->name);

        jint line_number = f->method->get_line_number(f->reader.pc - 1); // todo why 减1？ 减去opcode的长度

        o->set_field_value<jref>("fileName", "Ljava/lang/String;", file_name);
        o->set_field_value<jref>("declaringClass", "Ljava/lang/String;", class_name);
        o->set_field_value<jref>("methodName", "Ljava/lang/String;", method_name);
        o->set_field_value<jint>("lineNumber", line_number);

        // private transient Class<?> declaringClassObject;
        o->set_field_value<jref>("declaringClassObject", "Ljava/lang/Class;", c->java_mirror);
    }

    /*
     * Native code saves some indication of the stack backtrace in this slot.
     *
     * private transient Object backtrace;
     */
    throwable->set_field_value<jref>("backtrace", "Ljava/lang/Object;", backtrace);

    /*
     * The JVM code sets the depth of the backtrace for later retrieval
     * todo test-java on jdk15
     * private transient int depth;
     */
    throwable->set_field_value<jint>("depth", backtrace->arr_len);

    f0->pushr(throwable);
}

void java_lang_Throwable_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Throwable", #method, method_descriptor, method)

    R(fillInStackTrace, "(I)Ljava/lang/Throwable;");
}
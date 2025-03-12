module;
#include <cassert>
#include "../cabin.h"
#include "../slot.h"
#include "../classfile/bytecode_reader.h"

export module runtime;

import std.core;
import std.threading;
import classfile;

export struct Frame {
    Method *method;
    BytecodeReader reader;

    /*
     * this frame 执行的函数是否由虚拟机调用
     * 由虚拟机调用的函数不会将返回值压入下层frame的栈中，
     * 也不会后续执行其下层frame，而是直接返回。
     */
    bool vm_invoke;

    Frame *prev;

    jref jni_local_ref_table[JNI_LOCAL_REFERENCE_TABLE_MAX_CAPACITY];
    int jni_local_ref_count = 0;

    slot_t *lvars;   // local variables
    slot_t *ostack;  // operand stack,指向栈顶，当前值(*ostack)无效

    Frame(Method *m, bool vm_invoke0, slot_t *lvars0, slot_t *ostack0, Frame *prev0)
            : method(m), reader(m->code, m->code_len), vm_invoke(vm_invoke0),
              prev(prev0), lvars(lvars0), ostack(ostack0)
    { assert(m != nullptr && lvars != nullptr && ostack != nullptr); }

    // push to operand stack.
    void push(slot_t v)   { *ostack++ = v; }
    void pushi(jint v)    { slot::set<jint>(ostack, v); ostack++; }
    void pushf(jfloat v)  { slot::set<jfloat>(ostack, v); ostack++; }
    void pushl(jlong v)   { slot::set<jlong>(ostack, v); ostack += 2; }
    void pushd(jdouble v) { slot::set<jdouble>(ostack, v); ostack += 2; }
    void pushr(jref v)    { slot::set<jref>(ostack, v); ostack++; }

    // pop from operand stack.
    jint    popi() { ostack--;    return slot::get<jint>(ostack); }
    jfloat  popf() { ostack--;    return slot::get<jfloat>(ostack); }
    jlong   popl() { ostack -= 2; return slot::get<jlong>(ostack); }
    jdouble popd() { ostack -= 2; return slot::get<jdouble>(ostack); }
    jref    popr() { ostack--;    return slot::get<jref>(ostack); }

    // the end address of this frame
    intptr_t end_address() const { return (intptr_t)(ostack + method->max_stack); }

    void clear_operand_stack() { ostack = (slot_t *)(this + 1); }

    std::string toString() const {
        std::ostringstream oss;
        oss << "(" << this << ")"
            << method->toString() << ", pc = " << reader.pc << std::ends;
        return oss.str();
    }
};

/*
 * jvm中所定义的线程
 *
 * 如果Java虚拟机栈有大小限制，且执行线程所需的栈空间超出了这个限制，
 * 会导致StackOverflowError异常抛出。如果Java虚拟机栈可以动态扩展，
 * 但是内存已经耗尽，会导致OutOfMemoryError异常抛出。
 */
export class Thread {
    /*
     * VM stack 中的 Frame 布局：
     * ------------------------------------------------------------------
     * |lvars|Frame|ostack|, |lvars|Frame|ostack|, |lvars|Frame|ostack| ...
     * ------------------------------------------------------------------
     */
    u1 vm_stack[VM_STACK_SIZE]; // 虚拟机栈，一个线程只有一个虚拟机栈
public:
    Frame *top_frame = nullptr;

    Object *tobj = nullptr;  // 所关联的 Object of java.lang.Thread
    std::thread::id tid;     // 所关联的 local thread 对应的id

    jbool interrupted = false;

private:
    jref jni_excep = nullptr;
public:
    static void jniThrow(jref excep);
    static void jniThrow(Class *excep_class, const char *msg);
    static jref jniExceptionOccurred();
    static void jniExceptionClear();

    // Thread *next = nullptr;

    Thread();

    void bind(Object *tobj = nullptr);

    static Thread *from(Object *tobj);
    static Thread *from(jlong tid);

    Frame *alloc_frame(Method *, bool vm_invoke);
    void pop_frame();

    int count_stack_frames() const;

    [[noreturn]] void terminate(ExitCode code);

    /*
     * return [Ljava/lang/StackTraceElement;
     * where @max_depth < 0 to request entire stack dump
     */
    jarrRef dump(int max_depth) const;
};

export std::vector<Thread *> g_all_java_thread;

export namespace java_lang_Thread {
    void start(jref tobj);
    bool isAlive(jref tobj);

    void setStatus(jref tobj, jint status);
    jint getStatus(jref tobj);

    const char *getNameUtf8(jref tobj);
    const jstrRef getName(jref tobj);
    const jlong getTid(jref tobj);
};

export Thread *g_main_thread;

export void init_thread();

export Thread *get_current_thread();
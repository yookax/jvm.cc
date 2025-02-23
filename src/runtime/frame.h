#ifndef CABIN_FRAME_H
#define CABIN_FRAME_H

#include <cassert>
#include <sstream>
#include <string>
#include "../cabin.h"
#include "../slot.h"
#include "../classfile/bytecode_reader.h"
#include "../classfile/class.h"
#include "../classfile/method.h"

struct Frame {
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

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "(" << this << ")"
            << method->toString() << ", pc = " << reader.pc << std::ends;
        return oss.str();
    }
};

#endif
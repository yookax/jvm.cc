module;
#include "vmdef.h"

export module interpreter;

import std.core;
import slot;

// Java Function Return Type
// using jfrt = std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>;

// 此方法用于虚拟机主动调用函数，数调用指令（invokestatic, invokespecial, ...）中不能使用

export slot_t *execJava(Method *, const slot_t *args = nullptr);

export jref execJavaR(Method *method, const slot_t *args = nullptr) {
    assert(method != nullptr);
    slot_t *slots = execJava(method, args);
    assert(slots != nullptr);
    return slot::get<jref>(slots);
}
//export jref execJavaR(Method *, const slot_t *args = nullptr);

// std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>
// exec_java_method(Method *method, const slot_t *args = nullptr);

//export slot_t *execJava(Method *, std::initializer_list<slot_t> args);
export slot_t *execJava(Method *method, std::initializer_list<slot_t> args) {
    assert(method != nullptr);

    auto slots = new slot_t[args.size()];
    int i = 0;
    for (slot_t arg: args) {
        slots[i++] = arg;
    }
    return execJava(method, slots);
}

export jref execJavaR(Method *m, std::initializer_list<slot_t> args) {
    assert(m != nullptr);

    auto slots = new slot_t[args.size()];
    int i = 0;
    for (slot_t arg: args) {
        slots[i++] = arg;
    }
    return execJavaR(m, slots);
}

//export jref execJavaR(Method *m, std::initializer_list<slot_t> args);

// Object[] args;
export slot_t *execJava(Method *, jref _this, jarrRef args);
export jref execJavaR(Method *, jref _this, jarrRef args);

export slot_t *call_java(jref method_obj, jref _this, jarrRef args);
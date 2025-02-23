#ifndef CABIN_INTERPRETER_H
#define CABIN_INTERPRETER_H

#include <initializer_list>
#include <variant>

#include "cabin.h"
#include "slot.h"

// Java Function Return Type
// using jfrt = std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>;

// 此方法用于虚拟机主动调用函数，数调用指令（invokestatic, invokespecial, ...）中不能使用

slot_t *execJava(Method *, const slot_t *args = nullptr);
jref execJavaR(Method *, const slot_t *args = nullptr);

// std::variant<jint, jfloat, jlong, jdouble, jref, std::monostate>
// exec_java_method(Method *method, const slot_t *args = nullptr);

slot_t *execJava(Method *, std::initializer_list<slot_t> args);
jref execJavaR(Method *m, std::initializer_list<slot_t> args);

// Object[] args;
slot_t *execJava(Method *, jref _this, jarrRef args);
jref execJavaR(Method *, jref _this, jarrRef args);

#endif // CABIN_INTERPRETER_H
#ifndef CABIN_METHOD_H
#define CABIN_METHOD_H

#include <cstring>
#include <tuple>
#include <vector>
#include <string>
#include "../cabin.h"
#include "bytecode_reader.h"
#include "meta.h"
#include "../encoding.h"
#include "poly.h"
#include "../jni.h"

struct Frame;
class ConstantPool;

/* A Method represents a Java method. */
class Method: public Meta {
public:
    enum RetType {
        RET_INVALID, RET_VOID, RET_BYTE, RET_BOOL, RET_CHAR,
        RET_SHORT, RET_INT, RET_FLOAT, RET_LONG, RET_DOUBLE, RET_REFERENCE
    };

    Class *clazz = nullptr;
    const utf8_t *descriptor = nullptr;

    int vtable_index = -1;
    int itable_index = -1;

    u2 max_stack = 0;
    u2 max_locals = 0;
    u2 arg_slots_count = 0; // include 'this' if is not static.

    u1 *code = nullptr;
    size_t code_len = 0;

    void *native_method = nullptr; // present only if native
    // present only if native
    void (* native_invoker)(void *, JNIEnv *, jref, slot_t *, Frame *) = nullptr;
    
    RetType ret_type = RET_INVALID;

    /*
     * Each value in the exception_index_table array must be a valid index into
     * the constant_pool table. The constant_pool entry at that index must be a
     * CONSTANT_Class_info structure representing a class type that this
     * method is declared to throw.
     */
    // checked exceptions the method may throw.
    std::vector<u2> checked_exceptions;

    Annotation rt_visi_para_annos;     // runtime visible parameter annotations
    Annotation rt_invisi_para_annos;   // runtime invisible parameter annotations

    Annotation annotation_default;

    // std::vector<std::vector<Annotation>> rt_visi_para_annos;     // runtime visible parameter annotations
    // std::vector<std::vector<Annotation>> rt_invisi_para_annos;   // runtime invisible parameter annotations
    // ElementValue annotation_default;

private:
    struct MethodParameter {
        u2 name_index;
        // const utf8_t *name = nullptr;
        u2 access_flags;

        explicit MethodParameter(ConstantPool &cp, BytecodeReader &r) {
            name_index = r.readu2();
            // If the value of the name_index item is zero,
            // then this parameters element indicates a formal parameter with no name.
            // if (name_index > 0) {
            //     name = cp.utf8(name_index);
            // }
            access_flags = r.readu2();
        }
    };

    std::vector<MethodParameter> parameters;

    struct LineNumberTable {
        u2 start_pc;
        u2 line_number;
        LineNumberTable *next;
    } *line_number_tables = nullptr;

    struct LocalVarTable {
        // [start_pc, end_pc), 半闭半开区间表示有效范围 `index` 的有效范围
        u2 start_pc;
        u2 end_pc;
        u2 name_index;
        union {
            u2 descriptor_index; // The LocalVariableTable Attribute 
            u2 signature_index;  // The LocalVariableTypeTable Attribute
        };
        u2 index;

        explicit LocalVarTable(BytecodeReader &r) {
            start_pc = r.readu2();
            u2 length = r.readu2();
            end_pc = start_pc + length;
            name_index = r.readu2();
            descriptor_index = r.readu2();
            index = r.readu2();
        }

        bool match(u2 pc, u2 _index) const { return start_pc <= pc && pc < end_pc && index == _index; }
    };

    std::vector<LocalVarTable> local_variable_tables;
    std::vector<LocalVarTable> local_variable_type_tables;

public:    
    // `index`: index into the local variable array
    std::pair<const utf8_t *, const utf8_t *> find_local_variable(u2 pc, u2 index);

    /*
     * 异常处理表
     * start_pc 给出的是try{}语句块的第一条指令，end_pc 给出的则是try{}语句块的下一条指令。
     * 如果 catch_type 是 NULL（在class文件中是0），表示可以处理所有异常，这是用来实现finally子句的。
     */
    struct ExceptionTable {
        u2 start_pc;
        u2 end_pc;
        u2 handler_pc;

        struct CatchType {
            bool resolved;
            union {
                Class *clazz; // if resolved
                const char *class_name; // if not resolved
            };
        } *catch_type = nullptr;

        ExceptionTable(Class *clazz, BytecodeReader &r);
    };

    std::vector<ExceptionTable> exception_tables;

private:
    // 用来创建 polymorphic method
    Method(Class *c, const utf8_t *name, const utf8_t *descriptor, int acc, void *native_method);

    void parse_code_attr(BytecodeReader &r);
    void determine_ret_type();
    void gen_native_method_info();

public:
    Method(Class *c, BytecodeReader &r);

    bool is_abstract() const     { return accIsAbstract(access_flags); }
    bool is_synchronized() const { return accIsSynchronized(access_flags); }
    bool is_native() const       { return accIsNative(access_flags); }
    bool is_strict() const       { return accIsStrict(access_flags); }
    bool is_varargs() const      { return accIsVarargs(access_flags); }

    /*
     * 判断此方法是否由 invokevirtual 指令调用，
     * final方法虽然非虚，但也由 invokevirtual 调用。
     * todo
     * 一个 final non-private 方法则可以覆写基类的虚方法，并且可以被基类引用通过invokevirtual调用到。
     * 参考 https://www.zhihu.com/question/45131640
     */
    [[nodiscard]] bool is_virtual() const {
        return !isPrivate() && !isStatic() && !utf8::equals(name, "<init>");
    }

    // is <clinit>?
    [[nodiscard]] bool is_class_init() const {
        return strcmp(name, "<clinit>") == 0;
    }

    // is <init>?
    [[nodiscard]] bool is_object_init() const {
        return strcmp(name, "<init>") == 0;
    }
    
    static u2 cal_args_slots_count(const utf8_t *descriptor, bool is_static);

    void cal_args_slots_count() { arg_slots_count = cal_args_slots_count(descriptor, isStatic()); }

    // [Ljava/lang/Class;
    jarrRef get_parameter_types();

    jclsRef get_return_type();

    // [Ljava/lang/Class;
    jarrRef get_exception_types();

    jint get_line_number(int pc) const;

    /*
     * @pc, 发生异常的位置
     */
    int find_exception_handler(Class *exception_type, size_t pc);

    bool is_signature_polymorphic() {
        std::tuple<Class *, int, void *> p = lookup_polymorphic_method(clazz, name);
        return std::get<0>(p) != nullptr;
    }

    std::string toString() const;

    // 此方法用于测试
    // 对于隐藏类，匿名类可以通过此方法查看其字节码
    std::string get_bytecode_string() const;

    friend class Class;
};

#endif
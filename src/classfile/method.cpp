module;
#include "../vmdef.h"
#include "../jni.h"

module classfile;

import std.core;
import object;
import class_loader;

using namespace std;

Method::ExceptionTable::ExceptionTable(Class *clazz, BytecodeReader &r) {
    start_pc = r.readu2();
    end_pc = r.readu2();
    handler_pc = r.readu2();
    u2 index = r.readu2();
    if (index == 0) {
        // 异常处理项的 catch_type 有可能是 0。
        // 0 是无效的常量池索引，但是在这里 0 并非表示 catch-none，而是表示 catch-all。
        catch_type = nullptr;
    } else {
        catch_type = new CatchType;
        if (clazz->cp->get_type(index) == JVM_CONSTANT_ResolvedClass) {
            catch_type->resolved = true;
            catch_type->clazz = clazz->cp->resolve_class(index);
        } else {
            // Note:
            // 不能在这里load class，有形成死循环的可能。
            // 比如当前方法是 Throwable 中的方法，而此方法又抛出了 Throwable 子类的Exception（记为A），
            // 而此时 Throwable 还没有构造完成，所以无法构造其子类 A。
            catch_type->resolved = false;
            catch_type->class_name = clazz->cp->class_name(index);
        }
    }
}

jarrRef Method::get_parameter_types() {
    pair<jarrRef, jclsRef> p = parseMethodDescriptor(descriptor, clazz->loader);
    return p.first;
}

jclsRef Method::get_return_type() {
    pair<jarrRef, jclsRef> p = parseMethodDescriptor(descriptor, clazz->loader);
    return p.second;
}

jarrRef Method::get_exception_types() {
    jint count = (jint) checked_exceptions.size();
    auto ac = (ArrayClass *) loadClass(clazz->loader, "[Ljava/lang/Class;");
    jarrRef exception_types = Allocator::array(ac, count);

    for (int i = 0; i < count; i++) {
        Class *c = clazz->cp->resolve_class(checked_exceptions[i]);
        exception_types->setRefElt(i, c->java_mirror);
    }

    return exception_types;
}

u2 Method::cal_args_slots_count(const utf8_t *descriptor, bool is_static) {
    assert(descriptor != nullptr);
    // u2 count = 0;

    // const char *b = strchr(descriptor, '(');
    // const char *e = strchr(descriptor, ')');
    // if (b == nullptr || e == nullptr) {
    //     goto error;
    // }

    // while (++b < e) {
    //     if (*b == 'B' || *b == 'C' || *b == 'I' || *b == 'F' || *b == 'S'|| *b == 'Z'/* boolean */) {
    //         count++;
    //     } else if (*b == 'D' || *b == 'J'/* long */) {
    //         count += 2;
    //     } else if (*b == 'L') { // reference
    //         count++;
    //         b = strchr(b, ';');
    //         if (b == nullptr) {
    //             goto error;
    //         }
    //     } else if (*b == '[') { // array reference
    //         count++;
    //         while (*(++b) == '[');

    //         if (*b == 'L') {
    //             b = strchr(b, ';');
    //             if (b == nullptr) {
    //                 goto error;
    //             }
    //         }
    //     }
    // }

    int count = numSlotsInMethodDescriptor(descriptor);

    if (!is_static) { // note: 构造函数（<init>方法）是非static的，也会传递this reference  todo
        count++; // this reference
    }
    return (u2) count;
}

/*
 * 解析方法的 code 属性
 */
void Method::parse_code_attr(BytecodeReader &r) {
    max_stack = r.readu2();
    max_locals = r.readu2();
    code_len = r.readu4();
    // m->code = r.currPos();
    code = new u1[code_len];
    memcpy(code, r.curr_pos(), code_len);
    r.skip(code_len);

    // parse exception tables
    int exception_tables_count = r.readu2();
    for (int i = 0; i < exception_tables_count; i++) {
        exception_tables.emplace_back(clazz, r);
    }

    // parse attributes of code's attribute
    u2 attr_count = r.readu2();
    for (int k = 0; k < attr_count; k++) {
        const char *attr_name = clazz->cp->utf8(r.readu2());
        u4 attr_len = r.readu4();

        if (strcmp("LineNumberTable", attr_name) == 0) {
            u2 num = r.readu2();
            for (int i = 0; i < num; i++) {
                auto t = new LineNumberTable();
                t->start_pc = r.readu2();
                t->line_number = r.readu2();
                t->next = line_number_tables;
                line_number_tables = t;
            }
        } else if (strcmp("StackMapTable", attr_name) == 0) {
            r.skip(attr_len); // todo ....
        } else if (strcmp("LocalVariableTable", attr_name) == 0) {
            u2 num = r.readu2();
            for (int i = 0; i < num; i++) 
                local_variable_tables.emplace_back(r);
        } else if (strcmp("LocalVariableTypeTable", attr_name) == 0) {
            u2 num = r.readu2();
            for (int i = 0; i < num; i++)
                local_variable_type_tables.emplace_back(r);
        } else { // unknown attribute
            WARN("unknown attribute: %s\n", attr_name);
            r.skip(attr_len);
        }
    }
}

void Method::determine_ret_type() {
    const char *t = strchr(descriptor, ')'); // find return
    assert(t != nullptr);
    t++;
    if (*t == 'V') {
        ret_type = RET_VOID;
    } else if (*t == 'D') {
        ret_type = RET_DOUBLE;
    } else if (*t == 'F') {
        ret_type = RET_FLOAT;
    } else if (*t == 'J') {
        ret_type = RET_LONG;
    } else if (*t == 'L' || *t == '[') {
        ret_type = RET_REFERENCE;
    } else  if (*t == 'I') {
        ret_type = RET_INT;
    } else  if (*t == 'B') {
        ret_type = RET_BYTE;
    } else  if (*t == 'Z') {
        ret_type = RET_BOOL;
    } else  if (*t == 'C') {
        ret_type = RET_CHAR;
    } else  if (*t == 'S') {
        ret_type = RET_SHORT;
    } else {
        UNREACHABLE("Illegal method descriptor: %s\n", descriptor);
    }
}

void Method::gen_native_method_info() {
    // 本地方法帧的操作数栈至少要能容纳返回值，
    // 4 slots are big enough.
    max_stack = 4;
    // 因为本地方法帧的局部变量表只用来存放参数值，
    // 所以把arg_slot_count赋给max_locals字段刚好。
    max_locals = arg_slots_count;

    code_len = 2;
    code = new u1[code_len];
    code[0] = JVM_OPC_invokenative;

    if (ret_type == RET_VOID) {
        code[1] = JVM_OPC_return;
    } else if (ret_type == RET_DOUBLE) {
        code[1] = JVM_OPC_dreturn;
    } else if (ret_type == RET_FLOAT) {
        code[1] = JVM_OPC_freturn;
    } else if (ret_type == RET_LONG) {
        code[1] = JVM_OPC_lreturn;
    } else if (ret_type == RET_REFERENCE) {
        code[1] = JVM_OPC_areturn;
    } else {
        code[1] = JVM_OPC_ireturn;
    }
}

Method::Method(Class *c, BytecodeReader &r) {
    assert(c != nullptr);

    clazz = c;
    ConstantPool &cp = *(c->cp);

    access_flags = r.readu2();
    name = cp.utf8(r.readu2());
    descriptor = cp.utf8(r.readu2());
    u2 attr_count = r.readu2();

    // note: 构造函数（<init>方法）是非static的，也会传递this reference  todo
    cal_args_slots_count();

    // parse method's attributes
    for (u2 i = 0; i < attr_count; i++) {
        const char *attr_name = cp.utf8(r.readu2());
        u4 attr_len = r.readu4();

        if (strcmp("Code", attr_name) == 0) {
            parse_code_attr(r);
        } else if (strcmp("Deprecated", attr_name) == 0) {
            deprecated = true;
        } else if (strcmp("Synthetic", attr_name) == 0) {
            setSynthetic();
        } else if (strcmp("Signature", attr_name) == 0) {
            signature = cp.utf8(r.readu2());
        } else if (strcmp("MethodParameters", attr_name) == 0) {
            u1 num = r.readu1(); // 这里就是 u1，不是u2
            for (u2 k = 0; k < num; k++)
                parameters.emplace_back(cp, r);
        } else if (strcmp("Exceptions", attr_name) == 0) {
            u2 num = r.readu2();
            for (u2 j = 0; j < num; j++)
                checked_exceptions.push_back(r.readu2());
        } else if (strcmp("RuntimeVisibleParameterAnnotations", attr_name) == 0) {
            rt_visi_para_annos.parse(r, attr_len);
            // parseAnnos(rt_visi_para_annos, r, attr_len);
            // u2 num = r.readu2();
            // rt_visi_para_annos.resize(num);
            // for (u2 j = 0; j < num; j++) {
            //     u2 num_annos = r.readu2();
            //     for (u2 k = 0; k < num_annos; k++)
            //         rt_visi_para_annos[j].emplace_back(r);
            // }
        } else if (strcmp("RuntimeInvisibleParameterAnnotations", attr_name) == 0) {
            rt_invisi_para_annos.parse(r, attr_len);
            // parseAnnos(rt_invisi_para_annos, r, attr_len);
            // u2 num = r.readu2();
            // rt_invisi_para_annos.resize(num);
            // for (u2 j = 0; j < num; j++) {
            //     u2 num_annos = r.readu2();
            //     for (u2 k = 0; k < num_annos; k++)
            //         rt_invisi_para_annos[j].emplace_back(r);
            // }        
        } else if (strcmp("RuntimeVisibleAnnotations", attr_name) == 0) {
            rt_visi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleAnnotations", attr_name) == 0) {
            rt_invisi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeVisibleTypeAnnotations", attr_name) == 0) {
            rt_visi_type_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleTypeAnnotations", attr_name) == 0) {
            rt_invisi_type_annos.parse(r, attr_len);
        } else if (strcmp("AnnotationDefault", attr_name) == 0) {
            annotation_default.parse(r, attr_len);
            //parseAnnos(annotation_default, r, attr_len);
            // annotation_default.read(r);
        } else { // unknown attribute
            WARN("unknown attribute: %s\n", attr_name);
            r.skip(attr_len);
        }
    }

    determine_ret_type();

    if (is_native()) {        
        gen_native_method_info();
    }
}

Method::Method(Class *c, const utf8_t *name0, const utf8_t *descriptor0, int acc, void *native_method0) {
    assert(c != nullptr && name0 != nullptr && descriptor0 != nullptr);
    assert(acc > 0 && native_method0 != nullptr);

    clazz = c;
    access_flags = acc;
    name = name0;
    descriptor = descriptor0;
    native_method = native_method0;

    // note: 构造函数（<init>方法）是非static的，也会传递this reference  todo
    cal_args_slots_count();
    determine_ret_type();
    gen_native_method_info();
}

jint Method::get_line_number(int pc) const {
    // native函数没有字节码
    if (is_native()) {
        return -2;
    }

    /*
     * 和源文件名一样，并不是每个方法都有行号表。
     * 如果方法没有行号表，自然也就查不到pc对应的行号，这种情况下返回–1
     todo
     */
    for (LineNumberTable *t = line_number_tables; t != nullptr; t = t->next) {
        if (pc >= t->start_pc)
            return t->line_number;
    }
    return -1;
}

int Method::find_exception_handler(Class *exceptionType, size_t pc) {
    for (auto t : exception_tables) {
        // jvms: The start pc is inclusive and end pc is exclusive
        if (t.start_pc <= pc && pc < t.end_pc) {
            if (t.catch_type == nullptr)  // catch all
                return t.handler_pc;
            if (!t.catch_type->resolved) {
                t.catch_type->clazz = loadClass(clazz->loader, t.catch_type->class_name);
                t.catch_type->resolved = true;
            }
            if (exceptionType->is_subclass_of(t.catch_type->clazz))
                return t.handler_pc;
        }
    }

    return -1;
}

// bool Method::isSignaturePolymorphic() 
// {
//     bool b = utf8::equals(clazz->name, "java/lang/invoke/MethodHandle")
//                  || utf8::equals(clazz->name, "java/lang/invoke/VarHandle");
//     if (!b)
//         return false;

//     jarrRef ptypes = getParameterTypes(); // Class<?>[]
//     if (ptypes->arr_len != 1)
//         return false;

//     auto ptype = ptypes->getElt<jclsRef>(0);
//     if (!utf8::equals(ptype->jvm_mirror->name, "[Ljava/lang/Object;"))
//         return false;

//     if (!(isVarargs() && isNative()))
//         return false;

//     return true;
// }

pair<const utf8_t *, const utf8_t *> Method::find_local_variable(u2 pc, u2 index) {
    for (LocalVarTable &t: local_variable_tables) {
        if (t.match(pc, index)) {
            auto lcal_var_name = clazz->cp->utf8(t.name_index);
            auto lcal_var_descriptor = clazz->cp->utf8(t.descriptor_index);
            return make_pair(lcal_var_name, lcal_var_descriptor);
        }
    }
    return make_pair(nullptr, nullptr);
}

string Method::toString() const {
    ostringstream oss;
    oss << "method" 
        << (is_native() ? "(native)" : "") 
        << (isStatic() ? "(static)" : "(nonstatic)") 
        << ": " 
        << clazz->name << "~" << name <<  "~" << descriptor;
    return oss.str();
}

string Method::get_bytecode_string() const {
    ostringstream oss;

    BytecodeReader r(code, code_len);
    ConstantPool *cp = clazz->cp;

    while (r.has_more()) {
        auto c = r.readu1();
        const char *n = opcode_names[c];
        oss << n;
        if (c == JVM_OPC_invokestatic) {
            auto index = r.readu2();
            Method *m = cp->resolve_method_or_interface_method(index);
            oss << " #" << ((int) index) << " <" << m->toString().c_str() << ">";
        } else if (c == JVM_OPC_invokevirtual) {
            auto index = r.readu2();
            Method *m = cp->resolve_method(index);
            oss << " #" << ((int) index) << " <" << m->toString().c_str() << ">";
        } else if (c == JVM_OPC_getfield) {
            auto index = r.readu2();
            Field *f = cp->resolve_field(index);
            oss << " #" << ((int) index) << " <" << f->toString().c_str() << ">";
        } else if (c == JVM_OPC_checkcast) {
            auto index = r.readu2();
            Class *cls = cp->resolve_class(index);
            oss << " #" << ((int) index) << " <" << cls->name << ">";
        } else {
            r.skip(opcode_length[c] - 1); // already read 1 byte
        }

        oss << endl;
    }

    return oss.str();
}
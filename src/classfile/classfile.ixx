module;
#include "../cabin.h"
#include "../jni.h"

export module classfile;

export import poly;

import std.core;
import vmstd;
import bytecode_reader;


// 从 1 开始计数，第0位无效
export class ConstantPool {
    Class *owner = nullptr;
public:
    u1 *type = nullptr;
    slot_t *info = nullptr;
    u2 size = 0;

private:
    mutable std::recursive_mutex mutex;

    // Empty ConstantPool
    explicit ConstantPool(Class *c);

    ConstantPool(Class *c, BytecodeReader &r);

public:
    ~ConstantPool();

    u2 get_size() const;
    u1 get_type(u2 i) const;
    void set_type(u2 i, u1 new_type);
    void set_info(u2 i, slot_t new_info);

    utf8_t *utf8(u2 i) const;
    utf8_t *string(u2 i) const;
    utf8_t *class_name(u2 i) const;
    utf8_t *module_name(u2 i);
    utf8_t *package_name(u2 i);
    utf8_t *name_of_name_and_type(u2 i);
    utf8_t *type_of_name_and_type(u2 i);
    u2 field_class_index(u2 i);
    utf8_t *field_class_name(u2 i);
    utf8_t *field_name(u2 i);
    utf8_t *field_type(u2 i);
    u2 method_class_index(u2 i);
    utf8_t *method_class_name(u2 i);
    utf8_t *method_name(u2 i);
    utf8_t *method_type(u2 i);
    u2 interface_method_class_index(u2 i);
    utf8_t *interface_method_class_name(u2 i);
    utf8_t *interface_method_name(u2 i);
    utf8_t *interface_method_type(u2 i);
    utf8_t *method_type_descriptor(u2 i);
    u2 method_handle_reference_kind(u2 i);
    u2 method_handle_reference_index(u2 i);
    u2 invoke_dynamic_bootstrap_method_index(u2 i);
    utf8_t *invoke_dynamic_method_name(u2 i);
    utf8_t *invoke_dynamic_method_type(u2 i);
    jint get_int(u2 i) const;
    void set_int(u2 i, jint new_int);
    jfloat get_float(u2 i) const;
    void set_float(u2 i, jfloat new_float);
    jlong get_long(u2 i) const;
    void set_long(u2 i, jlong new_long);
    jdouble get_double(u2 i) const;
    void set_double(u2 i, jdouble new_double);

    Class  *resolve_class(u2 i);
    Method *resolve_method(u2 i);
    Method *resolve_interface_method(u2 i);
    Method *resolve_method_or_interface_method(u2 i);
    Field  *resolve_field(u2 i);
    Object *resolve_string(u2 i);
    Object *resolve_method_type(u2 i);
    Object *resolve_method_handle(u2 i);

    struct ResolvedInvDyn {
        const utf8_t *name;
        const utf8_t *descriptor;
        u2 boot_method_index;

        ResolvedInvDyn(const utf8_t *name0, const utf8_t *descriptor0, u2 boot_method_index0)
                :name(name0), descriptor(descriptor0), boot_method_index(boot_method_index0) { }
    };
    ResolvedInvDyn *resolve_invoke_dynamic(u2 i);

    std::string toString() const;

    friend class Class;
    friend class ArrayClass;
};

export Method *find_invoke_dynamic_invoker(
        Class *c, ConstantPool::ResolvedInvDyn *inv_dyn, Object *&appendix);


export struct DefinedModule {
    jref module;
    jbool is_open;
    jstrRef version;
    jstrRef location;
    std::vector<const utf8_t *> packages;

    // `packages0`: array of packages in the module
    DefinedModule(jref module0, jbool is_open0,
                  jstrRef version0, jstrRef location0, jarrRef packages0);

    bool contain_package(const utf8_t *pkg) const;
};

export void define_module_to_vm(jref module, jbool is_open,
                         jstrRef version, jstrRef location, jarrRef packages);

export jref set_class_module(Class *c);

export void init_module();

export void set_boot_loader_unnamed_module(jref module);

export namespace java_lang_Module {
    const utf8_t *get_name(jref module);
    jref get_loader(jref module);
}

export struct ModuleAttribute {
    const utf8_t *module_name;
    u2 module_flags;
    const utf8_t *module_version;

    struct Require {
        const utf8_t *require_module_name;
        u2 flags;
        // If requires_version is NULL,
        // then no version information about the current module is present.
        const utf8_t *version;

        explicit Require(ConstantPool &cp, BytecodeReader &r);
    };
    std::vector<Require> _requires;

    struct Export {
        const utf8_t *export_package_name;
        u2 flags;
        std::vector<const utf8_t *> exports_to;

        explicit Export(ConstantPool &cp, BytecodeReader &r);
    };
    std::vector<Export> exports;

    struct Open {
        const utf8_t *open_package_name;
        u2 flags;
        std::vector<const utf8_t *> opens_to;

        explicit Open(ConstantPool &cp, BytecodeReader &r);
    };
    std::vector<Open> opens;

    std::vector<const utf8_t *> uses;

    struct Provide {
        const utf8_t *class_name;
        std::vector<const utf8_t *> provides_with;

        explicit Provide(ConstantPool &cp, BytecodeReader &r);
    };
    std::vector<Provide> provides;

    explicit ModuleAttribute(ConstantPool &cp, BytecodeReader &r);
};


/*
 * 提取 Class, Field 和 Method 的公共特征
 */

export struct Annotation {
    u1 *data = nullptr;
    u4 len = 0;

    void parse(BytecodeReader &r, u4 attr_len) {
        len = attr_len;
        data = new u1[len];
        memcpy(data, r.curr_pos(), len);
        r.skip(len);
    }
};

export class Meta {
public:
    // name of Class, Field and Method
    // if is class name, 必须是全限定类名，包名之间以 '/' 分隔。
    const utf8_t *name = nullptr;
    int access_flags = 0;
    bool deprecated = false;
    const utf8_t *signature = nullptr;

    // std::vector<Annotation> rt_visi_annos;   // runtime visible annotations
    // std::vector<Annotation> rt_invisi_annos; // runtime invisible annotations

    Annotation rt_visi_annos;   // Runtime Visible nnotations
    Annotation rt_invisi_annos; // Runtime Invisible Annotations

    Annotation rt_visi_type_annos;  // Runtime Visible Type Annotations
    Annotation rt_invisi_type_annos;// Runtime Invisible Type Annotations

public:
    [[nodiscard]] bool isPublic() const    { return accIsPublic(access_flags); }
    [[nodiscard]] bool isProtected() const { return accIsProtected(access_flags); }
    [[nodiscard]] bool isPrivate() const   { return accIsPrivate(access_flags); }
    [[nodiscard]] bool isStatic() const    { return accIsStatic(access_flags); }
    [[nodiscard]] bool isFinal() const     { return accIsFinal(access_flags); }
    [[nodiscard]] bool isSynthetic() const { return accIsSynthetic(access_flags); }

    void setSynthetic() { accSetSynthetic(access_flags); }
};

export struct BootstrapMethod {
    /*
     * bootstrap_method_ref 项的值必须是一个对常量池的有效索引。
     * 常量池在该索引处的值必须是一个 CONSTANT_MethodHandle_info 结构。
     * 注意：此CONSTANT_MethodHandle_info结构的reference_kind项应为值6（REF_invokeStatic）或8（REF_newInvokeSpecial），
     * 否则在invokedynamic指令解析调用点限定符时，引导方法会执行失败。
     */
    u2 bootstrap_method_ref;

    /*
     * bootstrap_arguments 数组的每个成员必须是一个对常量池的有效索引。
     * 常量池在该索引出必须是下列结构之一：
     * CONSTANT_String_info, CONSTANT_Class_info, CONSTANT_Integer_info, CONSTANT_Long_info,
     * CONSTANT_Float_info, CONSTANT_Double_info, CONSTANT_MethodHandle_info, CONSTANT_MethodType_info。
     */
    std::vector<u2> bootstrap_arguments;

    Class *owner;

    explicit BootstrapMethod(Class *owner0, BytecodeReader &r);

    // slot_t *resolveArgs(ConstantPool *cp, slot_t *result);
    bool resolve_args(jobjArrRef &result);
};

/*
 * The metadata of a class.
 */
export class Class: public Meta {
public:
    enum State {
        EMPTY,
        LOADED,
        LINKED,
        INITING,
        INITED // 此类是否被初始化过了（是否调用了<clinit>方法）。
    } state = EMPTY;

    bool initialized() const { return state >= INITED; }

    ConstantPool *cp = nullptr;

    const utf8_t *pkg_name = nullptr; // 包名之间以 '.' 分隔。

    bool hidden = false; // if is hidden class.

    Object *java_mirror = nullptr;

    // the class loader who loaded this class
    // 可能为null，表示 bootstrap class loader.
    Object *loader = nullptr;

    Class *super_class = nullptr;

    // 本类声明实现的interfaces，父类声明实现的不包括在内。
    // 但如果父类声明实现，本类也声明了实现的接口，则包括在内。
    std::vector<Class *> interfaces;

    /*
     * 本类中定义的所有方法（不包括继承而来的）
     * 所有的 public functions 都放在了最前面
     */
    std::vector<Method *> methods;

    /*
     * 本类中所定义的变量（不包括继承而来的）
     * include both class variables and instance variables,
     * declared by this class or interface type.
     * 类型二统计为两个数量
     *
     * todo 接口中的变量怎么处理
     */
    std::vector<Field *> fields;

    // inst_fields_count 有可能大于 fields_count instFieldsCount 包含了继承过来的 field.
    // 类型二统计为两个数量
    int inst_fields_count = 0;

    // vtable 只保存虚方法。
    // 该类所有函数自有函数（除了private, static, final, abstract）和 父类的函数虚拟表。
    std::vector<Method *> vtable;

    /*
    * 为什么需要itable,而不是用vtable解决所有问题？
    * 一个类可以实现多个接口，而每个接口的函数编号是个自己相关的，
    * vtable 无法解决多个对应接口的函数编号问题。
    * 而对继承一个类只能继承一个父亲，子类只要包含父类vtable，
    * 并且和父类的函数包含部分编号是一致的，就可以直接使用父类的函数编号找到对应的子类实现函数。
    */
    struct ITable {
        std::vector<std::pair<Class *, size_t /* offset */>> itf_offsets;
        std::vector<Method *> methods;

        ITable() = default;
        ITable(const ITable &);
        ITable& operator=(const ITable &);
        void add(const ITable &);
    } itable;

    Method *findFromITable(Class *interface_class, int index);

    struct {
        Class *clazz = nullptr;       // the immediately enclosing class
        Object *name = nullptr;       // the immediately enclosing method or constructor's name (can be null).
        Object *descriptor = nullptr; // the immediately enclosing method or constructor's type (null if name is).
    } enclosing;

    // if this class is a inner class，下面两项有效
    u2 declaring_class = 0; // index in constant pool, CONSTANT_Class_info
    u2 inner_access_flags = 0;

    // Inner classes of this class
    // If not resolved, pair.first is 'false', pair.second is "u2", index in constant pool, CONSTANT_Class_info
    // If resolved, pair.first is 'true', pair.second is "Class *"
    std::vector<std::pair<bool, uintptr_t>> inner_classes;

    // std::vector<InnerClass> inner_classes;

    const char *source_file_name = nullptr;

    std::vector<BootstrapMethod> bootstrap_methods;

    std::mutex clinit_mutex;

    struct RecordComponent {
        const utf8_t *name = nullptr;
        const utf8_t *descriptor = nullptr;
        const utf8_t *signature = nullptr;

        Annotation rt_visi_annos;   // Runtime Visible Annotations
        Annotation rt_invisi_annos; // Runtime Invisible Annotations

        Annotation rt_visi_type_annos;  // Runtime Visible Type Annotations
        Annotation rt_invisi_type_annos;// Runtime Invisible Type Annotations

        RecordComponent(BytecodeReader &r, ConstantPool &cp);
    };
    std::vector<RecordComponent> record; // Present if is this class is a record


    // for module-info.class
    struct {
        ModuleAttribute *module = nullptr;
        std::vector<const utf8_t *> *module_packages = nullptr;
        const utf8_t *module_main_class = nullptr;
    } module_info;

private:
    Class *nest_host = nullptr;
    int nest_host_index = -1;

    bool is_nest_members_resolved = false;
    // if nest members is resolved, nest_members contais 'Class *'
    // else nest_members contais 'utf8_t *name'
    std::vector<void *> nest_members;

    // 创建 primitive class，由虚拟机直接生成。
    explicit Class(const char *class_name);

    Class(jref class_loader, const u1 *bytecode, size_t len);

    void generate_pkg_name();

    // 计算字段的个数，同时给它们编号
    void calc_fields_id();
    void parse_attribute(BytecodeReader &r, u2 this_idx);

    void create_vtable();
    void create_itable();

    bool inject_inst_field(const utf8_t *name, const utf8_t *descriptor);

    /*
     * maximally-specific super interface（MSSI, 最具体的超接口）
     *
     * 父类可能比子类实现的接口更具体，如下面的代码所示，子类B实现了接口 Father，
     * 而他的父类A却实现了更具体的接口 Son，查找接口方法时应该查找其父类接口中更具体的实现
     *
     * public class Foo {
     *
     *     interface Father {
     *         default String t() { return "Father"; }
     *     }
     *
     *     interface Son extends Father {
     *         default String t() { return "Son"; }
     *     }
     *
     *     private static class A implements Son { }
     *
     *     private static class B extends A implements Father { }
     *
     *     public static void main(String[] args) {
     *         System.out.println(new B().t()); // Should print "Son" not "Father"
     *     }
     * }
     *
     * MSSI 之间相互独立（没有重复的，也没有继承关系）
     */
    std::vector<Class *> mssis;

protected:
    explicit Class(jref class_loader): loader(class_loader) { }
    void generate_mssis();

public:
    ~Class();
    std::string toString() const;

    virtual size_t object_size() const;

    // 比较两个类是否相等
    bool equals(const Class *) const;

    // check can `this` cast to `t`?
    bool check_cast(Class *t);

    // Generate the class object of this (aka 'java_mirror' field).
    // This Object 只能由虚拟机创建
    void generate_class_object();

    // alloc non array object
    // virtual Object *alloc_object();

    // alloc native(non Heap) non array object
    virtual Object *alloc_native_object();

    bool is_subclass_of(Class *father);

    /*
     * 计算一个类的继承深度。
     * 如：java.lang.Object的继承的深度为0
     * java.lang.Number继承自java.lang.Object, java.lang.Number的继承深度为1.
     */
    int inherited_depth() const;

    bool check_class_name(const utf8_t *name0) const { return strcmp(name, name0) == 0; }

    bool is_prim_class() const;
    bool is_prim_wrapper_class();

    bool is_array_class() const { return name[0] == '['; }

    /*
     * 是否是基本类型的数组（当然是一维的）。
     * 基本类型
     * bool, byte, char, short, int, float, long, double
     * 分别对应的数组类型为
     * [Z,   [B,   [C,   [S,    [I,  [F,    [J,   [D
     */
    bool is_type_array_class() const;

    bool is_bool_array_class() const   { return check_class_name("[Z"); }
    bool is_byte_array_class() const   { return check_class_name("[B"); }
    bool is_char_array_class() const   { return check_class_name("[C"); }
    bool is_short_array_class() const  { return check_class_name("[S"); }
    bool is_int_array_class() const    { return check_class_name("[I"); }
    bool is_float_array_class() const  { return check_class_name("[F"); }
    bool is_long_array_class() const   { return check_class_name("[J"); }
    bool is_double_array_class() const { return check_class_name("[D"); }

    bool is_ref_array_class() const { return (is_array_class() && !is_type_array_class()); }

    bool is_interface() const { return accIsInterface(access_flags); }
    bool is_abstract() const  { return accIsAbstract(access_flags); }
    bool is_strict() const    { return accIsStrict(access_flags); }
    bool is_super() const     { return accIsSuper(access_flags); }
    bool is_module() const    { return accIsModule(access_flags); }

    // @descriptor 用于找打Field后检测类型是否匹配，如不想检测传NULL即可。
    Field *get_field(const char *name, const char *descriptor = nullptr) const;
    Field *get_field(int id) const;

    // @descriptor 用于找打Field后检测类型是否匹配，如不想检测传NULL即可。
    Field *lookup_field(const char *name, const char *descriptor = nullptr);
    Field *lookup_field(int id);

    // get在本类中定义的类，不包括继承的。
    Method *get_method(const char *name, const char *descriptor);

    Method *lookup_method(const char *name, const char *descriptor);
    // Method *lookupInterfaceMethod(const char *name, const char *descriptor);

    std::vector<Method *> get_declared_methods(const utf8_t *name, bool public_only);

    // generate polymorphic method
    Method *generate_poly_method(const utf8_t *method_name, const utf8_t *descriptor);

    Method *get_constructor(const utf8_t *descriptor);

    // Class<?>[] parameterTypes;
    Method *get_constructor(jarrRef parameter_types);

    std::vector<Method *> get_constructors(bool public_only);

    ArrayClass *generate_array_class() const;

    void set_nest_host(Class *host) { nest_host = host; }
    Class *get_nest_host();

    void add_nest_member(Class *member);
    std::vector<void *> &get_nest_members();

    bool test_nest_mate(Class *c);

    friend Class *load_boot_class(const utf8_t *name);
    friend Class *define_class(jref class_loader, const u1 *bytecode, size_t len);
};

export class ArrayClass: public Class {
    Class *comp_class = nullptr; // component class
    Class *elt_class = nullptr;  // element class
    size_t elt_size = 0;

    // array class 由虚拟机直接生成。
    ArrayClass(Object *loader, const char *class_name);
    friend ArrayClass *load_array_class(Object *loader, const utf8_t *arr_class_name);

public:
    int dimension = 0; // 数组的维度

    // 判断数组单个元素的大小
    // 除了基本类型的数组外，其他都是引用类型的数组
    // 多维数组是数组的数组，也是引用类型的数组
    size_t get_element_size();

    /*
     * Returns the representing the component class of an array class.
     * If this class does not represent an array class this method returns null.
     *
     * like，依次调用 componentClass():
     * [[I -> [I -> int -> null
     */
    Class *get_component_class();

    /*
     * Returns the representing the element class of an array class.
     * If this class does not represent an array class this method returns null.
     *
     * like [[[I 的元素类是 int.class
     */
    Class *get_element_class();

    size_t object_size() const override { UNREACHABLE("ArrayClass does not support this method."); }
    // Object *alloc_object() override { UNREACHABLE("ArrayClass does not support this method."); }
    Object *alloc_native_object() override { UNREACHABLE("ArrayClass does not support this method."); }

    size_t object_size(jint arr_len);
};

/* A Field represents a Java field. */
export class Field: public Meta {
public:
    Class *clazz = nullptr;
    const utf8_t *descriptor = nullptr;

    bool category_two;

    union {
        // Present if static field,
        // static value 必须初始化清零
        union {
            jbool z;
            jbyte b;
            jchar c;
            jshort s;
            jint i;
            jlong j;
            jfloat f;
            jdouble d;
            jref r;
            slot_t data[2];
        } static_value{};

        // Present if instance field
        int id;
    };

    Field(Class *c, BytecodeReader &r);

    Field(Class *, const utf8_t *name, const utf8_t *descriptor, int access_flags);

    bool is_transient() const { return accIsTransient(access_flags); }
    bool is_volatile() const  { return accIsVolatile(access_flags); }

    bool is_prim_field() const;

    // the declared type(class Object) of this field
    // like, int k; the type of k is int.class
    jclsRef get_type();

    std::string toString() const;
};


/* A Method represents a Java method. */
export class Method: public Meta {
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
    void (* native_invoker)(void *, JNIEnv *, jref, slot_t *, void*/*Frame*/) = nullptr;

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
    bool is_native() const       {
        return (access_flags & JVM_ACC_NATIVE) != 0;
//        return accIsNative(access_flags);
    }
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

    void cal_args_slots_count() {
        arg_slots_count = cal_args_slots_count(descriptor, isStatic());
    }

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


export int numSlotsInMethodDescriptor(const char *method_descriptor);

// @b: include
// @e：exclude
// eg. I[BLjava/lang/String;ZZ, return 5.
export int numEltsInDescriptor(const char *b, const char *e);

export int numEltsInMethodDescriptor(const char *method_descriptor);

export std::pair<jarrRef /*ptypes*/, jclsRef /*rtype*/>
parseMethodDescriptor(const char *desc, jref loader);

export std::string unparseMethodDescriptor(jarrRef ptypes /* ClassObject *[] */, jclsRef rtype);

// @method_type: Object of java.lang.invoke.MethodType
export std::string unparseMethodDescriptor(jref method_type);

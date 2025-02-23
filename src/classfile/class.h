#ifndef CABIN_CLASS_H
#define CABIN_CLASS_H

#include <cstring>
#include <vector>
#include <string>
#include <unordered_set>
#include <mutex>
#include "bytecode_reader.h"
#include "../cabin.h"
#include "constant_pool.h"
#include "meta.h"
#include "../primitive.h"

class ModuleAttribute;

struct BootstrapMethod {
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
class Class: public Meta {
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

    bool is_prim_class() const { return is_prim_class_name(name); }
    bool is_prim_wrapper_class() const { return is_prim_wrapper_class_name(name); }

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

#endif // CABIN_CLASS_H
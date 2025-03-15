module;
#include <cassert>
#include "../vmdef.h"

module classfile;

import std.core;
import interpreter;
import object;
import primitive;
import class_loader;
import exception;

using namespace std;
using namespace slot;
using namespace utf8;

BootstrapMethod::BootstrapMethod(Class *owner0, BytecodeReader &r): owner(owner0) {
    assert(owner != nullptr);
    bootstrap_method_ref = r.readu2();
    u2 num = r.readu2();
    for (u2 i = 0; i < num; i++) {
        bootstrap_arguments.push_back(r.readu2());
    }
}

bool BootstrapMethod::resolve_args(jobjArrRef &result) {
    assert(result != nullptr);

    if (result->arr_len < (jint) bootstrap_arguments.size()) {
        ERR("%d, %lld.\n", result->arr_len, bootstrap_arguments.size());
        return false;
    }

    int k = 0;    
    ConstantPool &cp = *(owner->cp);
    for (u2 i : bootstrap_arguments) {
        jref o;

        switch (cp.get_type(i)) {
            case JVM_CONSTANT_String:
            case JVM_CONSTANT_ResolvedString: o = cp.resolve_string(i);              break;
            case JVM_CONSTANT_Class:
            case JVM_CONSTANT_ResolvedClass:  o = cp.resolve_class(i)->java_mirror;  break;
            case JVM_CONSTANT_Integer:        o = int_box(cp.get_int(i));            break;
            case JVM_CONSTANT_Float:          o = float_box(cp.get_float(i));        break;
            case JVM_CONSTANT_Long:           o = long_box(cp.get_long(i));          break;
            case JVM_CONSTANT_Double:         o = double_box(cp.get_double(i));      break;
            case JVM_CONSTANT_MethodHandle:   o = cp.resolve_method_handle(i);       break;
            case JVM_CONSTANT_MethodType:     o = cp.resolve_method_type(i);         break;
            default: UNREACHABLE("Wrong type: %d.\n", cp.get_type(i));
        }

        result->setRefElt(k++, o);
    }

    return true;
}

// 计算字段的个数，同时给它们编号
void Class::calc_fields_id() {
    int ins_id = 0;
    if (super_class != nullptr) {
        ins_id = super_class->inst_fields_count;
    }

    for (Field *f: fields) {
        if (!f->isStatic()) {
            f->id = ins_id++;
            if (f->category_two)
                ins_id++;
        }
    }

    inst_fields_count = ins_id;
}

Class::RecordComponent::RecordComponent(BytecodeReader &r, ConstantPool &cp) {
    name = cp.utf8(r.readu2());
    descriptor = cp.utf8(r.readu2());

    u2 attributes_count = r.readu2();
    for (u2 i = 0; i < attributes_count; i++) {
        const char *attr_name = cp.utf8(r.readu2());
        u4 attr_len = r.readu4();

        if (strcmp("Signature", attr_name) == 0) {
            signature = cp.utf8(r.readu2());
        } else  if (strcmp("RuntimeVisibleAnnotations", attr_name) == 0) {
            rt_visi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleAnnotations", attr_name) == 0) {
            rt_invisi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeVisibleTypeAnnotations", attr_name) == 0) {
            rt_visi_type_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleTypeAnnotations", attr_name) == 0) {
            rt_invisi_type_annos.parse(r, attr_len);
        } else { // unknown attribute
            WARN("unknown attribute: %s\n", attr_name);
            r.skip(attr_len);
        }
    }
}

void Class::parse_attribute(BytecodeReader &r, u2 this_idx) {
    u2 attr_count = r.readu2();

    for (int i = 0; i < attr_count; i++) {
        const char *attr_name = cp->utf8(r.readu2());
        u4 attr_len = r.readu4();

        if (strcmp("Signature", attr_name) == 0) {
            signature = cp->utf8(r.readu2());
        } else if (strcmp("Synthetic", attr_name) == 0) {
            setSynthetic();
        } else if (strcmp("Deprecated", attr_name) == 0) {
            deprecated = true;
        } else if (strcmp("SourceFile", attr_name) == 0) {
            u2 source_file_index = r.readu2();
            if (source_file_index > 0) {
                source_file_name = cp->utf8(source_file_index);
            }
        } else if (strcmp("EnclosingMethod", attr_name) == 0) {
            u2 class_index = r.readu2();  // 指向 CONSTANT_Class_info
            u2 method_index = r.readu2(); // 指向 CONSTANT_NameAndType_info

            if (class_index > 0) {
                enclosing.clazz = cp->resolve_class(class_index);

                if (method_index > 0) {
                    enclosing.name = Allocator::string(cp->name_of_name_and_type(method_index));
                    enclosing.descriptor = Allocator::string(cp->type_of_name_and_type(method_index));
                }
            }
        } else if (strcmp("BootstrapMethods", attr_name) == 0) {
            u2 num = r.readu2();
            for (u2 k = 0; k < num; k++)
                bootstrap_methods.emplace_back(this, r);
        } else if (strcmp("InnerClasses", attr_name) == 0) {
            u2 num = r.readu2();
            for (u2 k = 0; k < num; k++) {
                // InnerClass ic(r);
                u2 inner_class_info_index = r.readu2();
                u2 outer_class_info_index = r.readu2();
                u2 inner_name_index = r.readu2();
                u2 inner_class_access_flags = r.readu2();
                
                if (inner_class_info_index == this_idx) {
                    declaring_class = outer_class_info_index;
                    // if (inner_name_index == 0) { // 无效
                    //     access_flags |= ANONYMOUS;  // 匿名类
                    // }
                    inner_access_flags = inner_class_access_flags;
                } else if (outer_class_info_index == this_idx) {
                    inner_classes.emplace_back(false, inner_class_info_index);
                } else {
                    // todo 不知道这个 InnerClasses 放在这里是干嘛的
                }
            }
        } else if (strcmp("SourceDebugExtension", attr_name) == 0) { // ignore
//            u1 source_debug_extension[attr_len];
//            bcr_read_bytes(reader, source_debug_extension, attr_len);
            r.skip(attr_len); // todo
        } else if (strcmp("RuntimeVisibleAnnotations", attr_name) == 0) {
           rt_visi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleAnnotations", attr_name) == 0) {
            rt_invisi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeVisibleTypeAnnotations", attr_name) == 0) {
            rt_visi_type_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleTypeAnnotations", attr_name) == 0) {
            rt_invisi_type_annos.parse(r, attr_len);
        } else if (strcmp("Module", attr_name) == 0) {
            module_info.module = new ModuleAttribute(*cp, r);
        } else if (strcmp("ModulePackages", attr_name) == 0) {
            module_info.module_packages = new vector<const utf8_t *>;
            u2 num = r.readu2();
            for (int j = 0; j < num; j++) {
                u2 index = r.readu2();
                module_info.module_packages->push_back(cp->package_name(index));
            }
        } else if (strcmp("ModuleMainClass", attr_name) == 0) {
            module_info.module_main_class = cp->class_name(r.readu2());
        } else if (strcmp("NestHost", attr_name) == 0) {
            nest_host_index = r.readu2();
        } else if (strcmp("NestMembers", attr_name) == 0) {
            u2 num = r.readu2();
            for (u2 j = 0; j < num; j++) {
                utf8_t *class_name = cp->class_name(r.readu2());
                nest_members.push_back(class_name);
                // 不要在这里 loadClass，有死循环的问题。
                // 比如java.lang.invoke.TypeDescriptor
                // 和其NestMember：java.lang.invoke.TypeDescriptor$OfField
            }
        } else if (strcmp("Record", attr_name) == 0) {
            u2 components_count = r.readu2();
            for (u2 k = 0; k < components_count; k++) {
                record.emplace_back(r, *cp);
            }
        } else { // unknown attribute
            WARN("unknown attribute: %s\n", attr_name);
            r.skip(attr_len);
        }
    }
}

void Class::create_vtable() {
    assert(vtable.empty());

    if (super_class == nullptr) {
        int i = 0;
        for (auto &m : methods) {
            if (m->is_virtual()) {
                vtable.push_back(m);
                m->vtable_index = i++;
            }
        }
        return;
    }

    // 将父类的vtable复制过来
    vtable.assign(super_class->vtable.begin(), super_class->vtable.end());

    for (auto m : methods) {
        if (m->is_virtual()) {
            auto iter = find_if(vtable.begin(), vtable.end(), [=](Method *m0){
                        return utf8::equals(m->name, m0->name)
                                && utf8::equals(m->descriptor, m0->descriptor);
            });
            if (iter != vtable.end()) {
                // 重写了父类的方法，更新
                m->vtable_index = (*iter)->vtable_index;
                *iter = m;
            } else {
                // 子类定义了要给新方法，加到 vtable 后面
                vtable.push_back(m);
                m->vtable_index = ((int) vtable.size()) - 1;
            }
        }
    }
}

Class::ITable::ITable(const ITable &itable0) {
    itf_offsets.assign(itable0.itf_offsets.begin(), itable0.itf_offsets.end());
    methods.assign(itable0.methods.begin(), itable0.methods.end());
}

Class::ITable& Class::ITable::operator=(const ITable &itable0) {
    itf_offsets.assign(itable0.itf_offsets.begin(), itable0.itf_offsets.end());
    methods.assign(itable0.methods.begin(), itable0.methods.end());
    return *this;
}

void Class::ITable::add(const ITable &itable0) {
    for (auto &itf_offset: itable0.itf_offsets) {
        itf_offsets.emplace_back(itf_offset.first, methods.size());
        for (auto m: itable0.methods)
            methods.push_back(m);
    }
}

Method *Class::findFromITable(Class *interface_class, int itable_index) {
    if(interface_class != nullptr && interface_class->is_interface()) {
        for (auto &itf_offset: itable.itf_offsets) {
            if (itf_offset.first->equals(interface_class)) {
                size_t offset = itf_offset.second;
                return itable.methods.at(offset + itable_index);
            }
        }
    }

    return nullptr;
}

void Class::create_itable() {
    if (is_interface()) {
        // 接口间的继承虽然用 extends 关键字（可以同时继承多个接口），但被继承的接口不是子接口的 super_class，
        // 而是在子接口的 itf_offsets 里面。所以接口的 super_class 就是 java/lang/Object

        for (Class *super_interface: interfaces) {
            itable.add(super_interface->itable);
        }

        itable.itf_offsets.emplace_back(this, itable.methods.size());
        int index = 0;
        for (Method *m : methods) {
            // todo default 方法怎么处理？进不进 itable？
            // todo 调用 default 方法 生成什么调用指令？
            m->itable_index = index++;
            itable.methods.push_back(m);
        }
        
        return;
    }

    /* parse non interface class */

    if (super_class != nullptr) {
        itable = super_class->itable;
    }

    for (auto ifc : interfaces) {
        for (auto &itf_offset : itable.itf_offsets) {
            if (ifc->equals(itf_offset.first)) {
                // 此接口已经在 itable.itf_offsets 中了
                goto next;
            }
        }

        // 发现一个新实现的接口
        itable.itf_offsets.emplace_back(ifc, itable.methods.size());
        // 检查新实现接口的方法是不是已经被重写了
        for (auto m: ifc->methods) {
            for (auto m0: itable.methods) {
                if (utf8::equals(m->name, m0->name) && utf8::equals(m->descriptor, m0->descriptor)) {
                    m = m0; // 重写了接口方法，更新
                    break;
                }
            }
            itable.methods.push_back(m);
        }
next:;
    }
    
    // 遍历 itable.methods，检查有没有接口函数在本类中被重写了。
    for (size_t i = 0; i < itable.methods.size(); i++) {
        auto m = itable.methods[i];
        for (auto m0 : methods) {
            if (utf8::equals(m->name, m0->name) && utf8::equals(m->descriptor, m0->descriptor)) {
                //m0->itable_index = m->itable_index;
                itable.methods[i] = m0; // 重写了接口方法，更新
                break;
            }
        }
    }
}

void Class::generate_mssis() {
    if (super_class != nullptr) {
        mssis = super_class->mssis;
    }

    for (Class *i: interfaces) {
        auto iter = mssis.begin();
        for (; iter != mssis.end(); iter++) {
            // 两两比较，保留子接口，抛弃父接口            
            if ((*iter)->is_subclass_of(i)) {
                break;
            } else if (i->is_subclass_of(*iter)) {
                *iter = i;
                break;
            }
        }

        if (iter == mssis.end()) {
            // i 和 mssis中的接口没有继承关系
            mssis.push_back(i);
        }
    }
}

// 根据类名生成包名
void Class::generate_pkg_name() {
    char *pkg = utf8::dup(name);
    char *p = strrchr(pkg, '/');
    if (p == nullptr) {
        free(pkg);
        pkg_name = ""; // 包名可以为空
    } else {
        *p = 0; // 得到包名
        slash_2_dot(pkg);
        const utf8_t *hashed = utf8_pool::find(pkg);
        if (hashed != nullptr) {
            free(pkg);
            pkg_name = hashed;
        } else {
            pkg_name = pkg;
            utf8_pool::save(pkg_name);
        }
    }
}

Class::Class(jref class_loader, const u1 *bytecode, size_t len): loader(class_loader) {
    assert(bytecode != nullptr);

    BytecodeReader r(bytecode, len);

    u4 magic = r.readu4();
    if (magic != 0xcafebabe) {
        throw java_lang_ClassFormatError("bad magic: " + to_string(magic));
    }

    r.readu2(); // minor_version
    u2 major_version = r.readu2();
    // todo 判断jdk版本和class文件版本是否匹配
    if (major_version != g_classfile_major_version) {
//        thread_throw(new ClassFormatError("bad class version")); // todo
    }

    cp = new ConstantPool(this, r);
    access_flags = r.readu2();
    u2 this_idx = r.readu2();
    name = cp->class_name(this_idx);
    generate_pkg_name();

    u2 super = r.readu2();
    if (super == 0) { // invalid constant pool reference
        super_class = nullptr;
    } else {
        if (utf8::equals(name, "java/lang/Object")) {
            throw java_lang_ClassFormatError("Object has super");
            // return nullptr;
        }
        super_class = cp->resolve_class(super);
    }

    // parse itf_offsets
    u2 interfaces_count = r.readu2();
    for (u2 i = 0; i < interfaces_count; i++) {
        interfaces.push_back(cp->resolve_class(r.readu2()));
    }

    // parse fields
    u2 fields_count = r.readu2();
    for (u2 i = 0; i < fields_count; i++) {
        fields.push_back(new Field(this, r));
    }

    calc_fields_id();

    // parse methods
    u2 methods_count = r.readu2();
    for (u2 i = 0; i < methods_count; i++) {
        methods.push_back(new Method(this, r));
    }

    parse_attribute(r, this_idx);

#if 0
    createVtable(); // todo 接口有没有必要创建 vtable
    createItable();
#endif

    generate_mssis();

    if (g_class_class != nullptr) {
        generate_class_object();

        // todo module != nullptr

        if (module_info.module == nullptr && class_loader != nullptr) {
            assert(class_loader != nullptr);
            // 对于unnamed module的处理
            // public final Module getUnnamedModule();
            Method *m = class_loader->clazz->lookup_method("getUnnamedModule", "()Ljava/lang/Module;");
            jref mo = execJavaR(m, { rslot(class_loader) });

            // set by VM
            // private transient Module module;
            java_mirror->set_field_value<jref>("module", "Ljava/lang/Module;", mo);
        }
    }

    // check inject fields
    if (class_loader == BOOT_CLASS_LOADER) {
        if (utf8::equals(name, "java/lang/invoke/MemberName")) {
            //@Injected intptr_t vmindex; // vtable index or offset of resolved member
            bool b = inject_inst_field("vmindex", "I");
            if (!b) {
                UNREACHABLE("inject field(vmindex) error");
            }
            return;
        }

        if (utf8::equals(name, "java/lang/invoke/ResolvedMethodName")) {
            //@Injected JVM_Method* vmtarget;
            //@Injected Class<?>    vmholder;
            bool b1 = inject_inst_field("vmtarget", "Ljava/lang/Object;");
            bool b2 = inject_inst_field("vmholder", "Ljava/lang/Class;");
            if (!b1 || !b2) {
                UNREACHABLE("inject fields(vmtarget, vmholder) error");
            }
            return;
        }

        if (utf8::equals(name, "java/lang/invoke/MethodHandleNatives$CallSiteContext")) {
            //@Injected JVM_nmethodBucket* vmdependencies;
            //@Injected jlong last_cleanup;
            bool b1 = inject_inst_field("vmdependencies", "Ljava/lang/Object;");
            bool b2 = inject_inst_field("last_cleanup", "J");
            if (!b1 || !b2) {
                UNREACHABLE("inject fields(vmdependencies, last_cleanup) error");
            }
            return;
        }
    }

    state = State::LOADED;
}

// 创建 primitive class，由虚拟机直接生成。
Class::Class(const char *class_name): loader(BOOT_CLASS_LOADER), super_class(g_object_class) {
    assert(class_name != nullptr);
    assert(is_prim_class_name(class_name));
    assert(g_object_class != nullptr);

    name = utf8::dup(class_name); /* 形参class_name可能非持久，复制一份 */
    access_flags = JVM_ACC_PUBLIC;

    cp = new ConstantPool(this); // todo

    pkg_name = "";

#if 0
    createVtable();  
    createItable();
#endif

    generate_mssis();

    if (g_class_class != nullptr) {
        generate_class_object();
    }

    state = State::INITED;
}

void Class::generate_class_object() {
    if (java_mirror == nullptr) {
        assert(g_class_class != nullptr);
        // static size_t size = sizeof(ClsObj) + g_class_class->inst_fields_count * sizeof(slot_t);

        // Class Object不在堆上分配，因为此对象无需gc。
        java_mirror = g_class_class->alloc_native_object();
        java_mirror->jvm_mirror = this;

        // Initialized in JVM not by private constructor
        // This field is filtered from reflection access, i.e. getDeclaredField
        // will throw NoSuchFieldException
        // private final ClassLoader classLoader;
        java_mirror->set_field_value<jref>("classLoader", "Ljava/lang/ClassLoader;", loader);

        if (is_array_class()) {
            Class *component = ((ArrayClass *)this)->get_component_class();
            assert(component != nullptr);            
            // private final Class<?> componentType;
            java_mirror->set_field_value<jref>("componentType", "Ljava/lang/Class;", component->java_mirror);
        }

        // set by VM
        // private transient Module module;
        set_class_module(this);

        // Set by VM
        // private transient Object classData;    todo
    }
    assert(java_mirror != nullptr);
}

// Object *Class::alloc_object() {
//     assert(!is_array_class());
//
//     size_t size = object_size();
//     return new (g_heap->alloc(size)) Object(this);
// }

Object *Class::alloc_native_object() {
    assert(!is_array_class());

    size_t size = object_size();
    return new (calloc(size, 1)) Object(this);
}

Class::~Class() {
    for (Field *f: fields) {
        delete f;
    }

    for (Method *m: methods) {
        delete m;
    }

    // todo something else
}

bool Class::equals(const Class *c) const {
    if (this == c)
        return true;
    if (c == nullptr)
        return false;
    return (loader == c->loader) && (utf8::equals(name, c->name));
}

bool Class::check_cast(Class *t) {
    assert(t != nullptr);
    if (!is_array_class()) {
        if (t->is_array_class())
            return false;
        return is_subclass_of(t);
    }
    // this is array type
    if (t->is_interface()) {
        // 数组实现了两个接口，看看t是不是其中之一。
        return is_subclass_of(t);
    } else if (t->is_array_class()) { // this and t are both array type
        Class *sc = ((ArrayClass *) this)->get_component_class();
        Class *tc = ((ArrayClass *) t)->get_component_class();
        if (sc->is_prim_class() || tc->is_prim_class()) {
            // this and t are same prim type array.
            return sc == tc;
        }
        return sc->check_cast(tc);
    } else { // t is not interface and array type,
        return utf8::equals(t->name, "java/lang/Object");
    }
}

size_t Class::object_size() const {
    if (is_array_class()) {
        UNREACHABLE("error"); // todo
    }
    return sizeof(Object) + inst_fields_count*sizeof(slot_t);
}

Field *Class::get_field(const char *_name, const char *descriptor) const {
    assert(_name != nullptr);

    for (Field *f: fields) {
        if (utf8::equals(f->name, _name)) { // find Field
            if (descriptor != nullptr && !utf8::equals(f->descriptor, descriptor)) {
                // It has been found, but the descriptors don't match.
                // This kind of situation generally shouldn't occur.
                // So where exactly is the problem?
                ERR("Find out \"%s\", but types mismatch, get \"%s\" want \"%s\"\n", _name, f->descriptor, descriptor);
                return nullptr;
            }
            return f;
        }
    }

    return nullptr;
}

Field *Class::get_field(int id) const {
    for (Field *f: fields) {
        if (!f->isStatic() && f->id == id)
            return f;
    }

    return nullptr;
}

Field *Class::lookup_field(const utf8_t *_name, const utf8_t *descriptor) {
    Field *f;
    Class *clazz = this;
    do {
        if ((f = clazz->get_field(_name, descriptor)) != nullptr)
            return f;
        clazz = clazz->super_class;
    } while (clazz != nullptr);

    // for (Class *itf: c->indep_interfaces) {
    //     // 在接口 c 及其父接口中查找
    //     if ((f = lookup_field(itf, _name, descriptor)) != nullptr)
    //         return f;
    // }
    // for (u2 i = 0; i < interfaces_count; i++) {
    //     if ((f = itf_offsets[i]->lookupField(_name, descriptor)) != nullptr)
    //         return f;
    // }
    for (Class *itf: mssis) {
        if ((f = itf->lookup_field(_name, descriptor)) != nullptr)
            return f;
    }

    return nullptr;
}

Field *Class::lookup_field(int id) {
    // Field *f = nullptr;
    Class *clazz = this;
    // do {
    //     try {
    //         return get_declared_inst_field(clazz, id);
    //     } catch (java_lang_NoSuchFieldError &e) { 
    //         // not found
    //     }
    //     clazz = clazz->super_class;
    // } while (clazz != nullptr);
    do {
        Field *f = clazz->get_field(id);
        if (f != nullptr) 
            return f;
        clazz = clazz->super_class;
    } while (clazz != nullptr);

    return nullptr;
}

bool Class::inject_inst_field(const utf8_t *_name, const utf8_t *descriptor) {
    assert(_name != nullptr && descriptor != nullptr);

    // 首先要确定class中不存在和要注入的field同名的field

    for (Field *f: fields) {
        if (utf8::equals(f->name, _name)) {
            // throw runtime_error(_name); // todo
            return false;
        }
    }

    for (Class *clazz = super_class; clazz != nullptr; clazz = clazz->super_class) {
        for (Field *f: clazz->fields) {
            // 在父类查找时只查子类可以看见的field，即非private field            
            if (!f->isPrivate() && utf8::equals(f->name, _name)) {
                // throw runtime_error(_name); // todo
                return false;
            }
        }
    }

    // todo 在接口及其父接口中查找

    int flags = JVM_ACC_PRIVATE | JVM_ACC_SYNTHETIC;
    auto injected = new Field(this, _name, descriptor, flags);
    fields.push_back(injected);

    injected->id = inst_fields_count;

    inst_fields_count++;
    if (injected->category_two)
        inst_fields_count++;

    return true;
}

Method *Class::get_method(const utf8_t *_name, const utf8_t *descriptor) {
    assert(_name != nullptr && descriptor != nullptr);

    for (Method *m: methods) {
        // if (m->isSignaturePolymorphic()) {
        //     if (strcmp(_name, "invoke") == 0&&strcmp(m->name, "invoke") == 0) {
        //         m->isSignaturePolymorphic();
        //         printvm("===== %s, %s, %s\n", m->clazz->name, m->name, m->descriptor);
        // }
        if (utf8::equals(m->name, _name) && utf8::equals(m->descriptor, descriptor))
            return m;
    }

    return nullptr;
}

// Method *Class::getPolymorphicMethod(const utf8_t *_name)
// {
//     assert(_name != nullptr);

//     for (Method *m: methods) {
//         if (utf8::equals(m->name, _name) && m->isSignaturePolymorphic())
//             return m;
//     }
    
//     return nullptr;
// }

Method *Class::generate_poly_method(const utf8_t *method_name, const utf8_t *descriptor) {
    auto [clazz, acc, native_method] = lookup_polymorphic_method(this, method_name);
    if (clazz == nullptr) {
        // Don't find polymorphic method named `method_name` 
        // in current class and it's super classes.
        return nullptr; 
    }

    auto m = new Method(clazz, method_name, descriptor, acc, native_method);
    return m;
}

vector<Method *> Class::get_declared_methods(const utf8_t *_name, bool public_only) {
    assert(_name != nullptr);
    
    vector<Method *> declared_methods;

    for (Method *m: methods) {
        if ((!public_only || m->isPublic()) && (utf8::equals(m->name, _name)))
            declared_methods.push_back(m);
    }

    return declared_methods;
}

Method *Class::get_constructor(const utf8_t *descriptor) {
    return get_method("<init>", descriptor);
}

Method *Class::get_constructor(jarrRef parameter_types) {
    assert(parameter_types != nullptr);

//    Class *c = loadBootClass("java/lang/invoke/MethodType");
//
//    // public static MethodType methodType(Class<?> rtype, Class<?>[] ptypes);
//    Method *m = c->getDeclaredStaticMethod(
//            "methodType", "(Ljava/lang/Class;[Ljava/lang/Class;)Ljava/lang/invoke/MethodType;");
//    auto mt = RSLOT(execJavaFunc(m, { loadBootClass("void")->java_mirror, parameter_types } ));
//
//    // public String toMethodDescriptorString();
//    m = c->getDeclaredInstMethod("toMethodDescriptorString", "()Ljava/lang/String;");
//    auto s = (RSLOT(execJavaFunc(m, {mt})))->toUtf8();
    
    string desc = unparseMethodDescriptor(parameter_types, nullptr);
    return get_constructor(desc.c_str());

    // char *desc = unparse_method_descriptor(parameter_types, nullptr);
    // return getConstructor(desc);
}

vector<Method *> Class::get_constructors(bool public_only) {
    return get_declared_methods("<init>", public_only);
}

Method *Class::lookup_method(const char *name0, const char *descriptor) {
    // Method *method = getMethod(_name, descriptor);
    // if (method != nullptr) 
    //     return method;

    // // todo 是否应该禁止查找父类的私有方法，因为子类看不见父类的私有方法

    // Class *super = super_class;
    // if (super != nullptr) 
    //     return super->lookupMethod(_name, descriptor);

    for (Class *c = this; c != nullptr; c = c->super_class) {
        Method *m = c->get_method(name0, descriptor);
        if (m != nullptr) 
            return m;
    }  

    for (Class *i: mssis) {
        Method *m = i->lookup_method(name0, descriptor);
        if (m != nullptr) 
            return m;
    }

    return nullptr;
}

ArrayClass *Class::generate_array_class() const {
    // todo 判断 c 的维度，jvms数组最大维度为255. ARRAY_MAX_DIMENSIONS
//    char buf[strlen(name) + 8]; // big enough
    auto buf= new char[strlen(name) + 8]; // big enough

    // 数组
    if (name[0] == '[') {
        sprintf(buf, "[%s", name);
        return load_array_class(loader, buf);
    }

    // 基本类型
    // todo 判断class是不是void，如果是void不能创建数组
    const char *tmp = get_prim_array_class_name(name);
    if (tmp != nullptr)
        return load_array_class(loader, tmp);

    // 类引用
    sprintf(buf, "[L%s;", name);
    return load_array_class(loader, buf);
}

string Class::toString() const {
    ostringstream oss;
    oss << "class: " << name << ends;

    oss << "\tdeclared static fields: " << ends;
    for (Field *f: fields) {
        if (f->isStatic()) {
            oss << "\t\t" << f->name << "~" << f->descriptor << ends;
        }
    }

    oss << "\tdeclared instance fields: " << ends;
    for (Field *f: fields) {
        if (!f->isStatic()) {
            oss << "\t\t" << f->name << "~" << f->descriptor << " | " << f->id << ends;
        }
    }

    return oss.str();
}

bool Class::is_subclass_of(Class *father) {
    assert(father != nullptr);

    if (this == father)
        return true;

    if (super_class != nullptr && super_class->is_subclass_of(father))
        return true;

    for (Class *itf: interfaces) {
        if (itf->is_subclass_of(father))
            return true;
    }

    // array class 特殊处理
    if (is_array_class() && father->is_array_class()) {
        auto ac1 = (ArrayClass *) this;
        auto ac2 = (ArrayClass *) father;
        if (ac1->dimension == ac2->dimension) {
            return ac1->get_element_class()->is_subclass_of(ac2->get_element_class());
        }
    }

    return false;
}

int Class::inherited_depth() const {
    int depth = 0;
    const Class *c = this->super_class;
    for (; c != nullptr; c = c->super_class) {
        depth++;
    }
    return depth;
}

bool Class::is_prim_class() const { return is_prim_class_name(name); }
bool Class::is_prim_wrapper_class() { return is_prim_wrapper_class_name(name); }

bool Class::is_type_array_class() const {
    if (strlen(name) != 2 || name[0] != '[')
        return false;

    return strchr("ZBCSIFJD", name[1]) != nullptr;
}

Class *Class::get_nest_host() {
    // Determine nest host, the algorithm refers to JVMS 5.4.4

    if (nest_host != nullptr)
        return nest_host; 

    if (nest_host_index < 0) {
        nest_host = this;
        return nest_host;
    }
                
    Class *c = cp->resolve_class(nest_host_index);
    if (c == nullptr || !utf8::equals(c->pkg_name, pkg_name)) {
        nest_host = this;
        return nest_host;
    } 

    vector<void *> &members = c->get_nest_members();
    for (auto mem_cls: members) {
        assert(mem_cls != nullptr);
        if (mem_cls == this) {
            // 'this' is a member nest of Class 'c'
            // so 'c' is the nest_host of 'this' class
            nest_host = c;
            return nest_host;
        }
    }

    nest_host = this;
    return nest_host;
}

void Class::add_nest_member(Class *member) {
    assert(member != nullptr);
    Class *host = get_nest_host();
    if (host != this)
        UNREACHABLE("只有 host class 才可以 add nest member"); // todo
    member->nest_host = this;
    nest_members.push_back(member);
}

vector<void *>& Class::get_nest_members() {
    // Resolve nest members, the algorithm refers to JVMS 5.4.4

    Class *host = get_nest_host();
    assert(host != nullptr);

    if (host->is_nest_members_resolved)
        return host->nest_members;

    for (auto &p: host->nest_members) {
        p = loadClass(loader, (const utf8_t *) p);
    }

    host->nest_members.push_back(host); 
    host->is_nest_members_resolved = true;
    return host->nest_members;
}

bool Class::test_nest_mate(Class *c) {
    assert(c != nullptr);

    // the algorithm refers to JVMS 5.4.4
    return (this == c) || (get_nest_host() == c->get_nest_host());
}
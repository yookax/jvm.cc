module;
#include <assert.h>
#include "../vmdef.h"

module classfile;

import std.core;
import object;
import class_loader;

using namespace std;
using namespace utf8;
using namespace java_lang_Module;

static jref boot_loader_unnamed_module = nullptr;

static vector<DefinedModule> all_modules;

// class Module
static int module_name_id = -1;   // private final String name;
static int module_loader_id = -1; // private final ClassLoader loader;

// class Class
static int class_module_id = -1;  // private transient Module module;

void init_module() {
    Class *c = load_boot_class("java/lang/Module");
    module_name_id = c->get_field("name")->id;
    module_loader_id = c->get_field("loader")->id;

    class_module_id = g_class_class->get_field("module")->id;
}

DefinedModule::DefinedModule(jref module0, jbool is_open0, 
                  jstrRef version0, jstrRef location0, jarrRef packages0)
        : module(module0), is_open(is_open0), version(version0), location(location0) {
    if (packages0 != nullptr) {
        for (jsize i = 0; i < packages0->arr_len; i++) {
            auto pkg = packages0->getElt<jstrRef>(i);
            packages.push_back(java_lang_String::to_utf8(pkg));
        }
    }
}

bool DefinedModule::contain_package(const utf8_t *pkg) const {
    return any_of(packages.begin(), packages.end(),
                  [=](const utf8_t *p){ return equals(p, pkg); });
}

const utf8_t *java_lang_Module::get_name(jref module) {
    assert(module != nullptr);
    jref name = module->get_field_value<jref>(module_name_id);
    return java_lang_String::to_utf8(name);
}

jref java_lang_Module::get_loader(jref module) {
    assert(module != nullptr);
    jref loader = module->get_field_value<jref>(module_loader_id);
    return loader;
}

// find java.base module form all_modules
static jref find_java_base_module() {
    static jref java_base_module = nullptr;

    if (java_base_module == nullptr) {
        for (auto &m: all_modules) {
            if (equals(get_name(m.module), "java.base")) {
                java_base_module = m.module;
                break;
            }
        }
    }

    return java_base_module;
}

/*
 * 遍历已经加载的所有类，每个遍历出的类被命名为`clazz`
 *
 * 比如要输出所有已经加载的所有类的类名，可以使用以下代码：
 * TRAVERSE_ALL_LOADED_CLASSES({
 *     std::cout << c->toString().c_str() << std::endl;
 * });
 */
#define ALL_LOADED_CLASSES(clazz, code_block) \
    do { \
        const std::unordered_set<const Object *> &_loaders_ = getAllClassLoaders(); \
        for (auto _loader_: _loaders_) { \
            std::unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *_classes_; \
            \
            if (_loader_ == BOOT_CLASS_LOADER) { \
                _classes_ = getAllBootClasses(); \
            } else { \
                _classes_ = _loader_->classes; \
            } \
            assert(_classes_ != nullptr); \
            \
            for (auto &_p_: *_classes_) { \
                Class *clazz = _p_.second; \
                code_block \
            } \
        } \
    } while(false)


void define_module_to_vm(jref module, jbool is_open,
                        jstrRef version, jstrRef location, jarrRef packages) {
    auto &m = all_modules.emplace_back(module, is_open, version, location, packages);
    // printvm("%s\n", getName(m.module));

    // if (g_vm_initing) { todo 在initPhase2阶段初始化模块系统。在初始化成功后还需不需要下面的遍历？
        ALL_LOADED_CLASSES(c, {
            if (c->java_mirror->get_field_value<jref>(class_module_id) == nullptr) {
                if (c->is_prim_class() || c->is_array_class()) {
                    set_class_module(c);
                } else if (m.contain_package(c->pkg_name)) {
                    c->java_mirror->set_field_value<jref>(class_module_id, m.module);
                }
            }
        });
    // }
}

/*
 * Sets the module that the class or interface is a member of.
 *
 * If this class represents an array type then this method sets the
 * Module for the element type. 
 * 
 * If this class represents a primitive type or void, 
 * then the Module object for the java.base module is setted.
 *
 * If this class is in an unnamed module then the 
 * ClassLoader.getUnnamedModule() unnamed Module of the class
 * loader for this class is setted.
 * 
 * @return the module that this class or interface is a member of
 */
jref set_class_module(Class *c) {
    assert(c != nullptr);

    if (class_module_id < 0) // todo
        return nullptr;
    
    jref module = c->java_mirror->get_field_value<jref>(class_module_id);
    if (module != nullptr)
        return module;

    if (c->is_prim_class()) {
        module = find_java_base_module();
        goto find_out;
        
    }
    
    if (c->is_array_class()) {
        Class *ec = ((ArrayClass *) c)->get_element_class();
        module = set_class_module(ec);
        goto find_out;
    }

    for (auto &m: all_modules) {
        if (m.contain_package(c->pkg_name)) {
            module = m.module;
            goto find_out;
        }
    }

find_out:
// printf("%p\n", module);
    c->java_mirror->set_field_value<jref>(class_module_id, module);
    return module; 
}

void set_boot_loader_unnamed_module(jref module) {
    boot_loader_unnamed_module = module;
    // printvm("%p\n", boot_loader_unnamed_module);
    // todo
}


ModuleAttribute::ModuleAttribute(ConstantPool &cp, BytesReader &r) {
    module_name = cp.module_name(r.readu2());
    module_flags = r.readu2();
    u2 v = r.readu2();
    module_version = v == 0 ? nullptr : cp.utf8(v);

    u2 requires_count = r.readu2();
    for (u2 j = 0; j < requires_count; j++) {
        _requires.emplace_back(cp, r);
    }

    u2 exports_count = r.readu2();
    for (u2 j = 0; j < exports_count; j++) {
        exports.emplace_back(cp, r);
    }

    u2 opens_count = r.readu2();
    for (u2 j = 0; j < opens_count; j++) {
        opens.emplace_back(cp, r);
    }

    u2 uses_count = r.readu2();
    for (u2 j = 0; j < uses_count; j++) {
        uses.push_back(cp.class_name(r.readu2()));
    }

    u2 provides_count = r.readu2();
    for (u2 j = 0; j < provides_count; j++) {
        provides.emplace_back(cp, r);
    }
}

ModuleAttribute::Require::Require(ConstantPool &cp, BytesReader &r) {
    require_module_name = cp.module_name(r.readu2());
    flags = r.readu2();
    u2 v = r.readu2();
    version = v == 0 ? nullptr : cp.utf8(v);
}

ModuleAttribute::Export::Export(ConstantPool &cp, BytesReader &r) {
    export_package_name = cp.package_name(r.readu2());
    flags = r.readu2();
    u2 exports_to_count = r.readu2();
    for (u2 i = 0; i < exports_to_count; i++) {
        exports_to.push_back(cp.module_name(r.readu2()));
    }
}

ModuleAttribute::Open::Open(ConstantPool &cp, BytesReader &r) {
    open_package_name = cp.package_name(r.readu2());
    flags = r.readu2();
    u2 exports_to_count = r.readu2();
    for (u2 i = 0; i < exports_to_count; i++) {
        opens_to.push_back(cp.module_name(r.readu2()));
    }
}

ModuleAttribute::Provide::Provide(ConstantPool &cp, BytesReader &r) {
    class_name = cp.class_name(r.readu2());
    u2 provides_with_count = r.readu2();
    for (u2 i = 0; i < provides_with_count; i++) {
        provides_with.push_back(cp.class_name(r.readu2()));
    }
}
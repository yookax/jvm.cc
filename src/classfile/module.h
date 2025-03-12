#ifndef CABIN_MODULE_H
#define CABIN_MODULE_H

#include <vector>
#include "../cabin.h"
#include "bytecode_reader.h"

class Class;
class ConstantPool;

struct DefinedModule {
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

void define_module_to_vm(jref module, jbool is_open,
                        jstrRef version, jstrRef location, jarrRef packages);

jref set_class_module(Class *c);

void init_module();

void set_boot_loader_unnamed_module(jref module);

namespace java_lang_Module {
    const utf8_t *get_name(jref module);
    jref get_loader(jref module);
}

struct ModuleAttribute {
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

#endif

module;
#include "../vmdef.h"

export module module0;

import std.core;
import bytes_reader;
import constant_pool;

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

        explicit Require(ConstantPool &cp, BytesReader &r);
    };
    std::vector<Require> _requires;

    struct Export {
        const utf8_t *export_package_name;
        u2 flags;
        std::vector<const utf8_t *> exports_to;

        explicit Export(ConstantPool &cp, BytesReader &r);
    };
    std::vector<Export> exports;

    struct Open {
        const utf8_t *open_package_name;
        u2 flags;
        std::vector<const utf8_t *> opens_to;

        explicit Open(ConstantPool &cp, BytesReader &r);
    };
    std::vector<Open> opens;

    std::vector<const utf8_t *> uses;

    struct Provide {
        const utf8_t *class_name;
        std::vector<const utf8_t *> provides_with;

        explicit Provide(ConstantPool &cp, BytesReader &r);
    };
    std::vector<Provide> provides;

    explicit ModuleAttribute(ConstantPool &cp, BytesReader &r);
};
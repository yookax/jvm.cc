module;
#include "../../../../vmdef.h"

module native;

import std.core;
import slot;
import classfile;
import object;
import runtime;
import class_loader;

/*
 * Returns an array of the binary name of the packages defined by
 * the boot loader, in VM internal form (forward slashes instead of dot).
 */
// private static native String[] getSystemPackageNames();
void getSystemPackageNames(Frame *f) {
    utf8_set &packages = get_boot_packages();
    auto size = packages.size();

    auto ao = Allocator::string_array(size);
    auto p = (Object **) ao->data;
    for (auto pkg : packages) {
        *p++ = Allocator::string(pkg);
    }

    f->pushr(ao);
}

/*
 * Returns the location of the package of the given name, if
 * defined by the boot loader; otherwise {@code null} is returned.
 *
 * The location may be a module from the runtime image or exploded image,
 * or from the boot class append path (i.e. -Xbootclasspath/a or
 * BOOT-CLASS-PATH attribute specified in java agent).
 */
// private static native String getSystemPackageLocation(String name);
void getSystemPackageLocation(Frame *f) {
    slot_t *args = f->lvars;
    auto name = slot::get<jref>(args);

    const char *utf8_name = java_lang_String::to_utf8(name);
    const char *pkg = get_boot_package(utf8_name);
    if (pkg == nullptr) {
        f->pushr(nullptr);
    } else {
        f->pushr(Allocator::string(pkg));
    }
}

// private static native void setBootLoaderUnnamedModule0(Module module);
void setBootLoaderUnnamedModule0(Frame *f) {
    slot_t *args = f->lvars;
    auto module = slot::get<jref>(args);

    set_boot_loader_unnamed_module(module);
}

void jdk_internal_loader_BootLoader_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/loader/BootLoader", #method, method_descriptor, method)

    R(getSystemPackageNames, "()[Ljava/lang/String;");
    R(getSystemPackageLocation, "(Ljava/lang/String;)Ljava/lang/String;");
    R(setBootLoaderUnnamedModule0, "(Ljava/lang/Module;)V");
}
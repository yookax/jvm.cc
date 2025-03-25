module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;

/*
 * Define a module with the specified packages and bind the module to the
 * given class loader.
 *  module:       module to define
 *  is_open:      specifies if module is open (currently ignored)
 *  version:      the module version
 *  location:     the module location
 *  packages:     array of packages in the module
 */
//private static native void defineModule0(Module module, boolean isOpen,
//                                  String version, String location, Object[] packages);
void defineModule0(Frame *f) {
    slot_t *args = f->lvars;
    auto module = slot::get<jref>(args++);
    auto is_open = slot::get<jbool>(args++);
    auto version = slot::get<jref>(args++);
    auto location = slot::get<jref>(args++);
    auto packages = slot::get<jref>(args++);

    // auto x = java_lang_String::toUtf8(version);
    // auto u = java_lang_String::toUtf8(location);


    // auto name = java_lang_Module::getName(module);
    // auto loader = java_lang_Module::getLoader(module);

    define_module_to_vm(module, is_open, version, location, packages);
}

/*
 * Add a module to the list of modules that a given module can read.
 *  from_module:   module requesting read access
 *  source_module: module that from_module wants to read
 */
//private static native void addReads0(Module from_module, Module source_module);
void addReads0(Frame *f) {
    // todo
}

/*
 * Do a qualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 *  to_module:   module to export the package to
 */
//private static native void addExports0(Module from_module, String package, Module to_module);
void addExports0(Frame *f) {
    // todo
}

/*
 * Do an unqualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 */
//private static native void addExportsToAll0(Module from_module, String package);
void addExportsToAll0(Frame *f) {
    // todo
}

/*
 * Do an export of a package to all unnamed modules.
 *  from_module: module containing the package to export
 *  package:     name of the package to export to all unnamed modules
 */
//private static native void addExportsToAllUnnamed0(Module from_module, String package);
void addExportsToAllUnnamed0(Frame *f) {
    unimplemented
}

void java_lang_Module_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Module", #method, method_descriptor, method)

    R(defineModule0, "(Ljava/lang/Module;ZLjava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)V");
    R(addReads0, "(Ljava/lang/Module;Ljava/lang/Module;)V");
    R(addExports0, "(Ljava/lang/Module;Ljava/lang/String;Ljava/lang/Module;)V");
    R(addExportsToAll0, "(Ljava/lang/Module;Ljava/lang/String;)V");
    R(addExportsToAllUnnamed0, "(Ljava/lang/Module;Ljava/lang/String;)V");
}
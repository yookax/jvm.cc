module;
#include "../../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

/*
 * Returns the available VM and Command Line Properties.
 * The VM supplies some key/value pairs and processes the command line
 * to extract key/value pairs from the {@code "-Dkey=value"} arguments.
 *
 * @return an array of strings, with alternating key and value strings.
 *      Either keys or values may be null, the array may not be full.
 *      The first null key indicates there are no more key, value pairs.
 */
// private static native String[] vmProperties();
void vmProperties(Frame *f) {
    jarrRef prop_array = Allocator::string_array(g_properties.size()*2);
    int i = 0;
    for (Property &p : g_properties) {
        prop_array->setRefElt(i++, Allocator::string(p.name));
        prop_array->setRefElt(i++, Allocator::string(p.value));
    }
    f->pushr(prop_array);
}

/*
 * Returns the platform specific property values identified
 * by {@code "_xxx_NDX"} indexes.
 * The indexes are strictly private, to be shared only with the native code.
 *
 * @return a array of strings, the properties are indexed by the {@code _xxx_NDX}
 * indexes.  The values are Strings and may be null.
 */
// private static native String[] platformProperties();
void platformProperties(Frame *f) {
    jarrRef prop_array = Allocator::string_array(g_properties.size()*2);
    int i = 0;
    for (Property &p : g_properties) {
        prop_array->setRefElt(i++, Allocator::string(p.name));
        prop_array->setRefElt(i++, Allocator::string(p.value));
    }
    f->pushr(prop_array);
}

void jdk_internal_util_SystemProps$Raw_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/util/SystemProps$Raw", #method, method_descriptor, method)

    R(vmProperties, "()[Ljava/lang/String;");
    R(platformProperties, "()[Ljava/lang/String;");
}
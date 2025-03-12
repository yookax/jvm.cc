#include <cassert>
#include "class.h"
#include "class_loader.h"
#include "invoke.h"

import std.core;

using namespace std;
using namespace java_lang_invoke_MethodHandle;

/*
 * A method is signature polymorphic（签名多态性） if all of the following are true:
 * 1. It is declared in the java.lang.invoke.MethodHandle class
 *      or the java.lang.invoke.VarHandle class.
 * 2. It has a single formal parameter of type Object[].
 * 3. It has the ACC_VARARGS and ACC_NATIVE flags set.
 */

struct PolyMethodInfo {
    Class *c;

    // polymorphic method name, access flag, native method.
    vector<tuple<const utf8_t *, int, void *>> poly;

    explicit PolyMethodInfo(Class *c0): c(c0) { assert(c != nullptr); }
};

static vector<PolyMethodInfo> poly_method_infos;

void MethodHandle_registerPolymorphicMethods(vector<tuple<const utf8_t *, int, void *>> &polys);
void VarHandle_registerPolymorphicMethods(vector<tuple<const utf8_t *, int, void *>> &polys);

void init_polymorphic_method() {
    Class *c = load_boot_class("java/lang/invoke/MethodHandle");
    PolyMethodInfo &info = poly_method_infos.emplace_back(c);
    MethodHandle_registerPolymorphicMethods(info.poly);

    // ---------------------------------------------------------

    c = load_boot_class("java/lang/invoke/VarHandle");
    PolyMethodInfo &info0 = poly_method_infos.emplace_back(c);
    VarHandle_registerPolymorphicMethods(info0.poly);
}

tuple<Class *, int, void *> lookup_polymorphic_method(Class *c, const utf8_t *name) {
    for (PolyMethodInfo &info: poly_method_infos) {
        if (c->is_subclass_of(info.c)) {
            for (auto &x: info.poly) {
                if (utf8::equals(get<0>(x), name)) {
                    return make_tuple(info.c, get<1>(x), get<2>(x));
                }
            }
        }
    }

    return make_tuple(nullptr, -1, nullptr);
}

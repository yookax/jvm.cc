module;
#include <cassert>
#include "../vmdef.h"

module poly;

import std.core;
import classfile;
import invoke;
import encoding;
import class_loader;
import access_flags;

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
    vector<pair<const utf8_t *, int>> poly;

    explicit PolyMethodInfo(Class *c0): c(c0) { assert(c != nullptr); }
};

static vector<PolyMethodInfo> poly_method_infos;

void init_polymorphic_method() {
    Class *c = load_boot_class("java/lang/invoke/MethodHandle");
    PolyMethodInfo &info = poly_method_infos.emplace_back(c);

    int acc = JVM_ACC_NATIVE | JVM_ACC_VARARGS | JVM_ACC_FINAL;
    info.poly.emplace_back("invokeBasic", acc);

    acc |= JVM_ACC_PUBLIC;
    info.poly.emplace_back("invokeExact", acc);
    info.poly.emplace_back("invoke",      acc);

    acc = JVM_ACC_NATIVE | JVM_ACC_VARARGS | JVM_ACC_STATIC;
    info.poly.emplace_back("linkToVirtual",   acc);
    info.poly.emplace_back("linkToStatic",    acc);
    info.poly.emplace_back("linkToSpecial",   acc);
    info.poly.emplace_back("linkToInterface", acc);
    info.poly.emplace_back("linkToNative",    acc);

    // ---------------------------------------------------------

    c = load_boot_class("java/lang/invoke/VarHandle");
    PolyMethodInfo &info0 = poly_method_infos.emplace_back(c);

    acc = JVM_ACC_PUBLIC| JVM_ACC_FINAL | JVM_ACC_NATIVE | JVM_ACC_VARARGS;
    info0.poly.emplace_back("get", acc);
    info0.poly.emplace_back("set", acc);
    info0.poly.emplace_back("getVolatile", acc);
    info0.poly.emplace_back("setVolatile", acc);
    info0.poly.emplace_back("getOpaque", acc);
    info0.poly.emplace_back("setOpaque", acc);
    info0.poly.emplace_back("getAcquire", acc);
    info0.poly.emplace_back("setRelease", acc);
    info0.poly.emplace_back("compareAndSet", acc);
    info0.poly.emplace_back("compareAndExchange", acc);
    info0.poly.emplace_back("compareAndExchangeAcquire", acc);
    info0.poly.emplace_back("compareAndExchangeRelease", acc);
    info0.poly.emplace_back("weakCompareAndSetPlain", acc);
    info0.poly.emplace_back("weakCompareAndSet", acc);
    info0.poly.emplace_back("weakCompareAndSetAcquire", acc);
    info0.poly.emplace_back("weakCompareAndSetRelease", acc);
    info0.poly.emplace_back("getAndSet", acc);
    info0.poly.emplace_back("getAndSetAcquire", acc);
    info0.poly.emplace_back("getAndSetRelease", acc);
    info0.poly.emplace_back("getAndAdd", acc);
    info0.poly.emplace_back("getAndAddAcquire", acc);
    info0.poly.emplace_back("getAndAddRelease", acc);
    info0.poly.emplace_back("getAndBitwiseOr", acc);
    info0.poly.emplace_back("getAndBitwiseOrAcquire", acc);
    info0.poly.emplace_back("getAndBitwiseOrRelease", acc);
    info0.poly.emplace_back("getAndBitwiseAnd", acc);
    info0.poly.emplace_back("getAndBitwiseAndAcquire", acc);
    info0.poly.emplace_back("getAndBitwiseAndRelease", acc);
    info0.poly.emplace_back("getAndBitwiseXor", acc);
    info0.poly.emplace_back("getAndBitwiseXorAcquire", acc);
    info0.poly.emplace_back("getAndBitwiseXorRelease", acc);
}

pair<Class *, int> lookup_polymorphic_method(Class *c, const utf8_t *name) {
    for (PolyMethodInfo &info: poly_method_infos) {
        if (c->is_subclass_of(info.c)) {
            for (auto &x: info.poly) {
                if (utf8::equals(get<0>(x), name)) {
                    return make_pair(info.c, get<1>(x));
                }
            }
        }
    }

    return make_pair(nullptr, -1);
}

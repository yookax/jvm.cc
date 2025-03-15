#include "../vmdef.h"

import std.core;
import slot;
import invoke;
import access_flags;
import constants;

using namespace std;
using namespace slot;

void MethodHandle_registerPolymorphicMethods(vector<tuple<const utf8_t *, int, void *>> &polys) {
    int acc = JVM_ACC_NATIVE | JVM_ACC_VARARGS | JVM_ACC_FINAL;
    polys.emplace_back("invokeBasic", acc, (void *) java_lang_invoke_MethodHandle::invokeBasic);

    acc |= JVM_ACC_PUBLIC;
    polys.emplace_back("invokeExact", acc, (void *) java_lang_invoke_MethodHandle::invokeExact);
    polys.emplace_back("invoke",      acc, (void *) java_lang_invoke_MethodHandle::invoke);

    acc = JVM_ACC_NATIVE | JVM_ACC_VARARGS | JVM_ACC_STATIC;
    polys.emplace_back("linkToVirtual",   acc, (void *) java_lang_invoke_MethodHandle::linkToVirtual);
    polys.emplace_back("linkToStatic",    acc, (void *) java_lang_invoke_MethodHandle::linkToStatic);
    polys.emplace_back("linkToSpecial",   acc, (void *) java_lang_invoke_MethodHandle::linkToSpecial);
    polys.emplace_back("linkToInterface", acc, (void *) java_lang_invoke_MethodHandle::linkToInterface);
    polys.emplace_back("linkToNative",    acc, (void *) java_lang_invoke_MethodHandle::linkToNative);
}

#include <algorithm>
#include "cabin.h"
#include "classfile/class_loader.h"
#include "interpreter.h"

using namespace std;
using namespace utf8;

/*
 * 基本类型的名称，描述符，等等
 */

static struct Primitive {
    const utf8_t *class_name;
    utf8_t descriptor;
    const utf8_t *array_class_name;
    const utf8_t *wrapper_class_name;
} prims[] = {
        { "void",    'V', "[V", "java/lang/Void" },
        { "boolean", 'Z', "[Z", "java/lang/Boolean" },
        { "byte",    'B', "[B", "java/lang/Byte" },
        { "char",    'C', "[C", "java/lang/Character" },
        { "short",   'S', "[S", "java/lang/Short" },
        { "int",     'I', "[I", "java/lang/Integer" },
        { "long",    'J', "[J", "java/lang/Long" },
        { "float",   'F', "[F", "java/lang/Float" },
        { "double",  'D', "[D", "java/lang/Double" }
};

bool is_prim_class_name(const utf8_t *class_name) {
    assert(class_name != nullptr);
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return equals(prim.class_name, class_name); }) != end(prims);
}

bool is_prim_descriptor(utf8_t descriptor) {
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return prim.descriptor == descriptor; }) != end(prims);
}

bool is_prim_wrapper_class_name(const utf8_t *class_name) {
    assert(class_name != nullptr);
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return equals(prim.wrapper_class_name, class_name); }) != end(prims);
}

const utf8_t *get_prim_array_class_name(const utf8_t *class_name) {
    assert(class_name != nullptr);
    for (auto &t : prims) {
        if (equals(t.class_name, class_name))
            return t.array_class_name;
    }
    return nullptr;
}

const utf8_t *get_prim_class_name(utf8_t descriptor) {
    for (auto &t : prims) {
        if (t.descriptor == descriptor)
            return t.class_name;
    }
    return nullptr;
}

const utf8_t *getPrimDescriptor(const utf8_t *wrapper_class_name) {
    for (auto &t : prims) {
        if (equals(t.wrapper_class_name, wrapper_class_name))
            return &(t.descriptor);
    }
    return nullptr;
}

const utf8_t *get_prim_descriptor_by_class_name(const utf8_t *class_name) {
    for (auto &t : prims) {
        if (equals(t.class_name, class_name))
            return &(t.descriptor);
    }
    return nullptr;
}

const utf8_t *get_prim_descriptor_by_wrapper_class_name(const utf8_t *wrapper_class_name) {
    for (auto &t : prims) {
        if (equals(t.wrapper_class_name, wrapper_class_name))
            return &(t.descriptor);
    }
    return nullptr;
}
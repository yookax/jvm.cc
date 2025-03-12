module;
#include <cassert>

export module primitive;

import std.core;

using namespace std;

/*
 * 基本类型的名称，描述符，等等
 */

static struct Primitive {
    const char *class_name;
    char descriptor;
    const char *array_class_name;
    const char *wrapper_class_name;
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

export bool is_prim_class_name(const char *class_name) {
    assert(class_name != nullptr);
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return strcmp(prim.class_name, class_name) == 0; }) != end(prims);
}

export bool is_prim_descriptor(char descriptor) {
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return prim.descriptor == descriptor; }) != end(prims);
}

export bool is_prim_wrapper_class_name(const char *class_name) {
    assert(class_name != nullptr);
    return find_if(begin(prims), end(prims),
                   [=](auto &prim){ return strcmp(prim.wrapper_class_name, class_name) == 0; }) != end(prims);
}

export const char *get_prim_array_class_name(const char *class_name) {
    assert(class_name != nullptr);
    for (auto &t : prims) {
        if (strcmp(t.class_name, class_name) == 0)
            return t.array_class_name;
    }
    return nullptr;
}

export const char *get_prim_class_name(char descriptor) {
    for (auto &t : prims) {
        if (t.descriptor == descriptor)
            return t.class_name;
    }
    return nullptr;
}

export const char *getPrimDescriptor(const char *wrapper_class_name) {
    for (auto &t : prims) {
        if (strcmp(t.wrapper_class_name, wrapper_class_name) == 0)
            return &(t.descriptor);
    }
    return nullptr;
}

export const char *get_prim_descriptor_by_class_name(const char *class_name) {
    for (auto &t : prims) {
        if (strcmp(t.class_name, class_name) == 0)
            return &(t.descriptor);
    }
    return nullptr;
}

export const char *get_prim_descriptor_by_wrapper_class_name(const char *wrapper_class_name) {
    for (auto &t : prims) {
        if (strcmp(t.wrapper_class_name, wrapper_class_name) == 0)
            return &(t.descriptor);
    }
    return nullptr;
}
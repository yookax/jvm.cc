module;
#include <cassert>

export module primitive;

import std.core;

using namespace std;

/*
 * 基本类型的名称，描述符，等等
 */

const struct Primitive {
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

export struct PRIMITIVE {
    static bool checkClassName(const char *class_name) {
        assert(class_name != nullptr);
        return ranges::any_of(prims, [=](auto &p){
            return strcmp(p.class_name, class_name) == 0;
        });
    }

    static bool checkDescriptor(char descriptor) {
        return ranges::any_of(prims, [=](auto &p){
            return p.descriptor == descriptor;
        });
    }

    static bool checkWrapperClassName(const char *class_name) {
        assert(class_name != nullptr);
        return ranges::any_of(prims, [=](auto &p){
            return strcmp(p.wrapper_class_name, class_name) == 0;
        });
    }

    // Class name to Array class name
    static const char *c2a(const char *class_name) {
        assert(class_name != nullptr);
        for (auto &t : prims) {
            if (strcmp(t.class_name, class_name) == 0)
                return t.array_class_name;
        }
        return nullptr;
    }

    // Descriptor to Class name
    static const char *d2c(char descriptor) {
        for (auto &t : prims) {
            if (t.descriptor == descriptor)
                return t.class_name;
        }
        return nullptr;
    }

    // Class name to Descriptor
    static const char *c2d(const char *class_name) {
        for (auto &t : prims) {
            if (strcmp(t.class_name, class_name) == 0)
                return &(t.descriptor);
        }
        return nullptr;
    }

    // Wrapper class name to Descriptor
    static const char *w2d(const char *wrapper_class_name) {
        for (auto &t : prims) {
            if (strcmp(t.wrapper_class_name, wrapper_class_name) == 0)
                return &(t.descriptor);
        }
        return nullptr;
    }
};
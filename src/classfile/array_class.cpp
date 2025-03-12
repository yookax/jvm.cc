#include <cassert>
#include "../cabin.h"
#include "class.h"
#include "method.h"
#include "../object/object.h"
#include "../runtime/heap.h"

import std.core;

using namespace std;

ArrayClass::ArrayClass(Object *loader0, const char *class_name): Class(loader0) {
    assert(class_name != nullptr);
    assert(class_name[0] == '[');

    name = utf8::dup(class_name); /* 形参class_name可能非持久，复制一份 */
    while (name[dimension] == '[')
        dimension++;
    access_flags = JVM_ACC_PUBLIC;
//    inited = true;
    super_class = g_object_class;

    cp = new ConstantPool(this);

    pkg_name = ""; // todo

    interfaces.push_back(load_boot_class("java/lang/Cloneable"));
    interfaces.push_back(load_boot_class("java/io/Serializable"));

#if 0
    createVtable();  
    createItable();
#endif
    // gen_indep_interfaces(this);
    generate_mssis();

    if (g_class_class != nullptr) {
        generate_class_object();
    }

    state = Class::State::INITED;
}

size_t ArrayClass::get_element_size() {
    if (elt_size == 0) {
        // 判断数组单个元素的大小
        // 除了基本类型的数组外，其他都是引用类型的数组
        // 多维数组是数组的数组，也是引用类型的数组
        char t = name[1]; // jump '['
        if (t == 'Z') {
            elt_size = sizeof(jbool);
        } else if (t == 'B') {
            elt_size = sizeof(jbyte);
        } else if (t == 'C') {
            elt_size = sizeof(jchar);
        } else if (t == 'S') {
            elt_size = sizeof(jshort);
        } else if (t == 'I') {
            elt_size = sizeof(jint);
        } else if (t == 'F') {
            elt_size = sizeof(jfloat);
        } else if (t == 'J') {
            elt_size = sizeof(jlong);
        } else if (t == 'D') {
            elt_size = sizeof(jdouble);
        } else {
            elt_size = sizeof(jref);
        }
    }

    assert(elt_size > 0);
    return elt_size;
}

Class *ArrayClass::get_component_class() {
    if (comp_class != nullptr)
        return comp_class;

    const char *comp_name = name;
    if (*comp_name != '[')
        return nullptr; // not a array

    comp_name++; // jump a '['

    // 判断 component's type
    if (*comp_name == '[') {
        comp_class = load_array_class(loader, comp_name);
        return comp_class;
    }

    const utf8_t *prim_class_name = get_prim_class_name(*comp_name);
    if (prim_class_name != nullptr) {  // primitive type
        comp_class = load_boot_class(prim_class_name);
        return comp_class;
    }

    // 普通类: Lxx/xx/xx; 型
    comp_name++; // jump 'L'
    size_t last = strlen(comp_name) - 1;
    assert(comp_name[last] == ';');
    auto buf = new char[last + 1];
    strncpy(buf, comp_name, last);
    buf[last] = 0;
    comp_class = loadClass(loader, buf);
    delete[] buf;
    return comp_class;
}

Class *ArrayClass::get_element_class() {
    if (elt_class != nullptr)
        return elt_class;

    if (!is_array_class())
        return nullptr;

    ArrayClass *curr = this;
    while(true) {
        Class *cc = curr->get_component_class();
        if (!cc->is_array_class()) {
            elt_class = cc;
            return elt_class;
        }
        curr = (ArrayClass *) cc;
    }

    UNREACHABLE(" "); // todo msg
}

size_t ArrayClass::object_size(jint arr_len) {
    return sizeof(Object) + get_element_size()*arr_len;
}

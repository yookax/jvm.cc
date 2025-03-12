module;
#include <cassert>
#include "../cabin.h"

module classfile;

import std.core;
import vmstd;
import class_loader;

using namespace std;

Field::Field(Class *c, BytecodeReader &r) {
    assert(c != nullptr);

    clazz = c;
    ConstantPool &cp = *(c->cp);

    access_flags = r.readu2();
    name = cp.utf8(r.readu2());
    descriptor = cp.utf8(r.readu2());

    category_two = (descriptor[0] == 'J' || descriptor[0]== 'D');
    deprecated = false;
    signature = nullptr;

    if (isStatic()) {
        memset(&static_value, 0, sizeof(static_value));
    } else {
        id = -1;
    }

    // parse field's attributes
    u2 attr_count = r.readu2();
    for (int i = 0; i < attr_count; i++) {
        const char *attr_name = cp.utf8(r.readu2());
        u4 attr_len = r.readu4();

        if (strcmp("Deprecated", attr_name) == 0) {
            deprecated = true;
        } else if (strcmp("ConstantValue", attr_name) == 0) {
            /*
             * ConstantValue属性表示一个常量字段的值。
             * 在一个field_info结构的属性表中最多只能有一个ConstantValue属性。
             *
             * 非静态字段包含了ConstantValue属性，那么这个属性必须被虚拟机所忽略。
             */
            u2 index = r.readu2();
            if (isStatic()) {
                utf8_t d = *descriptor;
                if (d == 'Z') {
                    static_value.z = JINT_TO_JBOOL(cp.get_int(index));
                } else if (d == 'B') {
                    static_value.b = JINT_TO_JBYTE(cp.get_int(index));
                } else if (d == 'C') {
                    static_value.c = JINT_TO_JCHAR(cp.get_int(index));
                } else if (d == 'S') {
                    static_value.s = JINT_TO_JSHORT(cp.get_int(index));
                } else if (d == 'I') {
                    static_value.i = cp.get_int(index);
                } else if (d == 'J') {
                    static_value.j = cp.get_long(index);
                } else if (d == 'F') {
                    static_value.f = cp.get_float(index);
                } else if (d == 'D') {
                    static_value.d = cp.get_double(index);
                } else if(utf8::equals(descriptor, "Ljava/lang/String;")) {
                    static_value.r = cp.resolve_string(index);
                }
            }
        } else if (strcmp("Synthetic", attr_name) == 0) {
            setSynthetic();
        } else if (strcmp("Signature", attr_name) == 0) {
            signature = cp.utf8(r.readu2());
        } else if (strcmp("RuntimeVisibleAnnotations", attr_name) == 0) {
            rt_visi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleAnnotations", attr_name) == 0) {
            rt_invisi_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeVisibleTypeAnnotations", attr_name) == 0) {
            rt_visi_type_annos.parse(r, attr_len);
        } else if (strcmp("RuntimeInvisibleTypeAnnotations", attr_name) == 0) {
            rt_invisi_type_annos.parse(r, attr_len);
        } else { // unknown attribute
            WARN("unknown attribute: %s\n", attr_name);
            r.skip(attr_len);
        }
    }
}

Field::Field(Class *c, const utf8_t *name0, const utf8_t *descriptor0, int access_flags0) {
    assert(c != nullptr && name0 != nullptr && descriptor0 != nullptr);
    
    clazz = c;
    name = name0;
    descriptor = descriptor0;
    access_flags = access_flags0;

    category_two = (descriptor[0] == 'J' || descriptor[0]== 'D');
    deprecated = false;
    signature = nullptr;

    if (isStatic()) {
        memset(&static_value, 0, sizeof(static_value));
    } else {
        id = -1;
    }
}

jclsRef Field::get_type() {
    if (*descriptor == '[') { // array
        return load_array_class(clazz->loader, descriptor)->java_mirror;
    } 
    
    if (*descriptor == 'L') { // non array Object
        auto buf = new utf8_t[strlen(descriptor)];
        // don't include the first 'L' and the last ';'
        buf[strlen(strcpy(buf, descriptor + 1)) - 1] = 0;
        auto c = loadClass(clazz->loader, buf)->java_mirror;
        delete[] buf;
        return c;
    }

    // primitive
    assert(strlen(descriptor) == 1);
    const utf8_t *n = get_prim_class_name(*descriptor);
    assert(n != nullptr);
    return load_boot_class(n)->java_mirror;
}

bool Field::is_prim_field() const {
    return get_prim_class_name(*descriptor) != nullptr;
}

string Field::toString() const {
    ostringstream oss;
    oss << "field" << (isStatic() ? "(static)" : "(nonstatic)") << ": "; 
    oss << clazz->name << "~" << name << "~" << descriptor << "~" << id;
    return oss.str();
}

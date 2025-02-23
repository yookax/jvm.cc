#include <cassert>
#include <sstream>
#include "../cabin.h"
#include "../runtime//heap.h"
#include "object.h"
#include "allocator.h"
#include "../exception.h"

using namespace std;

Object::Object(Class *c): clazz(c) {
    if (c->is_array_class()) {
        UNREACHABLE(" "); // todo msg
    }
    data = (slot_t *) (this + 1);
}

Object::Object(ArrayClass *ac, jint arr_len0): clazz(ac), arr_len(arr_len0) {
    assert(ac != nullptr);
    assert(arr_len >= 0); // 长度为0的array是合法的
    // todo something else

    data = (slot_t *) (this + 1);
    // int x;
}

Object::Object(ArrayClass *ac, jint dim, const jint lens[]): clazz(ac) {
    assert(ac != nullptr);
    assert(dim >= 1);

    arr_len = lens[0];
    assert(arr_len >= 0); // 长度为0的array是合法的

    data = (slot_t *) (this + 1);

    for (int d = 1; d < dim; d++) {
        for (int i = 0; i < arr_len; i++) {
            auto c = (ArrayClass *) ac->get_component_class();
            auto o = Allocator::multi_array(c, dim - 1, lens + 1);
            setRefElt(i, o);
        }
    }
}

Object *Object::clone() {
    if (clazz == g_class_class) {
        ERR("Object of java.lang.Class don't support clone"); 
        return (jref) this; 
    }

    size_t s = size();
    void *p = g_heap->alloc(s);
    memcpy(p, this, s);

    // todo mutex 怎么处理

    auto clone = (Object *) p;
    clone->data = (slot_t *) (clone + 1);
    return clone;
}

void Object::set_field_value_raw(Field *f, const slot_t *value) {
    assert(f != nullptr && !f->isStatic() && value != nullptr);

    data[f->id] = value[0];
    if (f->category_two) {
        data[f->id + 1] = value[1];
    }
}

void Object::set_field_value_unbox_if_necessary(int field_id, jref value) {
    Field *f = clazz->lookup_field(field_id);
    if (f == nullptr) {
        // todo
    }

    if (f->is_prim_field()) {
        const slot_t *unbox = value->unbox();
        data[field_id] = *unbox;
        if (f->category_two)
            data[field_id+1] = *++unbox;
    } else {
        set_field_value<jref>(f, value);
    }
}

const slot_t *Object::unbox() const {
    Class *c = clazz;
    if (!c->is_prim_wrapper_class()) {
        ERR(" "); // todo msg
        return nullptr;
    }
    if (strcmp(c->name, "java/lang/Void") == 0) {
        ERR(" "); // todo msg
        return nullptr;
    }

    // value 的描述符就是基本类型的类名。比如，private final boolean value;
    Field *f = c->lookup_field("value", get_prim_descriptor_by_wrapper_class_name(c->name));
    return data + f->id;
}

variant<monostate, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble>
Object::box_value() const {
    auto v = unbox();
    if (v == nullptr)
        return std::monostate{};
    if (strcmp(clazz->name, "java/lang/Byte") == 0)
        return *(const jbyte *)v;
    if (strcmp(clazz->name, "java/lang/Boolean") == 0)
        return *(const jbyte *)v; // jbool是一个类型
    if (strcmp(clazz->name, "java/lang/Character") == 0)
        return *(const jchar *)v;
    if (strcmp(clazz->name, "java/lang/Short") == 0)
        return *(const jshort *)v;
    if (strcmp(clazz->name, "java/lang/Integer") == 0)
        return *(const jint *)v;
    if (strcmp(clazz->name, "java/lang/Float") == 0)
        return *(const jfloat *)v;
    if (strcmp(clazz->name, "java/lang/Long") == 0)
        return *(const jlong *)v;
    if (strcmp(clazz->name, "java/lang/Double") == 0)
        return *(const jdouble *)v;

    UNREACHABLE("");
}

size_t Object::size() const {
    assert(clazz != nullptr);

    if (is_array_object()) {
        return ((ArrayClass *)clazz)->object_size(arr_len);
    } else {
        return clazz->object_size();
    }
}

void Object::display(string prefix_space) {
    if (!is_array_object()) {
        printvm("Not a array.\n");
        return;
    }

    jint len = array_len();
    jint dim = array_dimension();

    if (dim == 1) {
        function<string(const void *)> func = nullptr;
        if (is_bool_array())
            func = [](const void *e) { return std::to_string(*(jbool *) e); };
        else if (is_byte_array())
            func = [](const void *e) { return std::to_string(*(const jbyte *) e); };
        else if (is_char_array())
            func = [](const void *e) { return std::to_string(*(const jchar *) e); };
        else if (is_short_array())
            func = [](const void *e) { return std::to_string(*(const jshort *) e); };
        else if (is_int_array())
            func = [](const void *e) { return std::to_string(*(const jint *) e); };
        else if (is_float_array())
            func = [](const void *e) { return std::to_string(*(const jfloat *) e); };
        else if (is_long_array())
            func = [](const void *e) { return std::to_string(*(const jlong *) e); };
        else if (is_double_array())
            func = [](const void *e) { return std::to_string(*(const jdouble *) e); };
        else {
            func = [](const void *e) {
                if (*(const intptr_t *) e == 0) {
                    return std::string("null");
                }

                jref o = * (const jref *)e;
                if (o->is_string_object()) {
                    auto utf8 = java_lang_String::to_utf8(o);
                    string str = "\"";
                    str.append(utf8).append("\"");
                    return str;
                } else {
                    char buffer[1024];
                    sprintf(buffer, "%p", o);
                    return std::string(buffer);
                }
            };
        }

        printf("%s[", prefix_space.c_str());
        for (jint i = 0; i < len; i++) {
            printf("%s", func(index(i)).c_str());
            if (i < len - 1)
                printf(", ");
        }
        printf("]\n");
        return;
    }

    printf("%s[\n", prefix_space.c_str());

    for (jint i = 0; i < len; i++) {
        getElt<jarrRef>(i)->display(prefix_space + "    ");
    }

    printf("%s]\n", prefix_space.c_str());
}

string Object::toString() const {
    ostringstream oss;
    oss << "Object(" << (uintptr_t) this << "), " << clazz->name;
    return oss.str();
}

module;
#include <cassert>
#include "../vmdef.h"

module object;

import vmstd;
import classfile;
import class_loader;

using namespace std;
using namespace slot;
using namespace java_lang_String;

static Class *field_reflect_class;       // java.lang.reflect.Field
static int fld_class_id;                 // private Class<?> clazz;
static int fld_slot_id;                  // private int slot;

static Class *method_reflect_class;      // java.lang.reflect.Method
static int mthd_class_id;                // private Class<?> clazz;
static int mthd_slot_id;                 // private int slot;
static int mthd_name_id;                 // private String name;
static int mthd_returnType_id;           // private Class<?> returnType;
static int mthd_parameterTypes_id;       // private Class<?>[] parameterTypes;

static Class *constructor_reflect_class; // java.lang.reflect.Constructor
static int cons_class_id;                // private Class<?> clazz;
static int cons_slot_id;                 // private int slot;
static int cons_parameterTypes_id;       // private Class<?>[] parameterTypes;
 
void init_reflection() {
    field_reflect_class = load_boot_class("java/lang/reflect/Field");
    method_reflect_class = load_boot_class("java/lang/reflect/Method");
    constructor_reflect_class = load_boot_class("java/lang/reflect/Constructor");

    fld_class_id = field_reflect_class->get_field("clazz", "Ljava/lang/Class;")->id;
    fld_slot_id = field_reflect_class->get_field("slot", "I")->id;
    
    mthd_class_id = method_reflect_class->get_field("clazz", "Ljava/lang/Class;")->id;
    mthd_slot_id = method_reflect_class->get_field("slot", "I")->id;
    mthd_name_id = method_reflect_class->get_field("name", "Ljava/lang/String;")->id;
    mthd_returnType_id = method_reflect_class->get_field("returnType", "Ljava/lang/Class;")->id;
    mthd_parameterTypes_id 
                = method_reflect_class->get_field("parameterTypes", "[Ljava/lang/Class;")->id;
    
    cons_class_id = constructor_reflect_class->get_field("clazz", "Ljava/lang/Class;")->id;
    cons_slot_id = constructor_reflect_class->get_field("slot", "I")->id;
    cons_parameterTypes_id 
                = constructor_reflect_class->get_field("parameterTypes", "[Ljava/lang/Class;")->id;
}

jarrRef get_inner_classes_as_class_array(Class *c, bool public_only) {
    assert(c != nullptr);

//    Class *inners[c->inner_classes.size()];
    auto inners = new Class *[c->inner_classes.size()];
    int count = 0;

    for (auto &x: c->inner_classes) {
        Class *inner = (Class *) x.second;

        if (!x.first) { // not resolved
            inner = c->cp->resolve_class((u2) x.second);
            assert(inner != nullptr);
            x.first = true;
            x.second = (uintptr_t) inner;
        } 

        assert(inner != nullptr);
        if (!public_only || accIsPublic(inner->inner_access_flags)) {
            inners[count++] = inner;
        }
    }

    jarrRef result = Allocator::class_array(count);
    for (int i = 0; i < count; i++) {
        result->setRefElt(i, inners[i]->java_mirror);
    }

    delete[] inners;
    return result;
}

Class *get_declaring_class(Class *c) {
    assert(c != nullptr);
    if (c->declaring_class == 0)
        return nullptr;
    return c->cp->resolve_class(c->declaring_class);    
}

jint field_offset(jref fo) {
    assert(fo != nullptr);
    return fo->get_field_value<jint>(fld_slot_id);
}

jint field_offset(Field *f) {
    assert(f != nullptr);
    if (!f->isStatic())
        return f->id;

    for (size_t i = 0; i < f->clazz->fields.size(); i++) {
        if (utf8::equals(f->name, f->clazz->fields[i]->name))
            return i;
    }   
    
    return -1; 
}

Field *get_field_from_reflect_object(jref fo) {
    Class *decl_class = fo->get_field_value<jref>(fld_class_id)->jvm_mirror;
    jint slot = fo->get_field_value<jint>(fld_slot_id);
    for (Field *f : decl_class->fields) {
        if (f->id == slot)  // 参考函数 getDeclaredFieldsAsReflectArray
            return f;
    }
    
    UNREACHABLE(" "); // todo
}

Method *get_method_from_reflect_object(jref mo) {
    bool is_cons = (mo->clazz == constructor_reflect_class);
    Class *decl_class = mo->get_field_value<jref>(is_cons ? cons_class_id : mthd_class_id)->jvm_mirror;
    jint slot = mo->get_field_value<jint>(is_cons ? cons_slot_id : mthd_slot_id);
    Method *m = decl_class->methods.at(slot);
    return m;
}

jarrRef get_annotation_as_byte_array(Annotation& a) {
    if (a.data == nullptr)
        return nullptr;

    jarrRef arr = Allocator::array("[B", a.len);
    memcpy(arr->data, a.data, a.len);
    return arr;    
}

jref invoke_method(jref mo, jref _this, jarrRef args) {
    Class *c = mo->get_field_value<jref>(mthd_class_id)->jvm_mirror;
    auto name = to_utf8(mo->get_field_value<jref>(mthd_name_id));
    jref rtype = mo->get_field_value<jref>(mthd_returnType_id);
    jref ptypes = mo->get_field_value<jref>(mthd_parameterTypes_id);

    string desc = unparseMethodDescriptor(ptypes, rtype);
    Method *m;
    if (_this != nullptr) // instance method
        m = _this->clazz->lookup_method(name, desc.c_str());
    else // static method
        m = c->lookup_method(name, desc.c_str());
    assert(m != nullptr);

    slot_t *result = execJava(m, _this, args);
    switch (m->ret_type) {
        case Method::RET_VOID:      return void_box();
        case Method::RET_BYTE:      return byte_box(slot::get<jbyte>(result));
        case Method::RET_BOOL:      return bool_box(slot::get<jbool>(result));
        case Method::RET_CHAR:      return char_box(slot::get<jchar>(result));
        case Method::RET_SHORT:     return short_box(slot::get<jshort>(result));
        case Method::RET_INT:       return int_box(slot::get<jint>(result));
        case Method::RET_FLOAT:     return float_box(slot::get<jfloat>(result));
        case Method::RET_LONG:      return long_box(slot::get<jlong>(result));
        case Method::RET_DOUBLE:    return double_box(slot::get<jdouble>(result));
        case Method::RET_REFERENCE: return slot::get<jref>(result);
        case Method::RET_INVALID:
        default:                    UNREACHABLE("%d\n", m->ret_type);
    }
}

jref new_instance_from_constructor(jref co, jarrRef args) {
    Class *clazz = co->get_field_value<jref>(cons_class_id)->jvm_mirror;
    jarrRef parameter_types = co->get_field_value<jref>(cons_parameterTypes_id);
    
    Method *constructor = clazz->get_constructor(parameter_types);
    jref new_instance = Allocator::object(clazz);
    execJava(constructor, new_instance, args); 
    return new_instance;
}
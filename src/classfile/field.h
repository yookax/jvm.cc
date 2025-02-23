#ifndef CABIN_FIELD_H
#define CABIN_FIELD_H

#include <vector>
#include <string>
#include "../cabin.h"
#include "meta.h"

class Field: public Meta {
public:
    Class *clazz = nullptr;
    const utf8_t *descriptor = nullptr;

    bool category_two;

    union {
        // Present if static field,
        // static value 必须初始化清零
        union {
            jbool z;
            jbyte b;
            jchar c;
            jshort s;
            jint i;
            jlong j;
            jfloat f;
            jdouble d;
            jref r;
            slot_t data[2];
        } static_value{};

        // Present if instance field
        int id;
    };

    Field(Class *c, BytecodeReader &r);
    
    Field(Class *, const utf8_t *name, const utf8_t *descriptor, int access_flags);

    bool is_transient() const { return accIsTransient(access_flags); }
    bool is_volatile() const  { return accIsVolatile(access_flags); }
    
    bool is_prim_field() const;

    // the declared type(class Object) of this field
    // like, int k; the type of k is int.class
    jclsRef get_type();

    std::string toString() const;
};

#endif
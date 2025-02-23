#ifndef CABIN_REFLECT_H
#define CABIN_REFLECT_H

#include "../cabin.h"

/*
 * java.lang.reflect.Field
 * java.lang.reflect.Method
 * java.lang.reflect.Constructor
 */

void init_reflection();

jarrRef get_inner_classes_as_class_array(Class *c, bool public_only);

Class *get_declaring_class(Class *c);

// fo: object of "java.lang.reflect.Field"
jint field_offset(jref fo);

jint field_offset(Field *f);

// fo: object of "java.lang.reflect.Field"
Field *get_field_from_reflect_object(jref fo);

// mo: object of "java.lang.reflect.Method" or "java.lang.reflect.Constructor"
Method *get_method_from_reflect_object(jref mo);

jarrRef get_annotation_as_byte_array(Annotation &a);

// mo: object of java.lang.reflect.Method
// _this: If method is static, _this is NULL.
jref invoke_method(jref mo, jref _this, jarrRef args);

// co: object of java.lang.reflect.Constructor
jref new_instance_from_constructor(jref co, jarrRef args);

#endif
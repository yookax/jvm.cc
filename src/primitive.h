#ifndef CABIN_PRIMITIVE_H
#define CABIN_PRIMITIVE_H

#include "cabin.h"
#include "slot.h"

bool is_prim_class_name(const utf8_t *class_name);
bool is_prim_descriptor(utf8_t descriptor);
bool is_prim_wrapper_class_name(const utf8_t *class_name);
const utf8_t *get_prim_array_class_name(const utf8_t *class_name);
const utf8_t *get_prim_class_name(utf8_t descriptor);
const utf8_t *get_prim_descriptor_by_class_name(const utf8_t *class_name);
const utf8_t *get_prim_descriptor_by_wrapper_class_name(const utf8_t *wrapper_class_name);

#endif

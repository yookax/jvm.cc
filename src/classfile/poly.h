#ifndef CABIN_POLY_H
#define CABIN_POLY_H

#include <tuple>
#include "../cabin.h"

class Class;

void init_polymorphic_method();

/*
 * Lookup polymorphic method named `name` from class `c` and it's super class.
 * tuple<0>: the class which the polymorphic method declared in. If not find, this will be nullptr.
 * tuple<1>: access flag
 * tuple<2>: native method
 */
std::tuple<Class *, int, void *> lookup_polymorphic_method(Class *c, const utf8_t *name);

#endif
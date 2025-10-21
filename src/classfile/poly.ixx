module;
#include "../vmdef.h"

export module poly;

import std;

export void init_polymorphic_method();

/*
 * Lookup polymorphic method named `name` from class `c` and it's super class.
 * tuple<0>: the class which the polymorphic method declared in. If not find, this will be nullptr.
 * tuple<1>: access flag
 * tuple<2>: native method
 */
export std::pair<Class *, int> lookup_polymorphic_method(Class *c, const utf8_t *name);

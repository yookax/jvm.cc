module;
#include "../vmdef.h"

export module invoke;

import slot;

export void init_invoke();

export jref findMethodType(const utf8_t *desc, jref loader);
export jref findMethodType(jarrRef ptypes, jclsRef rtype);

export jref linkMethodHandleConstant(Class *caller_class, int ref_kind,
                              Class *defining_class, const char *name, jref type);


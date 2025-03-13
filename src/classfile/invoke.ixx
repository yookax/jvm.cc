module;
#include "../vmdef.h"

export module invoke;

import vmstd;

export void init_invoke();

export jref findMethodType(const utf8_t *desc, jref loader);
export jref findMethodType(jarrRef ptypes, jclsRef rtype);

export jref linkMethodHandleConstant(Class *caller_class, int ref_kind,
                              Class *defining_class, const char *name, jref type);

export namespace java_lang_invoke_MethodHandle {
    slot_t *invokeExact(const slot_t *args, u2 len);
    slot_t *invoke(const slot_t *args, u2 len);
    slot_t *invokeBasic(const slot_t *args, u2 len);

    slot_t *linkToVirtual(const slot_t *args, u2 len);
    slot_t *linkToStatic(const slot_t *args, u2 len);
    slot_t *linkToSpecial(const slot_t *args, u2 len);
    slot_t *linkToInterface(const slot_t *args, u2 len);
    slot_t *linkToNative(const slot_t *args, u2 len);
}

#ifndef CABIN_INVOKE_H
#define CABIN_INVOKE_H

#include "../cabin.h"
#include "../slot.h"

void init_invoke();

jref findMethodType(const utf8_t *desc, jref loader);
jref findMethodType(jarrRef ptypes, jclsRef rtype);

jref linkMethodHandleConstant(Class *caller_class, int ref_kind,
                              Class *defining_class, const char *name, jref type);

namespace java_lang_invoke_MethodHandle {
    slot_t *invokeExact(const slot_t *args, u2 len);
    slot_t *invoke(const slot_t *args, u2 len);
    slot_t *invokeBasic(const slot_t *args, u2 len);

    slot_t *linkToVirtual(const slot_t *args, u2 len);
    slot_t *linkToStatic(const slot_t *args, u2 len);
    slot_t *linkToSpecial(const slot_t *args, u2 len);
    slot_t *linkToInterface(const slot_t *args, u2 len);
    slot_t *linkToNative(const slot_t *args, u2 len);
}


#endif // CABIN_INVOKE_H
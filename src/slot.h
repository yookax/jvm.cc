#ifndef JVMCC_SLOT_H
#define JVMCC_SLOT_H

#include <cassert>
#include <cstdint>
#include "cabin.h"

// 一个slot_t类型必须可以容纳 jbool, jbyte, jchar, jshort，jint，jfloat, jref 称为类型一
// jlong, jdouble 称为类型二，占两个slot
typedef intptr_t slot_t;

static_assert(sizeof(slot_t) >= sizeof(jbool), ""); // todo msg
static_assert(sizeof(slot_t) >= sizeof(jbyte), "");
static_assert(sizeof(slot_t) >= sizeof(jchar), "");
static_assert(sizeof(slot_t) >= sizeof(jshort), "");
static_assert(sizeof(slot_t) >= sizeof(jint), "");
static_assert(sizeof(slot_t) >= sizeof(jfloat), "");
static_assert(sizeof(slot_t) >= sizeof(jref), "");
static_assert(2*sizeof(slot_t) >= sizeof(jlong), "");
static_assert(2*sizeof(slot_t) >= sizeof(jdouble), "");

namespace slot {

    /* setter */
    template<JavaValueType  T>
    void set(slot_t *slots, T v) {
        assert(slots != nullptr);
        * (T *) slots = v;
    }

    template <> void set<jbyte>(slot_t *, jbyte);
    template <> void set<jchar>(slot_t *, jchar);
    template <> void set<jshort>(slot_t *, jshort);

    /* getter */
    template<JavaValueType  T>
    T get(const slot_t *slots) {
        assert(slots != nullptr);
        return (* (const T *) (slots));
    }

    template <> jbyte get<jbyte>(const slot_t *);
    template <> jchar get<jchar>(const slot_t *);
    template <> jshort get<jshort>(const slot_t *);

    /* builder */
    slot_t islot(jint v);
    slot_t fslot(jfloat v);
    slot_t rslot(jref v);
}

#endif // JVMCC_SLOT_H
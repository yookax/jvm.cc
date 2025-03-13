module;
#include <cassert>
#include "vmdef.h"

export module slot;

import vmstd;

// 一个slot_t类型必须可以容纳 jbool, jbyte, jchar, jshort，jint，jfloat, jref 称为类型一
// jlong, jdouble 称为类型二，占两个slot
export using slot_t = intptr_t;

static_assert(sizeof(slot_t) >= sizeof(jbool), ""); // todo msg
static_assert(sizeof(slot_t) >= sizeof(jbyte), "");
static_assert(sizeof(slot_t) >= sizeof(jchar), "");
static_assert(sizeof(slot_t) >= sizeof(jshort), "");
static_assert(sizeof(slot_t) >= sizeof(jint), "");
static_assert(sizeof(slot_t) >= sizeof(jfloat), "");
static_assert(sizeof(slot_t) >= sizeof(jref), "");
static_assert(2*sizeof(slot_t) >= sizeof(jlong), "");
static_assert(2*sizeof(slot_t) >= sizeof(jdouble), "");

export namespace slot {
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

module : private;

template <> void slot::set<jbyte>(slot_t *slots, jbyte v) {
    assert(slots != nullptr);
    set<jint>(slots, v);
}

template <> void slot::set<jchar>(slot_t *slots, jchar v) {
    assert(slots != nullptr);
    set<jint>(slots, v);
}

template <> void slot::set<jshort>(slot_t *slots, jshort v) {
    assert(slots != nullptr);
    set<jint>(slots, v);
}

template <> jbyte slot::get<jbyte>(const slot_t *slots) {
    assert(slots != nullptr);
    return JINT_TO_JBYTE(get<jint>(slots));
}

template <> jchar slot::get<jchar>(const slot_t *slots) {
    assert(slots != nullptr);
    return JINT_TO_JCHAR(get<jint>(slots));
}

template <> jshort slot::get<jshort>(const slot_t *slots) {
    assert(slots != nullptr);
    return JINT_TO_JSHORT(get<jint>(slots));
}

slot_t slot::islot(jint v) {
    slot_t s;
    set<jint>(&s, v);
    return s;
}

slot_t slot::fslot(jfloat v) {
    slot_t s;
    set<jfloat>(&s, v);
    return s;
}

slot_t slot::rslot(jref v) {
    slot_t s;
    set<jref>(&s, v);
    return s;
}
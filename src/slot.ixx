module;
#include <cassert>
#include "vmdef.h"

export module slot;

import std.core;

// A type of slot_t must be able to hold jbool, jbyte, jchar, jshort,
// jint, and jfloat — these are referred to as Type 1;
// while jlong and jdouble are referred to as Type 2, which occupy two slots.
export using slot_t = intptr_t;

static_assert(sizeof(slot_t) >= sizeof(jbool), "A slot must be able to hold a bool.");
static_assert(sizeof(slot_t) >= sizeof(jbyte), "A slot must be able to hold a byte.");
static_assert(sizeof(slot_t) >= sizeof(jchar), "A slot must be able to hold a char.");
static_assert(sizeof(slot_t) >= sizeof(jshort), "A slot must be able to hold a short.");
static_assert(sizeof(slot_t) >= sizeof(jint), "A slot must be able to hold a int.");
static_assert(sizeof(slot_t) >= sizeof(jfloat), "A slot must be able to hold a float.");
static_assert(sizeof(slot_t) >= sizeof(jref), "A slot must be able to hold a reference.");
static_assert(2*sizeof(slot_t) >= sizeof(jlong), "2 slots must be able to hold a long.");
static_assert(2*sizeof(slot_t) >= sizeof(jdouble), "2 slots must be able to hold a double.");

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
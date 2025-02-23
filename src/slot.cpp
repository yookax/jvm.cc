#include <cassert>
#include "slot.h"

using namespace slot;

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
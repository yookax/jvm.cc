module;
#include "vmdef.h"

module reference;

import object;
import classfile;
import class_loader;

using namespace std;

static int referent_field_id; // private T referent;

void init_reference() {
    Class *c = load_boot_class("java/lang/ref/Reference");
    referent_field_id = c->get_field("referent")->id;
}

jref getAndClearReferencePendingList() {
    return nullptr; // todo
}

jboolean hasReferencePendingList() {
    unimplemented // todo
}

void waitForReferencePendingList() {
    return; // todo
}

jboolean referenceRefersTo(jref ref, jref obj) {
    jref referent = ref->get_field_value<jref>(referent_field_id);
    return referent == obj;
}

jboolean phantomReferenceRefersTo(jref phantom_ref, jref obj) {
    unimplemented
}

void referenceClear(jref ref) {
    // todo
}

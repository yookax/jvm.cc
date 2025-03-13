module;
#include "vmdef.h"

export module reference;

export void init_reference();

// Atomically get and clear (set to null) the VM's pending-Reference list.
export jref getAndClearReferencePendingList();

// Test whether the VM's pending-Reference list contains any entries.
export jboolean hasReferencePendingList();

// Wait until the VM's pending-Reference list may be non-null.
export void waitForReferencePendingList();

/**
 * Tests if the referent of this reference object is {@code obj}.
 * Using a {@code null} {@code obj} returns {@code true} if the
 * reference object has been cleared.
 *
 * @param  obj the object to compare with this reference object's referent
 * @return {@code true} if {@code obj} is the referent of this reference object
 * @since 16
 */
// ref: object of java.lang.Reference
// obj: the object to compare with "ref" object's referent
export jboolean referenceRefersTo(jref ref, jref obj);

/* Override the implementation of Reference.refersTo.
 * Phantom references are weaker than finalization, so the referent
 * access needs to be handled differently for garbage collectors that
 * do reference processing concurrently.
 */
// phantom_ref: object of java.lang.PhantomReference
// obj: the object to compare with "phantom_ref" object's referent
export jboolean phantomReferenceRefersTo(jref phantom_ref, jref obj);

export void referenceClear(jref ref);

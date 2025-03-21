module;
#include "../../../vmdef.h"

module native;

import runtime;
import heap;
import sysinfo;

// public native int availableProcessors();
void availableProcessors(Frame *f) {
    f->pushi(processor_number());
}

// public native long freeMemory();
void freeMemory(Frame *f) {
    f->pushl(g_heap->count_free_memory());
}

// public native long totalMemory();
void totalMemory(Frame *f) {
    f->pushl(g_heap->get_size());
}

// public native long maxMemory();
void maxMemory(Frame *f) {
    f->pushl(VM_HEAP_SIZE); // todo
}

// public native void gc();
void gc(Frame *f) {
    unimplemented
}

void java_lang_Runtime_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Runtime", #method, method_descriptor, method)

    R(availableProcessors, "()I");
    R(freeMemory, "()J");
    R(totalMemory, "()J");
    R(maxMemory, "()J");
    R(gc, "()V");
}


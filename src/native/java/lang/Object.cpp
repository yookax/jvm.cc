module;
#include "../../../vmdef.h"

module native;

import slot;
import classfile;
import object;
import class_loader;
import runtime;
import exception;

// public final native Class<?> getClass();
void getClass(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    f->pushr(_this->clazz->java_mirror);
}

// public native int hashCode();
void hashCode(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    f->pushi((jint)(intptr_t)_this); // todo
}

// protected native Object clone() throws CloneNotSupportedException;
void clone(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    if (!_this->clazz->is_subclass_of(load_boot_class("java/lang/Cloneable"))) {
        throw java_lang_CloneNotSupportedException(); // todo msg
    }
    f->pushr(_this->clone());
}

// public final native void notify();
void notify(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    _this->notify();
}

// public final native void notifyAll();
void notifyAll(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args);
    _this->notify_all();
}

// private final native void wait0(long timeoutMillis) throws InterruptedException;
void wait0(Frame *f) {
    slot_t *args = f->lvars;
    auto _this = slot::get<jref>(args++);
    auto timeout_millis = slot::get<jlong>(args);
    _this->wait(timeout_millis);
}

void java_lang_Object_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/Object", #method, method_descriptor, method)

    R(clone, "()Ljava/lang/Object;");
    R(getClass, "()Ljava/lang/Class;");
    R(hashCode, "()I");
    R(notify, "()V");
    R(notifyAll, "()V");
    R(wait0, "(J)V");
}
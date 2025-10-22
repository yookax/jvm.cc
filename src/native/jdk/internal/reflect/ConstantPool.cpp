module;
#include "../../../../vmdef.h"

module native;

import std;
import slot;
import runtime;
import constant_pool;

using namespace std;

// 参考 src\native\java\lang\Class.cpp getConstantPool函数

//private native in getSize0(Object constantPoolOop);
//static void getSize0(Frame *f) {
//    slot_t *args = f->lvars;
//    args++; // jump 'this'
//    auto cp = slot::get<jref>(args);
//
//    unimplemented
//}

//private native Class<?> getClassAt0         (Object constantPoolOop, int index);
//private native Class<?> getClassAtIfLoaded0 (Object constantPoolOop, int index);
//private native int      getClassRefIndexAt0 (Object constantPoolOop, int index);
//private native Member   getMethodAt0        (Object constantPoolOop, int index);
//private native Member   getMethodAtIfLoaded0(Object constantPoolOop, int index);
//private native Field    getFieldAt0         (Object constantPoolOop, int index);
//private native Field    getFieldAtIfLoaded0 (Object constantPoolOop, int index);
//private native String[] getMemberRefInfoAt0 (Object constantPoolOop, int index);
//private native int      getNameAndTypeRefIndexAt0(Object constantPoolOop, int index);
//private native String[] getNameAndTypeRefInfoAt0(Object constantPoolOop, int index);
//private native int      getIntAt0           (Object constantPoolOop, int index);
//private native long     getLongAt0          (Object constantPoolOop, int index);
//private native float    getFloatAt0         (Object constantPoolOop, int index);
//private native double   getDoubleAt0        (Object constantPoolOop, int index);
//private native String   getStringAt0        (Object constantPoolOop, int index);

//private native String getUTF8At0(Object constantPoolOop, int index);
static void getUTF8At0(Frame *f) {
    slot_t *args = f->lvars;
    args++; // jump 'this'
    auto cp = (ConstantPool *) slot::get<jref>(args++);
    auto index = slot::get<jint>(args);

    auto t = cp->types[index];

//cout << cp << endl;
//cout << index <<endl;
//cout << (int)t <<endl;

    unimplemented
}

//private native byte     getTagAt0           (Object constantPoolOop, int index);

void jdk_internal_reflect_ConstantPool_registerNatives() {
#undef R
#define R(method, method_descriptor) \
    registry("jdk/internal/reflect/ConstantPool", #method, method_descriptor, method)

    R(getUTF8At0, "(Ljava/lang/Object;I)Ljava/lang/String;");
}
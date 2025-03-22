module;
#include "../../../../vmdef.h"

module native;

import slot;
import object;
import classfile;
import constants;
import encoding;
import exception;
import class_loader;
import interpreter;

using namespace slot;
using namespace utf8;
using namespace java_lang_String;




void java_lang_invoke_MethodHandleNatives_registerNatives(Frame *f) {
#undef R
#define R(method, method_descriptor) \
    registry("java/lang/invoke/MethodHandleNatives", #method, method_descriptor, method)

}
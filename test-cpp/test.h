#ifndef JVMCC_TEST_H
#define JVMCC_TEST_H

#include "../src/vmdef.h"

import std.core;
import classfile;
import heap;
import object;
import invoke;
import jimage;
import encoding;
import sysinfo;
import primitive;
import class_loader;

#define TEST_CASE(func_name) \
    __declspec(dllexport) void func_name() { \
        printf("----------- %s -----------\n", #func_name);

#endif //JVMCC_TEST_H

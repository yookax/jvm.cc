#ifndef JVMCC_TEST_H
#define JVMCC_TEST_H

#include "../src/vmdef.h"

import std.core;
import classfile;
import heap;
import object;
import invoke;
import jimage_file;
import encoding;
import sysinfo;
import primitive;
import class_loader;

static struct {
    std::u8string s8;
    std::u16string s16;
} strings_for_testing[] = {
        { u8"Hello, World!", u"Hello, World!" },
        { u8"你好，世界！", u"你好，世界！" },
        { u8"こんにちは、世界！", u"こんにちは、世界！" },
        { u8"안녕하세요, 세상!", u"안녕하세요, 세상!" },
        { u8"Привет, мир!", u"Привет, мир!" },
        { u8"مرحبًا بالعالم!", u"مرحبًا بالعالم!" },
        { u8"Olá, mundo!", u"Olá, mundo!" },
        { u8"Hej, världen!", u"Hej, världen!" },
        { u8"Xin chào, thế giới!", u"Xin chào, thế giới!" },
        { u8"Hello, 你好😀", u"Hello, 你好😀" },
        { u8"👋世界！", u"👋世界！" },
};

#endif //JVMCC_TEST_H

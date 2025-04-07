module;
#include "../vmdef.h"

export module jimage;

import std.core;
import jimage_file;

using namespace std;

JImageFile *jf = nullptr;

export void init_jimage() {
    auto s = g_java_home + "/lib/modules";
    try {
        jf = new JImageFile(s.c_str());
    } catch (...) {
        jf = nullptr;
        panic("");
    }
}

export bool is_jimage_valid() {
    return jf != nullptr;
}

export optional<pair<const uint8_t *, size_t>> get_resource_from_jimage(const char8_t *path) {
    assert(jf != nullptr);
    assert(path != nullptr);
    return jf->get_resource(path);
}
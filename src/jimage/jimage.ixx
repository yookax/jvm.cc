module;
#include "../vmdef.h"

export module jimage;

import std.core;
import jimage_file;

using namespace std;

JImageFile *jf;

export void init_jimage() {
    auto s = g_java_home + "/lib/modules";
    jf = new JImageFile(s.c_str());
}

export optional<pair<const uint8_t *, size_t>> get_resource_from_jimage(const char8_t *path) {
    return jf->get_resource(path);
}

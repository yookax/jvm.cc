module;
#include <assert.h>
#include "../vmdef.h"

export module jimage;

import std.core;
import jimage_file;
import class_loader;
import classfile;
import object;

using namespace std;

//extern std::string g_java_home;

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

// export TEST_CASE(test_jimage_string)
//     static struct {
//         int hash_code;
//         const char8_t *str;
//     } pairs[] = {
//             { 16777619, u8"" },
//             { 1213053849, u8"foo" },
//             { 977475810, u8"bar" },
//             { -1678740824, u8"Hello, World!" },
//             { 1641313752, u8"你好，世界！" },
//             { -348596783, u8"123456789:一二三四五六七八九" },
//     };
//
//     for (auto &p: pairs) {
//         if (p.hash_code != unmasked_hash_code(p.str, HASH_MULTIPLIER)) {
//             cout << "failed. " << (const char *)p.str << " " << p.hash_code << endl;
//             cout << unmasked_hash_code(p.str, HASH_MULTIPLIER) << endl;
//         }
//     }
// }
//
// static auto _u8s = {// /<module>/<package>/<base>.<extension>
//         u8"/java.base/java/lang/Void.class",
//         u8"/java.base/java/lang/Class.class",
//         u8"/java.base/java/lang/Thread.class",
//         u8"/java.base/java/lang/classfile/constantpool/StringEntry.class",
//         u8"/jdk.net/jdk/net/Sockets.class",
//         u8"/java.logging/sun/net/www/protocol/http/logging/HttpLogFormatter.class",
// };
//
// export TEST_CASE(test_jimage)
//     // JImageFile jf("d:/modules");
//     // jf.print();
//     //
//     // for (auto u8: _u8s) {
//     //     cout << "---\n[" << (const char *)u8 << "]" << endl;
//     //     auto location = jf.find_location(u8);
//     //     if (!location.has_value()) {
//     //         cout << "not find" << endl;
//     //         continue;
//     //     }
//     //     cout << location.value().to_str(jf.index.strings);
//     // }
// }
//
// export TEST_CASE(test_jimage1)
//     for (auto u8: _u8s) {
//         cout << "---\n[" << (const char *)u8 << "]" << endl;
//         auto resource = get_resource_from_jimage(u8);
//         if (!resource.has_value()) {
//             cout << "not find" << endl;
//             continue;
//         }
//         auto [bytecode, size] = resource.value();
//         Class *c = define_class(BOOT_CLASS_LOADER, bytecode, size);
//         printf("%p, %s, %s, %d\n", c, c->name,
//                c->java_mirror->jvm_mirror->name,
//                c == c->java_mirror->jvm_mirror);
//     }
// }
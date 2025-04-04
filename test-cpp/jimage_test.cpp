#include "test.h"

using namespace std;

TEST_CASE(test_jimage_string)
    static struct {
        int hash_code;
        const char8_t *str;
    } pairs[] = {
        { 16777619, u8"" },
        { 1213053849, u8"foo" },
        { 977475810, u8"bar" },
        { -1678740824, u8"Hello, World!" },
        { 1641313752, u8"你好，世界！" },
        { -348596783, u8"123456789:一二三四五六七八九" },
    };

    for (auto &p: pairs) {
        if (p.hash_code != unmasked_hash_code(p.str, HASH_MULTIPLIER)) {
            cout << "failed. " << (const char *)p.str << " " << p.hash_code << endl;
            cout << unmasked_hash_code(p.str, HASH_MULTIPLIER) << endl;
        }
    }

//    require.Equal(t, int32(16777619), unmaskedHashCode("", HashMultiplier))
//    require.Equal(t, int32(1213053849), unmaskedHashCode("foo", HashMultiplier))
//    require.Equal(t, int32(977475810), unmaskedHashCode("bar", HashMultiplier))
//    require.Equal(t, int32(-1678740824), unmaskedHashCode("Hello, World!", HashMultiplier))
//    require.Equal(t, int32(1641313752), unmaskedHashCode("你好，世界！", HashMultiplier))
//    require.Equal(t, int32(-348596783), unmaskedHashCode("123456789:一二三四五六七八九", HashMultiplier))
}

static auto u8s = {// /<module>/<package>/<base>.<extension>
        u8"/java.base/java/lang/Void.class",
        u8"/java.base/java/lang/Class.class",
        u8"/java.base/java/lang/Thread.class",
        u8"/java.base/java/lang/classfile/constantpool/StringEntry.class",
        u8"/jdk.net/jdk/net/Sockets.class",
        u8"/java.logging/sun/net/www/protocol/http/logging/HttpLogFormatter.class",
};

TEST_CASE(test_jimage)
    JImageFile jf("d:/modules");
    jf.read();
    jf.print();

    for (auto u8: u8s) {
        cout << "---\n[" << (const char *)u8 << "]" << endl;
        auto location = jf.find_location(u8);
        if (!location.has_value()) {
            cout << "not find" << endl;
            continue;
        }
        cout << location.value().to_str(jf.index.strings);
    }
}
module;
#include <cassert>
#include <cstdint>
#include "vmdef.h"

export module encoding;

import std.core;

using namespace std;

/*
 * jvm内部使用的utf8是一种改进过的utf8，与标准的utf8有所不同，
 * 具体参考 jvms。
 *
 * this vm 操作的utf8字符串，要求以'\0'结尾并且不包含utf8的结束符.
 */

//export namespace utf8_pool {
//    // save a utf8 string to pool.
//    // 如不存在，返回新插入的值
//    // 如果已存在，则返回池中的值。
//    const utf8_t *save(const utf8_t *utf8);
//
//    // get utf8 from pool, return null if not exist.
//    const utf8_t *find(const utf8_t *utf8);
//}

export struct MUTF8 {
    const uint8_t *s;
    uint16_t len_by_byte;

    bool operator==(const MUTF8& other) const {
        return (len_by_byte == other.len_by_byte) && (memcmp(s, other.s, len_by_byte) == 0);
    }

    bool operator<(const MUTF8& other) const {
        return memcmp(s, other.s, min(len_by_byte, other.len_by_byte)) < 0;
    }
};

export namespace utf8 {
    size_t hash(const utf8_t *utf8);

    size_t length(const utf8_t *utf8);

    bool equals(const utf8_t *p1, const utf8_t *p2);

    utf8_t *dup(const utf8_t *utf8);

    utf8_t *dot_2_slash(utf8_t *utf8);
    utf8_t *dot_2_slash_dup(const utf8_t *utf8);

    utf8_t *slash_2_dot(utf8_t *utf8);
    utf8_t *slash_2_dot_dup(const utf8_t *utf8);

    unicode_t *toUnicode(const utf8_t *utf8, size_t unicode_len);

    struct Hash {
        size_t operator()(const utf8_t *utf8) const {
            return hash(utf8);
        }
    };

    struct Comparator {
        bool operator()(const utf8_t *s1, const utf8_t *s2) const {
            assert(s1 != nullptr && s2 != nullptr);
            return equals(s1, s2);
        }
    };
}

export u8string unicode_to_utf8(const wstring& wstr);
export u8string unicode_to_utf8(const unicode_t *wstr, size_t len);

export namespace unicode {
    // 由调用者 delete[] utf8 string
    utf8_t *to_utf8(const unicode_t *unicode, size_t len);
}

export u8string *mutf8_to_utf8(const uint8_t *mutf8, size_t len, u8string *utf8);

export u8string mutf8_to_utf8(const uint8_t *mutf8, size_t len) {
    u8string utf8;
    mutf8_to_utf8(mutf8, len, &utf8);
    return utf8;
}

export u8string *mutf8_to_new_utf8(const uint8_t *mutf8, size_t len) {
    auto s = new u8string;
    return mutf8_to_utf8(mutf8, len, s);
}

export u16string utf8_to_utf16(const u8string& utf8_str);
export optional<string> utf8_to_latin1(const u8string& utf8_str);

// ---------------------------------------------------------------------------------------

export void test_utf8_to_latin1();
export void test_utf8_to_utf16();
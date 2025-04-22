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

export u8string mutf8_to_utf8(const MUTF8& mutf8) {
    return mutf8_to_utf8(mutf8.s, mutf8.len_by_byte);
}

export u8string *mutf8_to_new_utf8(const uint8_t *mutf8, size_t len) {
    auto s = new u8string;
    return mutf8_to_utf8(mutf8, len, s);
}

export u8string utf16_to_utf8(const u16string& utf16_str) {
    u8string utf8_result;
    for (size_t i = 0; i < utf16_str.size(); ++i) {
        char16_t c = utf16_str[i];
        if (c <= 0x7F) {
            // 单字节字符
            utf8_result.push_back(static_cast<char8_t>(c));
        } else if (c <= 0x7FF) {
            // 双字节字符
            utf8_result.push_back(static_cast<char8_t>(0xC0 | (c >> 6)));
            utf8_result.push_back(static_cast<char8_t>(0x80 | (c & 0x3F)));
        } else {
            if (c >= 0xD800 && c <= 0xDBFF && i + 1 < utf16_str.size() && utf16_str[i + 1] >= 0xDC00 && utf16_str[i + 1] <= 0xDFFF) {
                // 处理代理对
                uint32_t code_point = 0x10000 + ((c - 0xD800) << 10) + (utf16_str[i + 1] - 0xDC00);
                utf8_result.push_back(static_cast<char8_t>(0xF0 | (code_point >> 18)));
                utf8_result.push_back(static_cast<char8_t>(0x80 | ((code_point >> 12) & 0x3F)));
                utf8_result.push_back(static_cast<char8_t>(0x80 | ((code_point >> 6) & 0x3F)));
                utf8_result.push_back(static_cast<char8_t>(0x80 | (code_point & 0x3F)));
                ++i;
            } else {
                // 三字节字符
                utf8_result.push_back(static_cast<char8_t>(0xE0 | (c >> 12)));
                utf8_result.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & 0x3F)));
                utf8_result.push_back(static_cast<char8_t>(0x80 | (c & 0x3F)));
            }
        }
    }
    return utf8_result;
}

export u16string utf8_to_utf16(const u8string& utf8_str);
export optional<string> utf8_to_latin1(const u8string& utf8_str);

// ---------------------------------------------------------------------------------------

struct {
    std::u8string s8;
    std::u16string s16;
} arr[] = {
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

export TEST_CASE(test_utf8_to_latin1)
//    for (auto &a: arr) {
//        auto x = utf8_to_latin1(a.s8);
//        if (x.has_value()) {
//            std::cout << (char *) a.s8.c_str() << " <---> "<< x.value() << std::endl;
//        }
//    }
}

export TEST_CASE(test_utf8_to_utf16)
    bool failed = false;
    for (auto &a: arr) {
        if (utf8_to_utf16(a.s8) != a.s16) {
            failed = true;
            std::cerr << "failed. " << (const char *) a.s8.c_str() << std::endl;
        }
    }

    if(!failed)
        cout << "passed." << endl;
}

export TEST_CASE(test_utf16_to_utf8)
    bool failed = false;
    for (auto &a: arr) {
        if (utf16_to_utf8(a.s16) != a.s8) {
            failed = true;
            std::cerr << "failed. " << (const char *) a.s8.c_str() << std::endl;
        }
    }

    if(!failed)
        cout << "passed." << endl;
}
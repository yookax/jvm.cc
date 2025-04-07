module;
#include <cassert>
//#include <pthread.h>
#include "vmdef.h"

module encoding;

import std.core;

using namespace std;

static unordered_set<const utf8_t *, utf8::Hash, utf8::Comparator> utf8Set;
//static pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

const utf8_t *utf8_pool::save(const utf8_t *utf8) {
    assert(utf8 != nullptr);

    //pthread_rwlock_wrlock(&lock);
    const utf8_t *s = *utf8Set.insert(utf8).first;
    //pthread_rwlock_unlock(&lock);
    return s;
}

const utf8_t *utf8_pool::find(const utf8_t *utf8) {
    assert(utf8 != nullptr);

    //pthread_rwlock_rdlock(&lock);
    auto iter = utf8Set.find(utf8);
    const utf8_t *s = iter == utf8Set.end() ? nullptr : *iter;
    //pthread_rwlock_unlock(&lock);
    return s;
}

static inline unicode_t get_utf8_char(const utf8_t *&utf8) {
    assert(utf8 != nullptr);

    unicode_t x = *utf8++;
    if (x & 0x80) {
        unicode_t y = *utf8++;
        if (x & 0x20) {
            unicode_t z = *utf8++;
            return (unicode_t) (((x & 0xf) << 12) + ((y & 0x3f) << 6) + (z & 0x3f));
        } else {
            return (unicode_t) (((x & 0x1f) << 6) + (y & 0x3f));
        }
    }

    return x;
}

size_t utf8::hash(const utf8_t *utf8) {
    if (utf8 == nullptr)
        return 0;

    size_t hash = 0;
    while (*utf8) {
        hash = hash * 37 + get_utf8_char(utf8);
    }

    return hash;
}

size_t utf8::length(const utf8_t *utf8) {
    assert(utf8 != nullptr);

    size_t count;
    for(count = 0; *utf8; count++) {
        int x = *utf8;
        utf8 += (x & 0x80) ? ((x & 0x20) ? 3 : 2) : 1;
    }

    return count;
}

// todo 为什么这个函数比下面注释掉的equals更快？？？？？？？？
bool utf8::equals(const utf8_t *p1, const utf8_t *p2) {
    assert(p1 != nullptr && p2 != nullptr);

    if (p1 == p2)
        return true;

    while(*p1 && *p2) {
        if(get_utf8_char(p1) != get_utf8_char(p2))
            return false;
    }

    return !(*p1 || *p2);
}

// bool utf8::equals(const utf8_t *p1, const utf8_t *p2)
// {
//     assert(p1 != nullptr && p2 != nullptr);
//     return (p1 == p2) || (strcmp(p1, p2) == 0);
// }

utf8_t *utf8::dup(const utf8_t *utf8) {
    assert(utf8 != nullptr);
    // 因为jvm改进的utf8中除结束符外不包含'\0'，所以用strcpy即可。
    // char *p = (char *) malloc((strlen(utf8) + 1)*sizeof(char));
    // strcpy(p, utf8);
    // return p;
    return strdup(utf8);
}

utf8_t *utf8::dot_2_slash(utf8_t *utf8) {
    assert(utf8 != nullptr);

    for(utf8_t *tmp = utf8; *tmp; tmp++) {
        if (*tmp == '.')
            *tmp = '/';
    }

    return utf8;
}

utf8_t *utf8::dot_2_slash_dup(const utf8_t *utf8) {
    assert(utf8 != nullptr);
    return dot_2_slash(dup(utf8));
}

utf8_t *utf8::slash_2_dot(utf8_t *utf8) {
    assert(utf8 != nullptr);

    for(utf8_t *tmp = utf8; *tmp; tmp++) {
        if (*tmp == '/')
            *tmp = '.';
    }

    return utf8;
}

utf8_t *utf8::slash_2_dot_dup(const utf8_t *utf8) {
    assert(utf8 != nullptr);
    return slash_2_dot(dup(utf8));
}

unicode_t *utf8::toUnicode(const utf8_t *utf8, size_t unicode_len) {
    assert(utf8 != nullptr);

    auto buf = new unicode_t[unicode_len + 1];
    buf[unicode_len] = 0;

    for (int i = 0; i < unicode_len; i++) {
        if (*utf8) {
            buf[i] = get_utf8_char(utf8);
        }
    }

//    auto tmp = buf;
//    while (*utf8) {
//        *tmp++ = get_utf8_char(utf8);
//    }
//    // 不应该写出buf的范围
//    assert(buf[unicode_len] == 0);
    return buf;
}

// 将此unicode转化为utf8时，有多少字节
static size_t utf8_bytes_count(const unicode_t *unicode, size_t len) {
    assert(unicode != nullptr);
    size_t count = 0;

    for(; len > 0; len--) {
        auto c = *unicode++;
        count += (c == 0 || c > 0x7f) ? (c > 0x7ff ? 3 : 2) : 1;
    }

    return count;
}

// Unicode 转 UTF-8
u8string unicode_to_utf8(const unicode_t *wstr, size_t len) {
    u8string utf8_str;
    for (size_t i = 0; i < len; i++) {
        unicode_t wc = wstr[i];
        if (wc <= 0x7F) {
            // 单字节字符
            utf8_str.push_back(static_cast<char8_t>(wc));
        } else if (wc <= 0x7FF) {
            // 双字节字符
            utf8_str.push_back(static_cast<char8_t>(0xC0 | ((wc >> 6) & 0x1F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | (wc & 0x3F)));
        } else if (wc <= 0xFFFF) {
            // 三字节字符
            utf8_str.push_back(static_cast<char8_t>(0xE0 | ((wc >> 12) & 0x0F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | ((wc >> 6) & 0x3F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | (wc & 0x3F)));
        }
    }
    return utf8_str;
}

// Unicode 转 UTF-8
u8string unicode_to_utf8(const wstring& wstr) {
    u8string utf8_str;
    for (wchar_t wc : wstr) {
        if (wc <= 0x7F) {
            // 单字节字符
            utf8_str.push_back(static_cast<char8_t>(wc));
        } else if (wc <= 0x7FF) {
            // 双字节字符
            utf8_str.push_back(static_cast<char8_t>(0xC0 | ((wc >> 6) & 0x1F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | (wc & 0x3F)));
        } else if (wc <= 0xFFFF) {
            // 三字节字符
            utf8_str.push_back(static_cast<char8_t>(0xE0 | ((wc >> 12) & 0x0F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | ((wc >> 6) & 0x3F)));
            utf8_str.push_back(static_cast<char8_t>(0x80 | (wc & 0x3F)));
        }
    }
    return utf8_str;
}

utf8_t *unicode::to_utf8(const unicode_t *unicode, size_t len) {
    assert(unicode != nullptr);

    auto utf8 = new utf8_t[utf8_bytes_count(unicode, len) + 1];
    auto p = utf8;

    for(; len > 0; len--) {
        auto c = *unicode++;
        if((c == 0) || (c > 0x7f)) {
            if(c > 0x7ff) {
                *p++ = (utf8_t) ((c >> 12) | 0xe0);
                *p++ = (utf8_t) (((c >> 6) & 0x3f) | 0x80);
            } else {
                *p++ = (utf8_t) ((c >> 6) | 0xc0);
            }
            *p++ = (utf8_t) ((c & 0x3f) | 0x80);
        } else {
            *p++ = (utf8_t) (c);
        }
    }

    *p = 0;
    return utf8;
}

u8string *mutf8_to_utf8(const uint8_t *mutf8, size_t len, u8string *utf8) {
    for (size_t i = 0; i < len; ) {
        uint8_t byte = mutf8[i];
        if (byte == 0xC0 && i + 1 < len && mutf8[i + 1] == 0x80) {
            // 处理 Java 改良版 UTF-8 中对 \u0000 的特殊编码
            utf8->push_back(u8'\0');
            i += 2;
        } else if ((byte & 0x80) == 0) {
            // 单字节字符
//            utf8 += static_cast<char>(byte);
            utf8->push_back(byte);
            ++i;
        } else if ((byte & 0xE0) == 0xC0) {
            // 双字节字符
            if (i + 1 < len) {
                uint8_t nextByte = mutf8[++i];
//                utf8 += static_cast<char>(byte);
//                utf8 += static_cast<char>(nextByte);
                utf8->push_back(byte);
                utf8->push_back(nextByte);
            }
            ++i;
        } else if ((byte & 0xF0) == 0xE0) {
            // 三字节字符
            if (i + 2 < len) {
                uint8_t nextByte1 = mutf8[++i];
                uint8_t nextByte2 = mutf8[++i];
//                utf8 += static_cast<char>(byte);
//                utf8 += static_cast<char>(nextByte1);
//                utf8 += static_cast<char>(nextByte2);
                utf8->push_back(byte);
                utf8->push_back(nextByte1);
                utf8->push_back(nextByte2);
            }
            ++i;
        } else if ((byte & 0xF8) == 0xF0) {
            // 处理 Java 虚拟机使用的两个三字节格式
            if (i + 5 < len) {
                uint32_t codePoint = ((mutf8[i] & 0x07) << 18) |
                                     ((mutf8[i + 1] & 0x3F) << 12) |
                                     ((mutf8[i + 2] & 0x3F) << 6) |
                                     ((mutf8[i + 3] & 0x07) << 18) |
                                     ((mutf8[i + 4] & 0x3F) << 12) |
                                     (mutf8[i + 5] & 0x3F);
                if (codePoint >= 0x10000 && codePoint <= 0x10FFFF) {
                    // 转换为标准 UTF-8 的四字节编码
//                    utf8 += static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07));
//                    utf8 += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
//                    utf8 += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
//                    utf8 += static_cast<char>(0x80 | (codePoint & 0x3F));
                    utf8->push_back(0xF0 | ((codePoint >> 18) & 0x07));
                    utf8->push_back(0x80 | ((codePoint >> 12) & 0x3F));
                    utf8->push_back(0x80 | ((codePoint >> 6) & 0x3F));
                    utf8->push_back(0x80 | (codePoint & 0x3F));
                }
            }
            i += 6;
        }
    }
    return utf8;
}

// 将 UTF - 8 字符串转换为 UTF - 16 字符串
std::u16string utf8_to_utf16(const std::u8string& utf8_str) {
    std::u16string utf16_str;
    const char8_t* ptr = utf8_str.data();
    const char8_t* end = ptr + utf8_str.size();

    while (ptr < end) {
        char32_t code_point;
        size_t bytes_read = 0;

        // 处理 UTF - 8 编码
        if ((*ptr & 0x80) == 0) {
            code_point = static_cast<char32_t>(*ptr);
            bytes_read = 1;
        } else if ((*ptr & 0xE0) == 0xC0) {
            if (ptr + 1 >= end) {
                throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence));
            }
            code_point = ((static_cast<char32_t>(*ptr & 0x1F) << 6) |
                          (static_cast<char32_t>(*(ptr + 1) & 0x3F)));
            bytes_read = 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            if (ptr + 2 >= end) {
                throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence));
            }
            code_point = ((static_cast<char32_t>(*ptr & 0x0F) << 12) |
                          (static_cast<char32_t>(*(ptr + 1) & 0x3F) << 6) |
                          (static_cast<char32_t>(*(ptr + 2) & 0x3F)));
            bytes_read = 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            if (ptr + 3 >= end) {
                throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence));
            }
            code_point = ((static_cast<char32_t>(*ptr & 0x07) << 18) |
                          (static_cast<char32_t>(*(ptr + 1) & 0x3F) << 12) |
                          (static_cast<char32_t>(*(ptr + 2) & 0x3F) << 6) |
                          (static_cast<char32_t>(*(ptr + 3) & 0x3F)));
            bytes_read = 4;
        } else {
            throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence));
        }

        // 将码点转换为 UTF - 16
        if (code_point <= 0xFFFF) {
            utf16_str += static_cast<char16_t>(code_point);
        } else {
            // 处理代理对
            code_point -= 0x10000;
            utf16_str += static_cast<char16_t>((code_point >> 10) + 0xD800);
            utf16_str += static_cast<char16_t>((code_point & 0x3FF) + 0xDC00);
        }

        ptr += bytes_read;
    }

    return utf16_str;
}

// 将 UTF - 8 字符串转换为 LATIN1 字符串
optional<string> utf8_to_latin1(const u8string& utf8_str) {
    std::string latin1_str;
    const char8_t* ptr = utf8_str.data();
    const char8_t* end = ptr + utf8_str.size();

    while (ptr < end) {
        auto first_byte = *ptr;
        if ((first_byte & 0x80) == 0) {
            // 单字节 UTF - 8 字符（ASCII 范围）
            latin1_str.push_back(static_cast<char>(first_byte));
            ptr++;
        } else if ((first_byte & 0xE0) == 0xC0 && ptr + 1 < end) {
            // 双字节字符 (110xxxxx 10xxxxxx)
            auto second_byte = *(ptr + 1);
            if ((second_byte & 0xC0) != 0x80) {
                // 无效的 UTF - 8 双字节序列
                return nullopt;
            }
            char32_t code_point = ((first_byte & 0x1F) << 6) | (second_byte & 0x3F);
            if (code_point <= 0xFF) {
                latin1_str.push_back(static_cast<char>(code_point));
            } else {
                // Character out of LATIN1 range.
                return nullopt;
            }
            ptr += 2;
        } else {
            // Unsupported UTF - 8 sequence or out of LATIN1 range.
            return nullopt;
        }
    }
    return latin1_str;
}

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

void test_utf8_to_latin1() {
//    for (auto &a: arr) {
//        auto x = utf8_to_latin1(a.s8);
//        if (x.has_value()) {
//            std::cout << (char *) a.s8.c_str() << " <---> "<< x.value() << std::endl;
//        }
//    }
}

void test_utf8_to_utf16() {
    bool failed = false;
    for (auto &a: arr) {
        if (utf8_to_utf16(a.s8) != a.s16) {
            failed = true;
            std::cerr << "failed. " << (const char *) a.s8.c_str() << std::endl;
        }
    }

    if(!failed)
        cout << "passed" << endl;
}
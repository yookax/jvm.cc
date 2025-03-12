#include <cassert>
#include <cstring>
//#include <pthread.h>
#include "cabin.h"
#include "encoding.h"

module vmstd;

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

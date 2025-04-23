module;
#include <cassert>
#include "../vmdef.h"

module object;

import std.core;
import slot;
import interpreter;

using namespace std;
using namespace slot;
using namespace utf8;

// set java/lang/String 的 coder 变量赋值
// private final byte coder;
// 可取一下两值之一：
// @Native static final byte LATIN1 = 0;
// @Native static final byte UTF16  = 1;
#define STRING_CODE_LATIN1 0
#define STRING_CODE_UTF16 1

jstrRef Allocator::string(const u8string& utf8) {
    std::string latin1;
    u16string u16;
    const void *data;
    size_t len_by_byte;
    jbyte coder;

    optional<std::string> opt = utf8_to_latin1(utf8);
    if (opt.has_value()) {
        latin1 = opt.value();
        data = latin1.c_str();
        len_by_byte = latin1.length();
        coder = STRING_CODE_LATIN1;
    } else {
        u16 = utf8_to_utf16(utf8);
        data = u16.c_str();
        len_by_byte = u16.length() * sizeof(char16_t);
        coder = STRING_CODE_UTF16;
    }

    init_class(g_string_class);
    jstrRef so = Allocator::object(g_string_class);

    // set java/lang/String 的 value 变量赋值
    // private final byte[] value;
    jarrRef value = Allocator::array("[B", len_by_byte); // [B
    memcpy(value->data, data, len_by_byte);
    so->set_field_value<jref>("value", "[B", value);

    so->set_field_value<jbyte>("coder", coder);
    return so;
}

jstrRef Allocator::string(const MUTF8& mutf8) {
    u8string utf8 = mutf8_to_utf8(mutf8);
    return Allocator::string(utf8);
}

jstrRef Allocator::string(const utf8_t *str) {
    assert(g_string_class != nullptr && str != nullptr);

    init_class(g_string_class);
    //assert(g_string_class->lookup_field("COMPACT_STRINGS", "Z")->static_value.z);

    size_t len = length(str);

    MUTF8 mutf8;
    mutf8.s = (const uint8_t *)str;
    mutf8.len_by_byte = len;

    return Allocator::string(mutf8);
}

utf8_t *java_lang_String::to_utf8(jstrRef so) {
    assert(so != nullptr);
    assert(g_string_class != nullptr);
    assert(so->is_string_object());

    u8string u8 = jstring_to_u8string(so);
    auto buf = new char8_t[u8.length() + 1];
    buf[u8.length()] = 0;
    memcpy(buf, u8.c_str(), u8.length() * sizeof(char8_t));
    return (char *)buf;
}

u8string jstring_to_u8string(jstrRef so) {
    assert(so != nullptr);
    assert(g_string_class != nullptr);
    assert(so->is_string_object());

    // byte[] value;
    auto value = so->get_field_value<jref>("value", "[B");
    auto coder = so->get_field_value<jbyte>("coder");
    if (coder == STRING_CODE_LATIN1) {
        u8string s((char8_t *)value->data, value->arr_len);
        return s;
    } else if (coder == STRING_CODE_UTF16) {
        u16string s((char16_t *)value->data, value->arr_len / sizeof(char16_t));
        return utf16_to_utf8(s);
    }

    UNREACHABLE("%d", coder);
}

bool java_lang_String::equals(jstrRef x, jstrRef y) {
    assert(x != nullptr && y != nullptr);
    assert(x->is_string_object() && y->is_string_object());

    // public boolean equals(Object anObject);
    Method *equals = g_string_class->get_method("equals", "(Ljava/lang/Object;)Z");
    return slot::get<jbool>(execJava(equals, { rslot(x), rslot(y) }));
}

size_t java_lang_String::hash(jstrRef x) {
    assert(x != nullptr && x->is_string_object());

    // public int hashCode();
    Method *hash_code = g_string_class->get_method("hashCode", "()I");
    return (size_t) slot::get<jint>(execJava(hash_code, { rslot(x) }));
}

/*--------------------------- String Pool ---------------------------*/

struct StringEquals {
    bool operator()(jstrRef x, jstrRef y) const     {
        return java_lang_String::equals(x, y);
    }
};

struct StringHash {
    size_t operator()(jstrRef x) const     {
        return java_lang_String::hash(x);
    }
};

// A pool of Java Strings, initially empty, is maintained privately by the String class.
static unordered_set<Object *, StringHash, StringEquals> str_pool;

static /*mutable*/ recursive_mutex str_pool_mutex;

jstrRef java_lang_String::intern(jstrRef so) {
    lock_guard<recursive_mutex> lock(str_pool_mutex);

    assert(so != nullptr);
    assert(so->is_string_object());

    // return either the newly inserted element
    // or the equivalent element already in the set
    Object *interned = *(str_pool.insert(so).first);
    return interned;
}

// ---------------------------------------------------------------------------------------

TEST_CASE(test_string)
    for (auto& s: strings_for_testing) {
        auto so = Allocator::string(s.s8);
        auto u = jstring_to_u8string(so);
        if (s.s8 != u) {
            printf("%s\n%s\nfailed\n", (char *) s.s8.c_str(), (char *) u.c_str());
        }
    }
}

TEST_CASE(test_string_intern)
    for (auto& s: strings_for_testing) {
        auto so1 = Allocator::string(s.s8);
        auto so2 = Allocator::string(s.s8);

        auto intern1 = java_lang_String::intern(so1);
        auto intern2 = java_lang_String::intern(so2);

        if (intern1 != intern2)
            printf("failed\n");
    }
}

TEST_CASE(test_string_equals)
    for (auto& s: strings_for_testing) {
        auto so1 = Allocator::string(s.s8);
        auto so2 = Allocator::string(s.s8);

        if (!java_lang_String::equals(so1, so2))
            printf("failed\n");
    }
}
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
using namespace unicode;

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

    jstrRef so = Allocator::object(g_string_class);
    size_t len = length(str);

    // set java/lang/String 的 value 变量赋值
    // private final byte[] value;
    jarrRef value = Allocator::array("[B", len); // [B
    memcpy(value->data, str, len);
    so->set_field_value<jref>("value", "[B", value);

    so->set_field_value<jbyte>("coder", STRING_CODE_LATIN1);
    return so;
}

jstrRef Allocator::string(const unicode_t *str, jsize len) {
    assert(str != nullptr && len >= 0);
    utf8_t *utf8 = unicode::to_utf8(str, len);
    jstrRef so = Allocator::string(utf8);
    //delete[] utf8; todo
    return so;
}

utf8_t *java_lang_String::to_utf8(jstrRef so) {
    assert(so != nullptr);
    assert(g_string_class != nullptr);
    assert(so->is_string_object());

    // byte[] value;
    jarrRef value = so->get_field_value<jref>("value", "[B");
    
    jbyte code = so->get_field_value<jbyte>("coder");
    if (code == STRING_CODE_LATIN1) {
        auto utf8 = new utf8_t[value->arr_len + 1];//(utf8_t *)vm_malloc(sizeof(utf8_t) * (value->arr_len + 1));
        utf8[value->arr_len] = 0;
        memcpy(utf8, value->data, value->arr_len * sizeof(jbyte));
        return utf8;
    }
    if (code == STRING_CODE_UTF16) {
        utf8_t *utf8 = unicode::to_utf8((unicode_t *)value->data, value->arr_len);
        return utf8;
    }

    UNREACHABLE("%d", code);
}

unicode_t *java_lang_String::to_unicode(jstrRef so) {
    assert(so != nullptr);
    assert(g_string_class != nullptr);
    assert(so->is_string_object());
    
    // byte[] value;
    jarrRef value = so->get_field_value<jref>("value", "[B");

    jbyte code = so->get_field_value<jbyte>("coder");
    if (code == STRING_CODE_LATIN1) {
        unicode_t *u = utf8::toUnicode((utf8_t *)value->data, value->arr_len);
        return u;
    }
    if (code == STRING_CODE_UTF16) {
        auto u = new unicode_t[value->arr_len + 1];//(unicode_t *)vm_malloc(sizeof(unicode_t) * (value->arr_len + 1));
        u[value->arr_len] = 0;
        memcpy(u, value->data, value->arr_len * sizeof(jbyte));
        return u;
    }

    UNREACHABLE("%d", code);
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

jsize java_lang_String::length(jstrRef so) {
    // todo
    
    // private final byte coder;
    // 可取一下两值之一：
    // @Native static final byte LATIN1 = 0;
    // @Native static final byte UTF16  = 1;
    jbyte code = so->get_field_value<jbyte>("coder");
    if (code == 1) {
        unimplemented
    } else {
        // private final byte[] value;
        jarrRef value = so->get_field_value<jref>("value", "[B");
        return value->arr_len;
    }
}

jsize java_lang_String::uft_length(jstrRef so) {
    // todo
    
    // private final byte coder;
    // 可取一下两值之一：
    // @Native static final byte LATIN1 = 0;
    // @Native static final byte UTF16  = 1;
    jbyte code = so->get_field_value<jbyte>("coder");
    if (code == 1) {
        unimplemented
    } else {
        // private final byte[] value;
        jarrRef value = so->get_field_value<jref>("value", "[B");
        return value->arr_len;
    }
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

static const utf8_t *utf8s[] = {
        "Hello, World!",
        "你好，世界！",
        "こんにちは、世界！",
};

TEST_CASE(test_string)
    for (auto s: utf8s) {
        auto so = Allocator::string(s);
        auto u = java_lang_String::to_utf8(so);
        if (!utf8::equals(s, u)) {
            printf("failed\n");
        }
    }
}

TEST_CASE(test_string_intern)
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        auto intern1 = java_lang_String::intern(so1);
        auto intern2 = java_lang_String::intern(so2);

        if (intern1 != intern2)
            printf("failed\n");
    }
}

TEST_CASE(test_string_equals)
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        if (!java_lang_String::equals(so1, so2))
            printf("failed\n");
    }
}

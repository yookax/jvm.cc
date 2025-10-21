// module;
// #include "../vmdef.h"

export module jimage_strings;

import std;

using namespace std;

// jdk/internal/jimage/ImageStringsReader.java

export const int HASH_MULTIPLIER = 0x01000193;
static const int POSITIVE_MASK = 0x7FFFFFFF;

export int unmasked_hash_code(u8string &s, int seed) {
    size_t len = s.length();
    int8_t *buffer = nullptr;

    for (size_t i = 0; i < len; i++) {
        int uch = s[i];

        if ((uch & ~0x7F) != 0) {
            if (buffer == nullptr) {
                buffer = new int8_t[8];
            }
            int mask = ~0x3F;
            int n = 0;

            do {
                buffer[n++] = (int8_t)(0x80 | (uch & 0x3F));
                uch >>= 6;
                mask >>= 1;
            } while ((uch & mask) != 0);

            buffer[n] = (int8_t)((mask << 1) | uch);

            do {
                seed = (seed * HASH_MULTIPLIER) ^ (buffer[n--] & 0xFF);
            } while (0 <= n);
        } else if (uch == 0) {
            seed = (seed * HASH_MULTIPLIER) ^ (0xC0);
            seed = (seed * HASH_MULTIPLIER) ^ (0x80);
        } else {
            seed = (seed * HASH_MULTIPLIER) ^ (uch);
        }
    }
    return seed;
}

export int unmasked_hash_code(const char8_t *s, int seed) {
    u8string str(s);
    return unmasked_hash_code(str, seed);
}

export int hash_code(u8string &s, int seed) {
    return unmasked_hash_code(s, seed) & POSITIVE_MASK;
}

export int hash_code(u8string &s) {
    return hash_code(s, HASH_MULTIPLIER);
}

export int hash_code(u8string &module, u8string &name, int seed) {
    seed = (seed * HASH_MULTIPLIER) ^ ('/');
    seed = unmasked_hash_code(module, seed);
    seed = (seed * HASH_MULTIPLIER) ^ ('/');
    seed = unmasked_hash_code(name, seed);
    return seed & POSITIVE_MASK;
}

export int hash_code(u8string &module, u8string &name) {
    return hash_code(module, name, HASH_MULTIPLIER);
}
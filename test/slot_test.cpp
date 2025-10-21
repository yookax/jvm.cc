#include "test.h"

import slot;

using namespace slot;

void test_slot() {
    slot_t s;
    slot::set<jref>(&s, (jref) 0x123456789);
    auto x = get<jref>(&s);
    printf("%p\n", x);

    slot::set<jref>(&s, nullptr);
    x = get<jref>(&s);
    printf("%p\n", x);
}
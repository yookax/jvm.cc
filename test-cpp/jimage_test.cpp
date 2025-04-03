#include "test.h"

TEST_CASE(test_jimage)
    JImageFile jf("d:/modules");
    jf.read();
    jf.print();
}
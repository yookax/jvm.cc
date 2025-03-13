#include "test.h"

TEST_CASE(test_sys_info)
    printf("processor number: %d\n", processor_number());
    printf("page size: %d\n", page_size());
    printf("os name: %s\n", os_name());
    printf("os arch: %s\n", os_arch());
    printf("is big endian?: %d\n", std::endian::native == std::endian::big);
}

TEST_CASE(test_properties)
    for (Property &p: g_properties) {
        printf("%s: %s\n", p.name, p.value);
    }
}

#include "test.h"

void test_sys_info() {
    printf("processor number: %d\n", processorNumber());
    printf("page size: %d\n", pageSize());
    printf("os name: %s\n", osName());
    printf("os arch: %s\n", osArch());
    printf("is big endian?: %d\n", std::endian::native == std::endian::big);
}

void test_properties() {
    // for (Property &p: g_properties) {
    //     printf("%s: %s\n", p.name, p.value);
    // }
}

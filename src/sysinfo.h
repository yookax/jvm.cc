#ifndef CABIN_SYSINFO_H
#define CABIN_SYSINFO_H

#include <cstdint>

int processor_number();
int page_size();

// 返回操作系统的名称。e.g. window 10
const char *os_name();

// 返回操作系统的架构。e.g. amd64
const char *os_arch();

const char *file_separator();
const char *path_separator();
const char *line_separator();

/*
 * 大端(big endian):低地址存放高字节
 * 小端(little endian):低字节存放低字节
 */
static inline bool is_big_endian() {
    static const union {
        char c[4]; 
        unsigned long l;
    } endian_test = { { 'l', '?', '?', 'b' } };

    return ((char) endian_test.l) == 'b';
}

int64_t current_time_milliseconds();

char *get_current_working_directory();

#endif // CABIN_SYSINFO_H
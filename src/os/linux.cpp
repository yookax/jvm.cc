#ifdef __linux__

#include <cassert>
#include <unistd.h>
#include <sys/time.h>
#include "../sysinfo.h"
#include "../cabin.h"

int processor_number() {
    return sysconf(_SC_NPROCESSORS_CONF);
}

int page_size() {
    JVM_PANIC("pageSize");
}

const char *os_name() {
    struct utsname x;
    uname(&x);
    return x.sysname;
}

const char *os_arch() {
    JVM_PANIC("osArch");
}

const char *file_separator() {
    return "/";
}

const char *path_separator() {
    return ":";
}

const char *line_separator() {
    return "\n";
}

char *get_current_working_directory() {
    char *cwd = NULL;
    int size = 256;

    while (true) {
        cwd = new char[size];

        if(getcwd(cwd, size) != NULL) {
            return cwd;
        }

        if(errno == ERANGE) {
            delete cwd;
            size *= 2;
        } else {
            JVM_PANIC("Couldn't get cwd");
        }
    }
}

void *find_library_entry(void *handle, const char *name) {
    assert(handle != nullptr && name != nullptr);
    unimplemented
}

void *open_library_os_depend(const char *name) {
    assert(name != nullptr);
    unimplemented
}

string find_jdk_dir() {
    unimplemented
}

#endif
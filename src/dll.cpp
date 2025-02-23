#include <cassert>
#include <string>
#include <sstream>
#include "cabin.h"
#include "classfile/class.h"
#include "classfile/method.h"
#include "classfile/class_loader.h"
#include "dll.h"

using namespace std;

void *g_libzip = nullptr;

static string mangle(const utf8_t *class_name, const utf8_t *method_name) {
    assert(class_name != nullptr && method_name != nullptr);

    ostringstream oss;

    oss << "Java_";
    for (const utf8_t *t = class_name; *t != 0; t++) {
        if (*t == '/') {
            // 将类名之间的'/'替换为'_'
            oss << '_';
        } else if (*t == '$') {
            // 将类名之间的'$'替换为"_00024"，比如
            // class: jdk/internal/util/SystemProps$Raw, method: platformProperties
            // Java_jdk_internal_util_SystemProps_00024Raw_platformProperties
            oss << "_00024";
        } else {
            oss << *t;
        }
    }

    oss << "_" << method_name;
    return oss.str();
}

// [lib name, lib handle]
static vector<pair<const char *, void *>> loaded_libs;

void *open_library_os_depend(const char *name);

void *open_library(const char *name) {
    for (auto &[lib_name, lib_handle]: loaded_libs) {
        if (strcmp(lib_name, name) == 0) {
            assert(lib_handle != nullptr);
            return lib_handle;
        }
    }

    void *handle = open_library_os_depend(name);
    if (handle != nullptr)
        loaded_libs.emplace_back(strdup(name), handle);
        
    return handle;
}

void close_library(void *handle) {
    unimplemented
}

void *find_native_from_boot_lib(const utf8_t *class_name, const utf8_t *method_name) {
    assert(class_name != nullptr && method_name != nullptr);

    string mangled = mangle(class_name, method_name);

    for (auto &[lib_name, lib_handle]: loaded_libs) {
        void *p = find_library_entry(lib_handle, mangled.c_str());
        if (p != nullptr)
            return p; // find out
    }

    return nullptr; // don't find
}

void init_dll() {
    char *boot_lib_path = get_boot_lib_path();    
    char path[PATH_MAX + 1];

    open_library(strcat(strcpy(path, boot_lib_path), "java.dll"));
    g_libzip = open_library(strcat(strcpy(path, boot_lib_path), "zip.dll"));
}

char *get_boot_lib_path() {
    static char boot_lib_path[PATH_MAX + 1] = { 0 };

    if (boot_lib_path[0] == 0)
        strcat(strcpy(boot_lib_path, g_java_home.c_str()), "/bin/");
    
    return boot_lib_path;
}



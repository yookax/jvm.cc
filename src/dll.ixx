module;
#include "cabin.h"

export module dll;

export void *g_libzip = nullptr;

export void init_dll();

export void *find_native_from_boot_lib(const utf8_t *class_name, const utf8_t *method_name);

export void *find_library_entry(void *handle, const char *name);

export char *get_boot_lib_path();

export void *open_library(const char *name);
export void close_library(void *handle);
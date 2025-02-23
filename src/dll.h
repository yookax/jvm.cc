#ifndef CABIN_DLL_H
#define CABIN_DLL_H

#include "cabin.h"

extern void *g_libzip;

void init_dll();

void *find_native_from_boot_lib(const utf8_t *class_name, const utf8_t *method_name);

void *find_library_entry(void *handle, const char *name);

char *get_boot_lib_path();

void *open_library(const char *name);
void close_library(void *handle);

#endif
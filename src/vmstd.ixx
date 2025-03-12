module;
#include "cabin.h"

export module vmstd;

export int processor_number();
export int page_size();
// 返回操作系统的名称。e.g. window 10
export const char *os_name();
// 返回操作系统的架构。e.g. amd64
export const char *os_arch();
export const char *file_separator();
export const char *path_separator();
export const char *line_separator();
export char *get_current_working_directory();


export bool is_prim_class_name(const char *class_name);
export bool is_prim_descriptor(char descriptor);
export bool is_prim_wrapper_class_name(const char *class_name);
export const char *get_prim_array_class_name(const char *class_name);
export const char *get_prim_class_name(char descriptor);
export const char *getPrimDescriptor(const char *wrapper_class_name);
export const char *get_prim_descriptor_by_class_name(const char *class_name);
export const char *get_prim_descriptor_by_wrapper_class_name(const char *wrapper_class_name);


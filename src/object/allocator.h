#ifndef JVMCC_ALLOCATOR_H
#define JVMCC_ALLOCATOR_H

#include "object.h"

/*
 * 所有从堆上申请对象的函数都在这里声明
 */

struct Allocator {
    // alloc non array object
    static Object *object(Class *c);

    // 一维数组
    static Object *array(ArrayClass *ac, jint arr_len);
    // alloc 一维数组
    static jarrRef array(jref class_loader, const char *arr_class_name, jint arr_len);
    // alloc 一维数组，使用 'BOOT_CLASS_LOADER'
    static jarrRef array(const char *arr_class_name, jint arr_len) {
        return array(BOOT_CLASS_LOADER, arr_class_name, arr_len);
    }

    static jarrRef string_array(jint arr_len) { return array("[Ljava/lang/String;", arr_len); }
    static jarrRef class_array(jint arr_len) { return array("[Ljava/lang/Class;", arr_len); }
    static jarrRef object_array(jint arr_len) { return array("[Ljava/lang/Object;", arr_len); }
    static jarrRef object_array(std::initializer_list<jref> os);

    // 多维数组
    static Object *multi_array(ArrayClass *ac, jint dim, const jint lens[]);

    static jstrRef string(const utf8_t *str);
    static jstrRef string(const unicode_t *str, jsize len);
};

#endif //JVMCC_ALLOCATOR_H

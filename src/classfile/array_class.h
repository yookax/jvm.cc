#ifndef CABIN_ARRAY_CLASS_H
#define CABIN_ARRAY_CLASS_H

#include "class.h"

class ArrayClass: public Class {
    Class *comp_class = nullptr; // component class
    Class *elt_class = nullptr;  // element class
    size_t elt_size = 0;
    
    // array class 由虚拟机直接生成。
    ArrayClass(Object *loader, const char *class_name);
    friend ArrayClass *load_array_class(Object *loader, const utf8_t *arr_class_name);

public:
    int dimension = 0; // 数组的维度

    // 判断数组单个元素的大小
    // 除了基本类型的数组外，其他都是引用类型的数组
    // 多维数组是数组的数组，也是引用类型的数组
    size_t get_element_size();

    /*
     * Returns the representing the component class of an array class.
     * If this class does not represent an array class this method returns null.
     *
     * like，依次调用 componentClass():
     * [[I -> [I -> int -> null
     */
    Class *get_component_class();

    /*
     * Returns the representing the element class of an array class.
     * If this class does not represent an array class this method returns null.
     *
     * like [[[I 的元素类是 int.class
     */
    Class *get_element_class();

    size_t object_size() const override { UNREACHABLE("ArrayClass does not support this method."); }
    // Object *alloc_object() override { UNREACHABLE("ArrayClass does not support this method."); }
    Object *alloc_native_object() override { UNREACHABLE("ArrayClass does not support this method."); }

    size_t object_size(jint arr_len);
};

#endif
#include <functional>
#include <string>
#include <sstream>

#include "allocator.h"
#include "../cabin.h"
#include "object.h"
#include "../exception.h"

using namespace std;

void *Object::index(jint i) const {
    assert(0 <= i && i < arr_len);
    auto ac = (ArrayClass *) clazz;
    return (void *) (((const u1 *) (data)) + ac->get_element_size() * i);
}

void Object::setRefElt(int i, jref value) {
    assert(0 <= i && i < arr_len);
    auto elt = (slot_t *) index(i);

    if (value == nullptr) {
        *elt = slot::rslot(nullptr);
    } else if (is_type_array()) {
        const slot_t *unbox = value->unbox();
        *elt = *unbox;
        if (is_long_array() || is_double_array())
            *++elt = *++unbox;
    } else {
        *elt = slot::rslot(value);
    }
}

void array_copy(jarrRef dst, jint dst_pos, const jarrRef src, jint src_pos, jint len) {
    assert(src != nullptr);
    assert(src->is_array_object());

    assert(dst != nullptr);
    assert(dst->is_array_object());

    if (len < 1) {
        // need to do nothing
        return;
    }

    auto src_elt_size = ((ArrayClass *)src->clazz)->get_element_size();
    auto dst_elt_size = ((ArrayClass *)dst->clazz)->get_element_size();

    /*
     * 首先确保src和dst都是数组，然后检查数组类型。
     * 如果两者都是引用数组，则可以拷贝，否则两者必须是相同类型的基本类型数组
     */
    if (src_elt_size != dst_elt_size) {
        throw java_lang_ArrayStoreException(
                    "src: " + to_string(src_elt_size) + " dst: " + to_string(dst_elt_size));
    }

    if (src_pos < 0
        || dst_pos < 0
        || src_pos + len > src->arr_len
        || dst_pos + len > dst->arr_len) {
        ostringstream oss;
        oss << "src:[" << src_pos << ", " << src_pos + len << "], ";
        oss << "dst:[" << dst_pos << ", " << dst_pos + len << "]" << ends;
        throw java_lang_ArrayIndexOutOfBoundsException(oss.str());
    }

    memcpy(dst->index(dst_pos), src->index(src_pos), dst_elt_size * len);
}

const char *arr_class_name_2_elt_class_name(const utf8_t *arr_class_name) {
    assert(arr_class_name != nullptr);
    assert(arr_class_name[0] == '['); // must be array class name

    const utf8_t *elt_name = arr_class_name;
    while (*++elt_name == '[');

    const utf8_t *prim_class_name = get_prim_class_name(*elt_name);
    if (prim_class_name != nullptr) {  // primitive type
        return prim_class_name;
    }

    // 普通类: Lxx/xx/xx; 型
    assert(*elt_name == 'L');
    elt_name++; // jump 'L'

    size_t last = strlen(elt_name) - 1;
    assert(last > 0);
    assert(elt_name[last] == ';');

    auto buf = new char[last + 1];//(char *)vm_malloc(sizeof(*buf) * (last + 1));
    strncpy(buf, elt_name, (size_t) last);
    buf[last] = 0;
    return buf;
}

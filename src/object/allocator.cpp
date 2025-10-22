module;
#include <assert.h>
#include "../vmdef.h"

module object;

import std;
import heap;

using namespace std;

Object *Allocator::object(Class *c) {
    assert(c != nullptr);
    if (c->is_array_class()) {
        UNREACHABLE("");
    }

    size_t size = c->object_size();
    return new (g_heap->alloc(size)) Object(c);
}

Object *Allocator::array(ArrayClass *ac, jint arr_len) {
    assert(ac != nullptr);

    size_t size = ac->object_size(arr_len);
    return new (g_heap->alloc(size)) Object(ac, arr_len);
}

jarrRef Allocator::array(jref class_loader, const char *arr_class_name, jint arr_len) {
    assert(arr_class_name != nullptr);

    auto ac = load_array_class(class_loader, arr_class_name);
    assert(ac != nullptr);

    return Allocator::array(ac, arr_len);
}

jarrRef Allocator::object_array(std::initializer_list<jref> os) {
    jarrRef a = object_array((jint) os.size());
    int i = 0;
    for (jref o: os)
        a->setRefElt(i++, o);
    return a;
}

Object *Allocator::multi_array(ArrayClass *ac, jint dim, const jint lens[]) {
    assert(ac != nullptr);
    assert(dim >= 1);
    assert(lens != nullptr);

    size_t size = ac->object_size(lens[0]);
    return new (g_heap->alloc(size)) Object(ac, dim, lens);
}

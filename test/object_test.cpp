#include "test.h"

using namespace std;

void test_box() {
    if (!std::holds_alternative<std::monostate>(void_box()->box_value())) {
        std::cout << "failed" << std::endl;
    }

    auto v = int_box(100)->box_value();
    if (std::holds_alternative<int>(v)) {
        auto x = std::get<jint>(v);
        if (x != 100)
            std::cout << "failed" << std::endl;
    }

    v = double_box(100.67)->box_value();
    if (std::holds_alternative<jdouble>(v)) {
        auto x = std::get<jdouble>(v);
        if (x != 100.67)
            std::cout << "failed" << std::endl;
    }
}

// --------------------------------------

static const utf8_t *utf8s[] = {
        "Hello, World!",
        "你好，世界！",
        "こんにちは、世界！",
};

void test_string1() {
    for (auto s: utf8s) {
        auto so = Allocator::string(s);
        auto u = java_lang_String::to_utf8(so);
        if (!utf8::equals(s, u)) {
            printf("failed\n");
        }
    }
}

void test_intern() {
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        auto intern1 = java_lang_String::intern(so1);
        auto intern2 = java_lang_String::intern(so2);

        if (intern1 != intern2)
            printf("failed\n");
    }
}

void test_equals() {
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        if (!java_lang_String::equals(so1, so2))
            printf("failed\n");
    }
}

// --------------------------------

static void init_int_array(jarrRef arr) {
    jint len = arr->array_len();
    jint dim = arr->array_dimension();

    if (dim == 1) {
        for (jint i = 0; i < len; i++)
            arr->setIntElt(i, i + 1);
        return;
    }

    for (jint i = 0; i < len; i++) {
        init_int_array(arr->getElt<jarrRef>(i));
    }
}

void test_new_array() {
    jarrRef arr = Allocator::array("[I", 10);
    arr->display(); // should be all 0

    printf("--------------------------------\n");
    init_int_array(arr);
    arr->display();
}

void test_multi_array1() {
    ArrayClass *ac = load_array_class(BOOT_CLASS_LOADER, "[[[I");
    jint dim = 3;
    jint lens[3];
    lens[0] = 2;
    lens[1] = 3;
    lens[2] = 4;
    jarrRef arr = Allocator::multi_array(ac, dim, lens);

    arr->display(); // should be all 0
    printf("--------------------------------\n");

    init_int_array(arr);
    arr->display();
}

void test_multi_array2() {
    printf("test_multi_array2 ---->\n");
    ArrayClass *ac = load_array_class(BOOT_CLASS_LOADER, "[[[[[I");
    jint dim = 5;
    jint lens[5];
    lens[0] = 2;
    lens[1] = 3;
    lens[2] = 4;
    lens[3] = 3;
    lens[4] = 2;
    jarrRef arr = Allocator::multi_array(ac, dim, lens);

    arr->display(); // should be all 0
    printf("--------------------------------\n");

    init_int_array(arr);
    arr->display();
}

void test_string_array() {
    printf("test_string_array ---->\n");
    auto array = Allocator::string_array(10);
    array->display();  // should be all null

    auto s = Allocator::string("aaa");
    array->setRefElt(0, s);
    array->display();
}

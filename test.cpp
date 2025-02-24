#include <unordered_set>
#include <random>
#include "src/slot.h"
#include "src/runtime//heap.h"
#include "src/classfile/class.h"
#include "src/object/object.h"
#include "src/classfile/invoke.h"
#include "src/classfile/descriptor.h"
#include "src/sysinfo.h"
#include "src/jni.h"
#include "src/encoding.h"
#include "src/object/allocator.h"

using namespace std;
using namespace slot;

static void test_sys_info() {
    printf("test_sys_info ---->\n");
    printf("processor number: %d\n", processor_number());
    printf("page size: %d\n", page_size());
    printf("os name: %s\n", os_name());
    printf("os arch: %s\n", os_arch());
    printf("is big endian?: %d\n", is_big_endian());
}

static void test_properties() {
    printf("test_properties ---->\n");
    for (Property &p: g_properties) {
        printf("%s: %s\n", p.name, p.value);
    }
}

static const char *method_descriptors[] = {
        "()V",
        "(I)V",
        "(B)C",
        "(Ljava/lang/Integer;)V",
        "(Ljava/lang/Object;[[BLjava/lang/Integer;[Ljava/lang/Object;)V",
        "(II[Ljava/lang/String;)Ljava/lang/Integer;",
        "([Ljava/io/File;)Ljava/lang/Object;",
        "(J[Ljava/io/File;Ljava/io/File;D)Ljava/lang/Object;",
        "([[[Ljava/lang/Double;)[[Ljava/lang/Object;",
        "(ZBSIJFD)[[Ljava/lang/String;",
        "(ZZZZZZZZZZZZZZZZ)Z",
};

static void test_method_descriptor() {
    printf("test_method_descriptor ---->\n");

#define PRINT_PTYPES(ptypes) \
    for (int i = 0; i < (ptypes)->arr_len; i++) { \
        jref o = (ptypes)->getElt<jref>(i); \
        printf("%s, ", o->jvm_mirror->name); \
    }

    for (const char *d : method_descriptors) {
        printf("%s\n\tparameters number: %d\n", d, numEltsInMethodDescriptor(d));
        printf("\tparameter slots number: %d\n\tparameter types: ", numSlotsInMethodDescriptor(d));

        pair<jarrRef /*ptypes*/, jclsRef /*rtype*/> p = parseMethodDescriptor(d, BOOT_CLASS_LOADER);
        PRINT_PTYPES(p.first)
        printf("\n\treturn type: %s\n", p.second->jvm_mirror->name);
        printf("\tunparse: %s\n\t-----\n\tparameter types: ", unparseMethodDescriptor(p.first, p.second).c_str());

        jref mt = findMethodType(d, BOOT_CLASS_LOADER);
        jarrRef ptypes = mt->get_field_value<jref>("ptypes", "[Ljava/lang/Class;");
        PRINT_PTYPES(ptypes)

        printf("\n\tunparse: %s\n", unparseMethodDescriptor(mt).c_str());
    }


    printf("---\nunparse(NULL, NULL): %s\n", unparseMethodDescriptor(nullptr, nullptr).c_str());
}

// ------------------------------------------------------------------------------------------------

static void test_inject_field() {
    printf("test_inject_field ---->\n");
    // const char *class_names[] = {
    //     "java/lang/Object",
    //     "java/lang/Class",
    //     "java/lang/Object", // 第二次注入 java/lang/Object
    // };

    // for (const char *class_name : class_names) {
    //     Class *c = loadBootClass(class_name);
    //     printf("%s\n", c->name);

    //     // 因为 injectInstField 只能在 loaded 之后进行，
    //     // 所以这里为了测试强制设置一下。
    //     Class::State state = c->state;
    //     c->state = Class::State::LOADED;
    //     bool b1 = c->injectInstField("inject1", "C");
    //     bool b2 = c->injectInstField("inject2", "I");
    //     bool b3 = c->injectInstField("inject3", "J");
    //     c->state = state;

    //     printf("\t%d, %d, %d\n", b1, b2, b3);
    //     printf("\t%s\n", c->toString().c_str());
    // }
}


// ------------------------------------------------------------------------------------------------

static void test_utf8_equals() {
    bool x = false;
    for (int i = 0; i < 190555; i++) {
        x = utf8::equals("abcdefeeinerinviernvienvir", "abcdefeeinerinviernvienvir");
    }
    cout << x << endl;
}

static void test_slot() {
    slot_t s;
    set<jref>(&s, (jref) 0x123456789);
    auto x = get<jref>(&s);
    printf("%p\n", x);

    set<jref>(&s, nullptr);
    x = get<jref>(&s);
    printf("%p\n", x);
}

// ------------------------------------------------------------------------------------------------

static void test_alloc_continuously() {
    printf("test_alloc_continuously ===>\n");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1024*1024*1024);

    for (int i = 0; i < 100; i++) {
        auto heap = new Heap(dis(gen));
        int alloced_size = 0;

        for (auto free_size = heap->count_free_memory(); free_size > 0; ) {
            // 创建一个均匀分布，范围是[1, size]
            std::uniform_int_distribution<> d(1, free_size);
            auto len = d(gen);
            alloced_size += len;
            heap->alloc(len);

            free_size = heap->count_free_memory();
            if (alloced_size + free_size != heap->get_size()) {
                delete heap;
                printf("failed");
            }
        }

        delete heap;
    }
}

static void test_heap1() {
    printf("test_heap1 ===>\n");
    auto heap = new Heap(1024);

    // 创建一个随机数引擎，使用默认的随机数引擎
    std::random_device rd;
    std::mt19937 gen(rd());

    for (auto free_memory = heap->count_free_memory();
            free_memory > 0; free_memory = heap->count_free_memory()) {
        // 创建一个均匀分布，范围是[1, heap->get_size()]
        std::uniform_int_distribution<> dis(1, free_memory);

        auto len = dis(gen);
        // printf("%d------%d\n", free_memory,len);
        if (free_memory - len < 10)
            break;
        void *p = heap->alloc(len);
        heap->display();
            }

    printf("------");

    heap->display();

    void *p1 = heap->alloc(2);
    heap->display();

    void *p2= heap->alloc(2);
    heap->display();

    void *p3 = heap->alloc(2);
    heap->display();

    void *p4 = heap->alloc(2);
    heap->display();

    heap->recycle((address_t)p2, 2);
    heap->display();

    heap->recycle((address_t)p3, 2);
    heap->display();

    heap->recycle((address_t)p4, 2);
    heap->display();

    delete heap;
}

void test_heap() {
    test_alloc_continuously();
    test_heap1();
}

// ------------------------------------------------------------------------------------------------

static void test_load_class() {
    printf("-------------- testLoadClass ------------------\n");
    const char *class_names[] = {
        "boolean", "java/lang/Class"
    };

    for (const char *class_name : class_names) {
        Class *c = load_boot_class(class_name);
        printf("%p, %s, %s, %d\n", c, c->name,
                        c->java_mirror->jvm_mirror->name,
                        c == c->java_mirror->jvm_mirror);
    }
}

static void test_classloader1() {
    printf("-------------- test_class_loader ------------------\n");
    const unordered_set<const Object *> &_loaders = getAllClassLoaders();
    printf("loaders count: %lld\n", _loaders.size());
    for (auto loader: _loaders) {
        if (loader == BOOT_CLASS_LOADER)
            printf("boot class loader\n");
        else
            printf("%s\n", loader->clazz->name);
    }

    auto bc = getAllBootClasses();
    printf("---\n\tboot classes count: %d\n", bc->size());
    for (auto &p: *bc) {
        printf("\t\t%s\n", p.first);
    }
}

void test_classloader() {
    test_load_class();
    test_classloader1();
}

// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

static const utf8_t *utf8s[] = {
    "Hello, world",
    "你好，世界",
    "こんにちは、世界",
};

static void test_string1() {
    printf("test_string1 ---->\n");
    for (auto s: utf8s) {
        auto so = Allocator::string(s);
        auto u = java_lang_String::to_utf8(so);
        if (!utf8::equals(s, u)) {
            printf("failed\n");
        }
    }
}

static void test_intern() {
    printf("test_intern ---->\n");
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        auto intern1 = java_lang_String::intern(so1);
        auto intern2 = java_lang_String::intern(so2);

        if (intern1 != intern2)
            printf("failed\n");
    }
}

static void test_equals() {
    printf("test_equals ---->\n");
    for (auto s: utf8s) {
        auto so1 = Allocator::string(s);
        auto so2 = Allocator::string(s);

        if (!java_lang_String::equals(so1, so2))
            printf("failed\n");
    }
}

void test_string() {
    test_string1();
    test_intern();
    test_equals();
}

// ------------------------------------------------------------------------------------------------

static void init_int_array(jarrRef arr) {
    jint len = arr->array_len();
    jint dim = arr->array_dimension();

    if (dim == 1) {
        for (jint i = 0; i < len; i++)
            arr->setIntElt(i, i+1);
        return;
    }

    for (jint i = 0; i < len; i++) {
        init_int_array(arr->getElt<jarrRef>(i));
    }
}

static void test_new_array() {
    printf("test_new_array ---->\n");
    jarrRef arr = Allocator::array("[I", 10);
    arr->display(); // should be all 0

    printf("--------------------------------\n");
    init_int_array(arr);
    arr->display();
}

static void test_multi_array1() {
    printf("test_multi_array1 ---->\n");
    ArrayClass *ac = load_array_class(BOOT_CLASS_LOADER, "[[[I");
    jint dim = 3;
    jint lens[] = { 2, 3, 4 };
    jarrRef arr = Allocator::multi_array(ac, dim, lens);

    arr->display(); // should be all 0
    printf("--------------------------------\n");

    init_int_array(arr);
    arr->display();
}

static void test_multi_array2() {
    printf("test_multi_array2 ---->\n");
    ArrayClass *ac = load_array_class(BOOT_CLASS_LOADER, "[[[[[I");
    jint dim = 5;
    jint lens[] = { 2, 3, 4, 3, 2 };
    jarrRef arr = Allocator::multi_array(ac, dim, lens);

    arr->display(); // should be all 0
    printf("--------------------------------\n");

    init_int_array(arr);
    arr->display();
}

static void test_string_array() {
    printf("test_string_array ---->\n");
    auto array = Allocator::string_array(10);
    array->display();  // should be all null

    auto s = Allocator::string("aaa");
    array->setRefElt(0, s);
    array->display();
}

void test_array() {
    test_new_array();
    test_multi_array1();
    test_multi_array2();
    test_string_array();
}

// ------------------------------------------------------------------------------------------------

int main() {
    test_slot();
    test_utf8_equals();

    JNI_CreateJavaVM(nullptr, nullptr, nullptr);

    test_heap();
    test_classloader();
    test_box();
    test_string();
    test_array();

    test_properties();
    test_sys_info();
    test_method_descriptor();
    test_inject_field();

    cout << endl << "Testing is over." << endl;
}

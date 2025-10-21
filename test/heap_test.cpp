#include "test.h"

using namespace std;

void test_alloc_continuously() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1024 * 1024 * 1024);

    for (int i = 0; i < 100; i++) {
        auto heap = new Heap(dis(gen));
        int alloced_size = 0;

        for (auto free_size = heap->countFreeMemory(); free_size > 0;) {
            // 创建一个均匀分布，范围是[1, size]
            std::uniform_int_distribution<> d(1, free_size);
            auto len = d(gen);
            alloced_size += len;
            heap->alloc(len);

            free_size = heap->countFreeMemory();
            if (alloced_size + free_size != heap->getSize()) {
                delete heap;
                printf("failed");
            }
        }

        delete heap;
    }
}

void test_heap1() {
    auto heap = new Heap(1024);

    // 创建一个随机数引擎，使用默认的随机数引擎
    std::random_device rd;
    std::mt19937 gen(rd());

    for (auto free_memory = heap->countFreeMemory();
        free_memory > 0; free_memory = heap->countFreeMemory()) {
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

    void *p2 = heap->alloc(2);
    heap->display();

    void *p3 = heap->alloc(2);
    heap->display();

    void *p4 = heap->alloc(2);
    heap->display();

    heap->recycle((address_t) p2, 2);
    heap->display();

    heap->recycle((address_t) p3, 2);
    heap->display();

    heap->recycle((address_t) p4, 2);
    heap->display();

    delete heap;
}
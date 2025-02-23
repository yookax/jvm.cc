#ifndef CABIN_HEAP_H
#define CABIN_HEAP_H

#include <cstddef>
#include <cstdint>
#include <mutex>

class Object;
typedef uintptr_t address_t;

class Heap {
    const size_t size; // The total size of the heap, measured in bytes.
    const address_t starting_address; // The starting address of the heap.

    struct Chunk {
        address_t head;
        size_t len;
        Chunk *next;

        Chunk(address_t head0, size_t len0, Chunk *next0)
            : head(head0), len(len0), next(next0) { }
    } *freelist;

    /*mutable*/ std::recursive_mutex mutex;

    address_t jump_freelist(address_t p);

    void *do_alloc(size_t len);


public:
    explicit Heap(size_t heap_size);
    ~Heap();

    void collect_garbage();
    size_t get_size() const { return size; }

    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    void *alloc(size_t len);
    void recycle(address_t p, size_t len);

    bool is_in(address_t p) const {
        bool b = (starting_address <= p) && (p < starting_address + size);
        return b;
    }
    
    // 堆还有多少剩余空间，以字节为单位。
    size_t count_free_memory() const;

    bool find(Object *o);
    void traversal(void (*)(Object *));

    void display();
};

#endif //CABIN_HEAP_H

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "../cabin.h"
#include "../slot.h"
#include "../classfile/class.h"
#include "../object/object.h"
#include "frame.h"
#include "thread.h"
#include "heap.h"

using namespace std;
using namespace slot;

void gc();

Heap::Heap(size_t heap_size): size(heap_size), starting_address((address_t) malloc(heap_size)) {
    assert(starting_address != 0);
    freelist = new Heap::Chunk(starting_address, size, nullptr);
}

Heap::~Heap() {
    for (Heap::Chunk *p = freelist; p != nullptr;) {
        Heap::Chunk *t = p->next;
        delete p;
        p = t;
    }

    std::free((void *) starting_address);
}

/*
 * 如果不在 freelist 里面，返回 p，
 * 否则跳过此 free chunk
 * 如果到堆尾，返回0
 */
address_t Heap::jump_freelist(address_t p) {
    assert(is_in(p));

    for (auto curr = freelist; curr != nullptr; curr = curr->next) {
        if (p < curr->head)
            return p; // p is not in freelist
        address_t offset = p - curr->head;
        if (0 <= offset && offset < curr->len) {
            // p is in freelist, jump
            p += curr->len;
            break;
        }
    }

    // now, p is not in freelist.

    if (p >= starting_address + size)
        return 0; // 到达heap的尾部, 返回0.
    return p;
}

void *Heap::do_alloc(size_t len) {
    lock();

    void *p = nullptr;
    Chunk *prev = nullptr;
    Chunk *curr = freelist;
    for (; curr != nullptr; prev = curr, curr = curr->next) {
        if (curr->len == len) {
            if (prev != nullptr) {
                prev->next = curr->next;
            } else {
                assert(curr == freelist);
                freelist = curr->next;
            }
            p = (void *) curr->head;
            delete curr;

            goto over;
        }

        if (curr->len > len) {
            p = (void *) (curr->head);
            curr->head += len;
            curr->len -= len;

            goto over;
        }
    }

over:
    unlock();

    if (p != nullptr) {
        memset(p, 0, len);
        return p;
    }
    return nullptr;
}

void *Heap::alloc(size_t len) {
    void *p = do_alloc(len);
    if (p != nullptr)
        return p;

//    gc();

    //p = do_alloc(len);
    //if (p != nullptr)
    //    return p;

//    throw "java_lang_OutOfMemoryError";
    panic("java_lang_OutOfMemoryError"); // todo 堆可以扩张
}

void Heap::recycle(address_t p, size_t len) {
    assert(is_in(p));

   // lock();

    Chunk *prev = nullptr;
    Chunk *curr = freelist;
    for (; curr != nullptr; prev = curr, curr = curr->next) {
        if (p > curr->head)
            continue;

        assert(p + len <= curr->head);
        if (prev == nullptr) {
            assert(curr == freelist);
            if (p + len == curr->head) { // 空间右连续
                curr->head = p;
                curr->len += len;
                goto over;
            } else { // 右不连续
                freelist = new Chunk(p, len, curr);
                goto over;
            }
        }

        assert(prev != nullptr);
        assert(prev->head + prev->len <= p);
        if (prev->head + prev->len == p) {
            if (p + len == curr->head) {
                // 左右都连续
                prev->len += (len + curr->len);
                prev->next = curr->next;
                delete curr;
                goto over;
            } else {
                // 左连续，右不连续
                prev->len += len;
                goto over;
            }
        } else {
            if (p + len == curr->head) {
                // 左不连续，右连续
                curr->len += len;
                curr->head = p;
                goto over;
            } else {
                // 左右都不连续
                prev->next = new Chunk(p, len, curr);
                goto over;
            }
        }
    }

over:
    ;
   // unlock();
}

size_t Heap::count_free_memory() const {
    size_t free_mem = 0;
    for (Chunk *chunk = freelist; chunk != nullptr; chunk = chunk->next) {
        free_mem += chunk->len;
    }
    return free_mem;
}

bool Heap::find(Object *o) {
    if (o == nullptr)
        return false;

    const address_t end = starting_address + size;
    
    for (auto curr = starting_address; curr < end;) {
        curr = jump_freelist(curr);
        if (curr == 0)
            return false;
        auto obj = (Object *) curr;
        if (obj == o)
            return true;
        curr += obj->size();
    }

    return false;
}

void Heap::traversal(void (* touch)(Object *)) {
    const address_t end = starting_address + size;
    
    for (auto curr = starting_address; curr < end;) {
        curr = jump_freelist(curr);
        if (curr == 0)
            return;
        auto obj = (Object *) curr;
        touch(obj);
        curr += obj->size();
    }
}

void Heap::collect_garbage() {
    const address_t end = starting_address + size;

    for (auto curr = starting_address; curr < end;) {
        curr = jump_freelist(curr);
        if (curr == 0)
            return;
        auto o = (Object *) curr;
        if (!o->reachable) {
            //printf("collect：%p, %s\n", o, o->clazz->name);
            recycle((address_t) o, o->size());
        }
        curr += o->size();
    }
}

void Heap::display() {
    lock();

    printf("freelist: \n|");
    for (Chunk *node = freelist; node != nullptr; node = node->next) {
        printf("%p,", (void *) node->head);
        cout << node->len << "|";
    }

    printf("\n");
    unlock();
}

// 判断slot存放的是不是一个Java Object Reference
static bool is_java_object_reference(const slot_t *slot) {
    assert(slot != nullptr);

    address_t p = *slot;
    if (!g_heap->is_in(p)) {
        return false;
    }

    jref o = slot::get<jref>(slot);
    // assert(o != nullptr); todo
    if (g_heap->find(o)) {
        /*
         * 至此slot中的值和堆中的一个对象的地址相等，
         * 但不能完全确定slot中存放的就是一个对象引用，
         * 也可能存放的是一个数字，数值恰好和对象的地址相等。
         * 这种情况下会错误的将此对象标记为可达，导致无法回收，
         * 不过这种情况比较罕见，影响不大。
         */
        return true;
    }

    return false;
}

/*
 * 分析一个可达的对象，此对象引用的对象也是可达的
 */
static void analysis_reachable_object(jref obj) {
    assert(obj != nullptr);
    assert(obj->reachable); // 分析一个可达的对象

    if (obj->is_type_array())
        return;

    if (obj->is_reference_array()) {
        jarrRef arr = obj;
        for (int i = 0; i < arr->arr_len; i++) {
            jref o = arr->getElt<jref>(i);
            if (o != nullptr) {
                o->reachable = true; // 此对象可达
                analysis_reachable_object(o);
            }
        }

        return;
    }

    // Non-array object
    int count = obj->clazz->inst_fields_count;
    for (int i = 0; i < count; i++) {
        slot_t *slot = obj->data + i;
        if (is_java_object_reference(slot)) {
            jref o = slot::get<jref>(slot);
            o->reachable = true;
            analysis_reachable_object(o);
        }
    }
}

static void analysis_class(Class *c) {
    assert(c != nullptr);

    // 1. 分析类静态属性引用的对象
    for (Field *f: c->fields) {
        if (!f->isStatic() || f->is_prim_field())
            continue;

        // static and reference field of a class
        jref o = f->static_value.r;
        o->reachable = true;
        analysis_reachable_object(o);
    }

    // 2. 分析类对象中引用的对象
    Object *co = c->java_mirror;
    co->reachable = true;
    analysis_reachable_object(co);
}

/*
 * 判断对象是否可达(GC Roots Analysis)
 *
 * 可作为GC Roots对象的包括如下几种：
    a.虚拟机栈(栈桢中的本地变量表)中的引用的对象  todo 操作栈中的引用的对象呢？？？？？
    b.方法区中的类静态属性引用的对象
    c.方法区中的常量引用的对象
    d.本地方法栈中JNI的引用的对象
    e.ClassObject对象（保存在本地内存）中所引用的对象
 */
static void reachability_analysis() {
    /* 虚拟机栈(栈桢中的本地变量表)中的引用的对象 */
    ALL_JAVA_THREADS(t) {
        for (Frame *frame = t->top_frame; frame != nullptr; frame = frame->prev) {
            slot_t *lvars = frame->lvars;
            u2 max_locals = frame->method->max_locals;

            for (u2 i = 0; i < max_locals; i++) {
                if (is_java_object_reference(lvars + i)) {
                    jref o = slot::get<jref>(lvars + i);
                    o->reachable = true;
                    analysis_reachable_object(o);
                }
            }

            // todo How about frame->ostack？？？？
        }
    }

    /* 类静态属性引用的对象 和 类对象中引用的对象 */
//    traverseAllLoadedClasses(analysis_class);


    // const unordered_set<const Object *> &loaders = getAllClassLoaders();
    // for (auto loader: loaders) {
    //     unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *classes;

    //     if (loader == BOOT_CLASS_LOADER) {
    //         classes = getAllBootClasses();
    //     } else {
    //         classes = loader->classes;
    //     }
    //     assert(classes != nullptr);

    //     for (auto &p: *classes) {
    //         Class *c = p.second;
    //         analysisClass(c);
    //     }
    // }

    // todo ..... c,d 中的判断
}

void gc() {
    g_heap->lock();

    // 将全部对象标记为不可达
    g_heap->traversal([](Object *obj) {
        assert(obj != nullptr);
        obj->reachable = false;
//        if (!reachable(obj)) {
//            obj->marked = 1;
//            // todo 调用 finalize() 后进行二次标记，然后才可以归还
//            heap_free(heap, mem, obj->size());
//        }
    });

    reachability_analysis();
    g_heap->collect_garbage();

    g_heap->unlock();
}

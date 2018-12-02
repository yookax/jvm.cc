/*
 * Author: Jia Yang
 */

#ifndef JVM_JOBJECT_H
#define JVM_JOBJECT_H

#include "../../slot.h"
#include "../ma/jclass.h"

enum jobject_type {
    NORMAL_OBJECT,
    ARRAY_OBJECT,
    STRING_OBJECT,
    CLASS_OBJECT
};

struct jobject {
    /*
     * 对象头
     */
    struct objhead {

    } head;

    // 保存所有实例变量的值
    // 包括此Object中定义的和从父类继承来的。
    int instance_fields_count;
    struct slot *instance_fields_values;

    struct jclass *jclass;

    union {
        struct { // array object
            jint len; // len of array
            size_t ele_size;  // size of single element
            s1 *data;
        } a;

        struct { // string object
            const char *str; // key in hash table
            const jchar *wstr;
        } s;

        struct { // class object
            // 必须是全限定类名，用作 hash 表中的 key
            // 形如 xxx/xxx/xxx 的形式
            char class_name[FILENAME_MAX];
        } c;
    };

    enum jobject_type t;

    void *extra;
};

struct jobject* jobject_create(struct jclass *c);

void set_instance_field_value_by_id(const struct jobject *o, int id, const struct slot *value);
void set_instance_field_value_by_nt(const struct jobject *o,
                                    const char *name, const char *descriptor, const struct slot *value);

const struct slot* get_instance_field_value_by_id(const struct jobject *o, int id);
const struct slot* get_instance_field_value_by_nt(const struct jobject *o, const char *name, const char *descriptor);

struct jobject* jstrobj_create(struct classloader *loader, const char *str);
const char* jstrobj_value(struct jobject *so);

/*
 * @jclass_class: class of java/lang/Class
 */
struct jobject* jclassobj_create(struct jclass *jclass_class, const char *class_name);

/*
 * 创建一维数组
 * todo 说明 c 是什么东西
 */
struct jobject *jarrobj_create(struct jclass *arr_class, jint arr_len);

/*
 * 创建多维数组
 * todo 说明 c 是什么东西
 */
struct jobject *jarrobj_create_multi(struct jclass *arr_class, size_t arr_dim, const size_t *arr_lens);

#define STROBJ_CHECK(so) \
    do { \
        if ((so) == NULL) { \
            jvm_abort("NULL point\n"); \
        } \
        if ((so)->t != STRING_OBJECT) { \
            jvm_abort("is not string object. %d\n", (so)->t); \
        } \
    } while (false)

#define ARROBJ_CHECK(ao) \
    do { \
        if ((ao) == NULL) { \
            jvm_abort("NULL point\n"); \
        } \
        if ((ao)->t != ARRAY_OBJECT) { \
            jvm_abort("is not array object. %d\n", (ao)->t); \
        } \
    } while (false)

#define CLSOBJ_CHECK(co) \
    do { \
        if ((co) == NULL) { \
            jvm_abort("NULL point\n"); \
        } \
        if ((co)->t != CLASS_OBJECT) { \
            jvm_abort("is not class object. %d\n", (co)->t); \
        } \
    } while (false)

/*
 * 判断两个数组是否是同一类型的数组
 * todo 这里的判断略简陋
 */
bool jarrobj_is_same_type(const struct jobject *one, const struct jobject *other);

bool jarrobj_check_bounds(const struct jobject *o, jint index);

void jarrobj_copy(struct jobject *dst, jint dst_pos, const struct jobject *src, jint src_pos, jint len);

void* jarrobj_index(struct jobject *ao, jint index);

void jobject_destroy(struct jobject *o);


bool jobject_is_instance_of(const struct jobject *o, const struct jclass *c);


const char* jobject_to_string(struct jobject *o);

//void jobject_set_field_value(struct jobject *o, int id, const struct slot *v);

/*
 * set filed value by name and type
 */
//void jobject_set_field_value_nt(struct jobject *o, const char *name, const char *descriptor, struct slot *v);

//struct slot* jobject_instance_field_value(struct jobject *o, int id);

/*
 * get the point of filed value by name and type
 */
//struct slot* jobject_instance_field_value_nt(struct jobject *o, const char *name, const char *descriptor);


#endif //JVM_JOBJECT_H

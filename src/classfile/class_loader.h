#ifndef CABIN_CLASS_LOADER_H
#define CABIN_CLASS_LOADER_H

#include <unordered_set>
#include <unordered_map>
#include "../cabin.h"
#include "constants.h"
#include "../encoding.h"
#include "../jni.h"

class ArrayClass;

void set_bootstrap_classpath(const char *bcp);

JNIEXPORT void set_classpath(const char *cp);
const char *get_classpath();

// Cache 常用的类
extern Class *g_object_class;
extern Class *g_class_class;
extern Class *g_string_class;

void init_classloader();

/*
 * 加载 JDK 类库中的类，不包括Array Class.
 * xxx/xxx/xxx
 */
Class *load_boot_class(const utf8_t *name);

ArrayClass *load_array_class(Object *loader, const utf8_t *arr_class_name);

// Load byte[].class, boolean[].class, char[].class, short[].class, 
//      int[].class, float[].class, long[].class, double[].class.
ArrayClass *load_type_array_class(ArrayType type);

const utf8_t *get_boot_package(const utf8_t *name);

using utf8_set = std::unordered_set<const utf8_t *, utf8::Hash, utf8::Comparator>;

utf8_set &get_boot_packages();

/*
 * @name: 全限定类名，不带 .class 后缀
 *
 * class names:
 *    - primitive types: boolean, byte, int ...
 *    - primitive arrays: [Z, [B, [I ...
 *    - non-array classes: java/lang/Object ...
 *    - array classes: [Ljava/lang/Object; ...
 */
Class *loadClass(Object *class_loader, const utf8_t *name);

Class *find_loaded_class(Object *class_loader, const utf8_t *name);

Class *define_class(jref class_loader, const u1 *bytecode, size_t len);

Class *define_class(jref class_loader, const char *name, const jbyte *buf,
                   jsize len, jref pd, const char *source);

// Define a class with the specified lookup class.
Class *define_class(Class *lookup, const char *name, const jbyte *buf,
                   jsize len, jref pd, jboolean init, int flags, jref class_data);

Class *define_class(jref class_loader, jstrRef name,
                   jarrRef bytecode, jint off, jint len, jref protection_domain);

// Class *defineClass(jref class_loader, jref name,
//                      jarrRef bytecode, jint off, jint len, jref protection_domain, jref source);

/*
 * 类的初始化在下列情况下触发：
 * 1. 执行new指令创建类实例，但类还没有被初始化。
 * 2. 执行 putstatic、getstatic 指令存取类的静态变量，但声明该字段的类还没有被初始化。
 * 3. 执行 invokestatic 调用类的静态方法，但声明该方法的类还没有被初始化。
 * 4. 当初始化一个类时，如果类的超类还没有被初始化，要先初始化类的超类。
 * 5. 执行某些反射操作时。
 *
 * 每个类的此方法只会执行一次。
 *
 * 调用类的类初始化方法。
 * clinit are the static initialization blocks for the class, and static Field initialization.
 */
Class *init_class(Class *c);

Class *link_class(Class *c);

jref get_platform_classloader();

/* todo
 * 返回 System Class Loader(sun/misc/Launcher$AppClassLoader) to loader user classes.
 *
 * 继承体系为：
 * java/lang/Object
 *     java/lang/ClassLoader
 *         java/security/SecureClassLoader
 *             java/net/URLClassLoader
 *                 sun/misc/Launcher$AppClassLoader
 */
jref get_app_classloader();


#define IS_SLASH_CLASS_NAME(class_name) (strchr(class_name, '.') == NULL)
#define IS_DOT_CLASS_NAME(class_name) (strchr(class_name, '/') == NULL)

std::unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *getAllBootClasses();
const std::unordered_set<const Object *> &getAllClassLoaders();

/*
 * 遍历已经加载的所有类，每个遍历出的类被命名为`clazz`
 *
 * 比如要输出所有已经加载的所有类的类名，可以使用以下代码：
 * TRAVERSE_ALL_LOADED_CLASSES({
 *     std::cout << c->toString().c_str() << std::endl;
 * });
 */
#define ALL_LOADED_CLASSES(clazz, code_block) \
    do { \
        const std::unordered_set<const Object *> &_loaders_ = getAllClassLoaders(); \
        for (auto _loader_: _loaders_) { \
            std::unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *_classes_; \
            \
            if (_loader_ == BOOT_CLASS_LOADER) { \
                _classes_ = getAllBootClasses(); \
            } else { \
                _classes_ = _loader_->classes; \
            } \
            assert(_classes_ != nullptr); \
            \
            for (auto &_p_: *_classes_) { \
                Class *clazz = _p_.second; \
                code_block \
            } \
        } \
    } while(false)

// 遍历已经加载的所有类
void traverseAllLoadedClasses(void (*_do)(Class *));

#endif // CABIN_CLASS_LOADER_H
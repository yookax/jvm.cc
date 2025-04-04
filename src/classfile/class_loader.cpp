module;
#include <cassert>
#include "../vmdef.h"
#include "../jni.h"
#include "../../lib/minizip/unzip.h"

module class_loader;

import std.core;
import std.filesystem;
import sysinfo;
import slot;
import primitive;
import object;
import classfile;
import jimage;
import dll;
import exception;
import interpreter;

using namespace std;
using namespace slot;
using namespace utf8;
using namespace std::filesystem;

// jdk modules 全路径
vector<string> jdk_modules;

/* jdk module names
 * /java.base/, /java.net/ ....
 */
vector<string> jdk_module_names;

static char classpath[PATH_MAX + 1] = { 0 };


#define IS_SLASH_CLASS_NAME(class_name) (strchr(class_name, '.') == NULL)
#define IS_DOT_CLASS_NAME(class_name) (strchr(class_name, '/') == NULL)

__declspec(dllexport) void set_classpath(const char *cp) {
    assert(cp != nullptr);
    strcpy(classpath, cp);
}

const char *get_classpath() {
    return classpath;
}

/*
 * 查找`path`目录下所有已`suffix`为后缀的文件，将文件名保存在`result`中
 */
void find_files_by_suffix(const char *path0, const char *suffix, vector<string> &result) {
    path curr_path(path0);
    if (!exists(curr_path)) {
        // todo error
        return;
    }

    directory_entry entry(curr_path);
    if (entry.status().type() != file_type::directory) {
        // todo error
        return;
    }

    directory_iterator files(curr_path);
    for (auto& f: files) {
        if (f.is_regular_file()) {
            char abspath[PATH_MAX + 1];
            // sprintf 和 snprintf 会自动在加上字符串结束符'\0'
            sprintf(abspath, "%s/%s", path0, f.path().filename().string().c_str()); // 绝对路径

            char *tmp = strrchr(abspath, '.');
            if (tmp != nullptr && strcmp(++tmp, suffix) == 0)
                result.emplace_back(abspath);
        }
    }
}

static void init_classpath() {
    assert(!g_java_home.empty()); // not empty

    find_files_by_suffix((g_java_home + "/jmods").c_str(), "jmod", jdk_modules);

    // 第0个位置放java.base.jmod，因为java.base.jmod常用，所以放第0个位置首先搜索。
    for (auto iter = jdk_modules.begin(); iter != jdk_modules.end(); iter++) {
        auto i = iter->rfind('\\');
        auto j = iter->rfind('/');
        if ((i != iter->npos && iter->compare(i + 1, 6, "java.base.jmod") == 0)
            || (j != iter->npos && iter->compare(j + 1, 6, "java.base.jmod") == 0)) {
            std::swap(*(jdk_modules.begin()), *iter);
            break;
        }
    }

    for (auto &s: jdk_modules) {
        size_t i = s.rfind('\\');
        size_t j = s.rfind('/');
        if (i == string::npos && j == string::npos) {
            panic(""); // todo
        }
        size_t x;
        if (i == string::npos) {
            x = j;
        } else if (j == string::npos) {
            x = i;
        } else {
            x = std::max(i, j) + 1;
        }
        size_t y = s.rfind(".jmod");
        if (y == string::npos || y <= x) {
            panic(""); // todo
        }
        jdk_module_names.emplace_back("/" + s.substr(x, y - x) + "/");
    }

    if (classpath[0] == 0) {
        // 没有在启动命令中指定CLASSPATH，尝试读取CLASSPATH环境变量
        const char *cp = getenv("CLASSPATH");
        if (cp == nullptr) {
            // 没有在启动命令中指定CLASSPATH，也没有设置CLASSPATH环境变量，那么就将当前路径设为CLASSPATH
            cp = get_current_working_directory();
        }
        strcpy(classpath, cp);
    }
}

enum ClassLocation {
    IN_JAR,
    IN_MODULE
};

#if 1

static void* (*zipOpen)(const char *name, char **msg);
static void* (*zipFindEntry)(void *zip, char *name, jint *entry_size, jint *name_len);
static jboolean (*zipReadEntry)(void *zip, void *entry, unsigned char *buf, char *entry_name);
static void (*zipClose)(void *zip);

/*
 * @param class_name: xxx/xxx/xxx
 */
static optional<pair<u1 *, size_t>> read_class(
                   const char *path, const char *class_name, ClassLocation location) {
    char *msg;
    // JAVA_HOME/bin/zip.dll 中已经对 zipOpen 做了Cache，
    // 所有这里无需 Cache，直接打开即可，效率OK。
    void *zip_file = zipOpen(path, &msg);

    if (zip_file == nullptr) {
        throw java_io_IOException(string("zipOpen failed: ") + path + ". " + msg);
        return nullopt;
    }

    auto buf = new char[strlen(class_name) + 32]; // big enough
    if (location == IN_JAR) {
        strcat(strcpy(buf, class_name), ".class");
    } else if (location == IN_MODULE) {
        // All classes 放在 module 的 "classes" 目录下
        strcat(strcat(strcpy(buf, "classes/"), class_name), ".class");
    }

    jint entry_size;
    jint name_len;
    void *entry = zipFindEntry(zip_file, buf, &entry_size, &name_len);
    if (entry == nullptr) { // not found
        zipClose(zip_file);
        delete[] buf;
        return nullopt;
    }

    auto bytecode = new u1[entry_size];
    auto entry_name  = new char[name_len + 1];
    jbool b = zipReadEntry(zip_file, entry, bytecode, entry_name);

    zipClose(zip_file);
    delete[] entry_name;
    delete[] buf;

    if (!b) {
        // todo error
        return nullopt;
    }
    return make_pair(bytecode, entry_size);
}

#endif

vector<pair<const char *, unzFile>> zfiles;

#if 0
/*
 * @param class_name: xxx/xxx/xxx
 */
static optional<pair<u1 *, size_t>> read_class(const char *path,
                                       const char *class_name, ClassLocation location) {
    unzFile zip_file = nullptr;
    for (auto &p : zfiles) {
        if (strcmp(p.first, path) == 0) {
            zip_file = p.second;
            break;
        }
    }
    if (zip_file == nullptr) {
        zip_file = unzOpen64(path);
        if (zip_file == nullptr) {
            throw java_io_IOException(string("unzOpen64 failed: ") + path);
        }
        zfiles.emplace_back(strdup(path), zip_file);
    }

    auto buf = new char[strlen(class_name) + 32]; // big enough
    if (location == IN_JAR) {
        strcat(strcpy(buf, class_name), ".class");
    } else if (location == IN_MODULE) {
        // All classes 放在 module 的 "classes" 目录下
        strcat(strcat(strcpy(buf, "classes/"), class_name), ".class");
    }

    if (unzLocateFile(zip_file, buf, 1) != UNZ_OK) {
        // not found
        // unzClose(zip_file);
        delete[] buf;
        return nullopt;
    }

    char file_name[PATH_MAX];
    unz_file_info64 file_info;
    if (unzGetCurrentFileInfo64(zip_file, &file_info, file_name, sizeof(file_name), nullptr, 0, nullptr, 0) != UNZ_OK) {
        // unzClose(zip_file);
        throw java_io_IOException(string("unzGetCurrentFileInfo64 failed: ") + path);
    }

    if (unzOpenCurrentFile(zip_file) != UNZ_OK) {
        // unzClose(zip_file);
        throw java_io_IOException(string("unzOpenCurrentFile failed: ") + path);
    }

    size_t uncompressed_size = file_info.uncompressed_size;
    auto bytecode = new u1[uncompressed_size];
    int size = unzReadCurrentFile(zip_file, bytecode, (unsigned int) uncompressed_size);
    // unzCloseCurrentFile(zip_file); // todo 干嘛的
    // unzClose(zip_file);
    delete[] buf;

    if (size != uncompressed_size)
        throw java_io_IOException(string("unzReadCurrentFile failed: ") + path);
    return make_pair(bytecode, uncompressed_size);
}

#endif

/*
 * Read JDK 类库中的类，不包括Array Class.
 * xxx/xxx/xxx
 */
static optional<pair<const u1 *, size_t>> read_boot_class(const utf8_t *class_name) {
    assert(class_name != nullptr);
//    assert(isSlashName(class_name));
    assert(class_name[0] != '['); // don't load array class

    // 从jimage中读取
    for (auto &s: jdk_module_names) {
        string path = s + class_name + ".class";
        auto content = get_resource_from_jimage((const char8_t *)path.c_str());
        if (content.has_value()) { // find out
            return content;
        }
    }

//    for (auto &mod : jdk_modules) {
//        auto content = read_class(mod.c_str(), class_name, IN_MODULE);
//        if (content.has_value()) { // find out
//            return content;
//        }
//    }

    return nullopt;
}

// -----------------------------------------------------------------------

static utf8_set boot_packages;
static unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> boot_classes; // todo 加读写锁

// vm中所有存在的 class loaders，include "boot class loader".
static unordered_set<const Object *> loaders;

static void addClassToClassLoader(Object *class_loader, Class *c) {
    assert(c != nullptr);
    if (class_loader == BOOT_CLASS_LOADER) {
        boot_classes.insert(make_pair(c->name, c));
        return;
    }


    loaders.insert(class_loader);
    if (class_loader->classes == nullptr) {
        class_loader->classes = new unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator>;
    }
    class_loader->classes->insert(make_pair(c->name, c));

    // Invoked by the VM to record every loaded class with this loader.
    // void addClass(Class<?> c);
//    Method *m = classLoader->clazz->getDeclaredInstMethod("addClass", "Ljava/lang/Class;");
//    assert(m != nullptr);
//    execJavaFunc(m, { (slot_t) classLoader, (slot_t) c });
}

Class *load_boot_class(const utf8_t *name) {
    assert(name != nullptr);
    assert(IS_SLASH_CLASS_NAME(name));
    assert(name[0] != '['); // don't load array class

    auto iter = boot_classes.find(name);
    if (iter != boot_classes.end()) {
        TRACE("find loaded class (%s) from pool.", name);
        return iter->second;
    }

    Class *c = nullptr;
    if (PRIMITIVE::check_class_name(name)) {
        c = new Class(name);
        addClassToClassLoader(BOOT_CLASS_LOADER, c);
    } else {
        auto content = read_boot_class(name);
        if (content.has_value()) { // find out
            c = define_class(BOOT_CLASS_LOADER, content->first, content->second);
        }
    }

    if (c != nullptr) {
        boot_packages.insert(c->pkg_name);
        // addClassToClassLoader(BOOT_CLASS_LOADER, c);
    }
    
    return c;
}

ArrayClass *load_array_class(Object *loader, const utf8_t *arr_class_name) {
    assert(arr_class_name != nullptr);
    assert(arr_class_name[0] == '['); // must be array class name

    const char *elt_class_name = arr_class_name_2_elt_class_name(arr_class_name);
    Class *c = loadClass(loader, elt_class_name);
    if (c == nullptr)
        return nullptr; // todo

    /* Array Class 用它的元素的类加载器加载 */

    Class *arr_class = find_loaded_class(c->loader, arr_class_name);
    if (arr_class != nullptr)
        return (ArrayClass *) arr_class; // find out
    
    arr_class = new ArrayClass(c->loader, arr_class_name);
    if (arr_class->loader == BOOT_CLASS_LOADER) {
        boot_packages.insert(arr_class->pkg_name); // todo array class 的pkg_name是啥
    }
    addClassToClassLoader(arr_class->loader, arr_class);
    return (ArrayClass *) arr_class;
}

ArrayClass *load_type_array_class(ArrayType type) {
    const char *arr_class_name = nullptr;

    switch (type) {
        case JVM_AT_BOOLEAN: arr_class_name = "[Z"; break;
        case JVM_AT_CHAR:    arr_class_name = "[C"; break;
        case JVM_AT_FLOAT:   arr_class_name = "[F"; break;
        case JVM_AT_DOUBLE:  arr_class_name = "[D"; break;
        case JVM_AT_BYTE:    arr_class_name = "[B"; break;
        case JVM_AT_SHORT:   arr_class_name = "[S"; break;
        case JVM_AT_INT:     arr_class_name = "[I"; break;
        case JVM_AT_LONG:    arr_class_name = "[J"; break;
        default:
            UNREACHABLE("Invalid array type: %d", type);
    }

    return load_array_class(BOOT_CLASS_LOADER, arr_class_name);
}

const utf8_t *get_boot_package(const utf8_t *name) {
    auto iter = boot_packages.find(name);
    return iter != boot_packages.end() ? *iter : nullptr;
}

utf8_set &get_boot_packages() {
    return boot_packages;
}

Class *find_loaded_class(Object *class_loader, const utf8_t *name) {
    assert(name != nullptr);
    assert(IS_SLASH_CLASS_NAME(name));

     if (class_loader == nullptr) {
        auto iter = boot_classes.find(name);
        return iter != boot_classes.end() ? iter->second : nullptr;
    }

    // is not boot classLoader
    if (class_loader->classes != nullptr) {
        auto iter = class_loader->classes->find(name);
        return iter != class_loader->classes->end() ? iter->second : nullptr;
    }

    // not find
    return nullptr;
}

Class *loadClass(Object *class_loader, const utf8_t *name) {
    assert(name != nullptr);
//    assert(isSlashName(name));

    utf8_t *slash_name = utf8::dot_2_slash_dup(name);
    Class *c = find_loaded_class(class_loader, slash_name);
    if (c != nullptr)
        return c;

    if (slash_name[0] == '[')
        return load_array_class(class_loader, slash_name);

    // 先尝试用boot class loader load the class
    c = load_boot_class(slash_name);
    if (c != nullptr || class_loader == nullptr)
        return c;

    // todo 再尝试用扩展classLoader load the class

    // public Class<?> loadClass(String name) throws ClassNotFoundException
    Method *m = class_loader->clazz->lookup_method("loadClass",
                                   "(Ljava/lang/String;)Ljava/lang/Class;");
    assert(m != nullptr && !m->access_flags.is_static());

    utf8_t *dot_name = slash_2_dot_dup(name);
    slot_t *slot = execJava(m, { rslot(class_loader), rslot(Allocator::string(dot_name)) });
    assert(slot != nullptr);
    jclsRef co = slot::get<jref>(slot);
    assert(co != nullptr && co->jvm_mirror != nullptr);
    c = co->jvm_mirror;
    addClassToClassLoader(class_loader, c);
    // init_class(c); /////////// todo /////////////////////////////////////////
    return c;
}

Class *define_class(jref class_loader, const u1 *bytecode, size_t len) {
    auto c = new Class(class_loader, bytecode, len);
    addClassToClassLoader(class_loader, c);
    return c;
}

Class *define_class(jref class_loader, const char *name, const jbyte *buf,
                   jsize len, jref pd, const char *source) {
    Class *c = define_class(class_loader, (const u1 *) buf, len);

    if (!utf8::equals(c->name, name)) {
        unimplemented; // todo
    }
    
    jref co = c->java_mirror;
    co->protection_domain = pd;

    if (source != nullptr) {
        c->source_file_name = strdup(source);
    }

    // addClassToClassLoader(class_loader, c);
    return c;
}

Class *define_class(Class *lookup, const char *name, const jbyte *buf,
                   jsize len, jref pd, jboolean init, int flags, jref class_data) {
    Class *c = define_class(lookup->loader, (const u1 *) buf, len);

    // if (init) {
    //     init_class(c); // todo
    // }

    if (name != nullptr && !utf8::equals(c->name, name)) {
        cout << c->name << endl;
        cout << name << endl;
        unimplemented; // todo
    }

    c->access_flags.set(flags);

    jref co = c->java_mirror;
    co->protection_domain = pd;

    if (class_data != nullptr) {
        // UNIMPLEMENTED; // todo
    }

    // todo `c`的 classloader 是不是lookup的classloader
    // addClassToClassLoader(lookup->loader, c);
    return c;
}

Class *define_class(jref class_loader, jstrRef name,
                    jarrRef bytecode, jint off, jint len, jref protection_domain) {
    unimplemented

//    u1 *data = (u1 *) bytecode->data;
//    Class *c = defineClass(class_loader, data + off, len);
//    // c->class_name和name是否相同 todo
//    //    printvm("class_name: %s\n", c->class_name);
//    return c;
}

Class *init_class(Class *c) {
    assert(c != nullptr);

    if (c->initialized()) {
        return c;
    }

    c->clinit_mutex.lock();
    if (c->initialized()) { // 需要再次判断 inited，有可能被其他线程置为 true
        c->clinit_mutex.unlock();
        return c;
    }

    c->state = Class::State::INITING;

    if (c->super_class != nullptr) {
        init_class(c->super_class);
    }

    // 在这里先行 set inited true, 如不这样，后面执行<clinit>时，
    // 可能调用putstatic等函数也会触发<clinit>的调用造成死循环。

    c->state = Class::State::INITED;

    Method *m = c->get_method("<clinit>", "()V");
    if (m != nullptr) { // 有的类没有<clinit>方法
        execJava(m);
    }

    c->state = Class::State::INITED;
    c->clinit_mutex.unlock();
    return c;
}

Class *link_class(Class *c) {
    assert(c != nullptr);

    // todo

    c->state = Class::State::LINKED;
    return c;
}

Object *get_platform_classloader() {
    Class *c = load_boot_class("java/lang/ClassLoader");

    // public static ClassLoader getPlatformClassLoader();
    Method *get = c->get_method("getPlatformClassLoader", "()Ljava/lang/ClassLoader;");
    if (get == nullptr || !get->access_flags.is_static()) {
        // todo error
    }
    return execJavaR(get);
}

Object *get_app_classloader() {
    Class *c = load_boot_class("java/lang/ClassLoader");

    // public static ClassLoader getSystemClassLoader();
    Method *get = c->get_method("getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    if (get == nullptr || !get->access_flags.is_static()) {
        // todo error
    }
    return execJavaR(get);
}

//Class *g_object_class = nullptr;
//Class *g_class_class = nullptr;
//Class *g_string_class = nullptr;

void init_classloader() {
    init_classpath();

    if (g_libzip == nullptr) {
        panic("g_libzip"); // todo
    }

    // from zip.dll 中读取我们需要的函数接口
    zipOpen = (decltype(zipOpen)) find_library_entry(g_libzip, "ZIP_Open");
    zipFindEntry = (decltype(zipFindEntry)) find_library_entry(g_libzip, "ZIP_FindEntry");
    zipReadEntry = (decltype(zipReadEntry)) find_library_entry(g_libzip, "ZIP_ReadEntry");
    zipClose = (decltype(zipClose)) find_library_entry(g_libzip, "ZIP_Close");

    assert(zipOpen != nullptr && zipFindEntry != nullptr );
    assert(zipReadEntry != nullptr && zipClose != nullptr );

    g_object_class = load_boot_class("java/lang/Object");
    g_class_class = load_boot_class("java/lang/Class");

    // g_class_class 至此创建完成。
    // 在 g_class_class 创建完成之前创建的 Class 都没有设置 java_mirror 字段，现在设置下。
    for (auto iter: boot_classes) {
        Class *c = iter.second;
        c->generate_class_object();
    }

    g_string_class = load_boot_class("java/lang/String");

    loaders.insert(BOOT_CLASS_LOADER);
}

unordered_map<const utf8_t *, Class *, utf8::Hash, utf8::Comparator> *getAllBootClasses() {
    return &boot_classes;
}

const unordered_set<const Object *> &getAllClassLoaders() {
    return loaders;
}

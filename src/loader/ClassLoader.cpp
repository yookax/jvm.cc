/*
 * Author: kayo
 */

#include "ClassLoader.h"
#include "minizip/unzip.h"
#include "../rtda/ma/Class.h"
#include "../rtda/ma/Field.h"
#include "../rtda/heap/ClassObject.h"
#include "../symbol.h"


using namespace std;

struct bytecode_content {
    u1 *bytecode;
    size_t len;
};

static struct bytecode_content invalid_bytecode_content = { NULL, 0 };

#define IS_INVALID(bytecode_content) ((bytecode_content).bytecode == NULL || (bytecode_content).len == 0)

static struct bytecode_content read_class_from_jar(const char *jar_path, const char *class_name)
{
    unz_global_info64 global_info;
    unz_file_info64 file_info;

    unzFile jar_file = unzOpen64(jar_path);
    if (jar_file == NULL) {
        // todo error
        printvm("unzOpen64 failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }
    if (unzGetGlobalInfo64(jar_file, &global_info) != UNZ_OK) {
        // todo throw “文件错误”;
        unzClose(jar_file);
        printvm("unzGetGlobalInfo64 failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }

    if (unzGoToFirstFile(jar_file) != UNZ_OK) {
        // todo throw “文件错误”;
        unzClose(jar_file);
        printvm("unzGoToFirstFile failed: %s\n", jar_path);
        return invalid_bytecode_content;
    }

    for (unsigned long long int i = 0; i < global_info.number_entry; i++) {
        char file_name[PATH_MAX];
        if (unzGetCurrentFileInfo64(jar_file, &file_info, file_name, sizeof(file_name), NULL, 0, NULL, 0) != UNZ_OK) {
            unzClose(jar_file);
            printvm("unzGetCurrentFileInfo64 failed: %s\n", jar_path);
            return invalid_bytecode_content;
        }

        char *p = strrchr(file_name, '.');
        if (p != NULL && strcmp(p, ".class") == 0) {
            *p = 0; // 去掉后缀

            if (strcmp(file_name, class_name) == 0) {
                // find out!
                if (unzOpenCurrentFile(jar_file) != UNZ_OK) {
                    // todo error
                    unzClose(jar_file);
                    printvm("unzOpenCurrentFile failed: %s\n", jar_path);
                    return invalid_bytecode_content;
                }

                size_t uncompressed_size = (size_t) file_info.uncompressed_size;
                u1 *bytecode = (u1 *) vm_malloc(sizeof(u1) * uncompressed_size);
                if (unzReadCurrentFile(jar_file, bytecode, (unsigned int) uncompressed_size) != uncompressed_size) {
                    // todo error
                    unzCloseCurrentFile(jar_file);  // todo 干嘛的
                    unzClose(jar_file);
                    printvm("unzReadCurrentFile failed: %s\n", jar_path);
                    return invalid_bytecode_content;
                }
                unzCloseCurrentFile(jar_file); // todo 干嘛的
                unzClose(jar_file);
                return (struct bytecode_content) { bytecode, uncompressed_size };
            }
        }

        int t = unzGoToNextFile(jar_file);
        if (t == UNZ_END_OF_LIST_OF_FILE) {
            break;
        }
        if (t != UNZ_OK) {
            // todo error
            unzClose(jar_file);
            printvm("unzGoToNextFile failed: %s\n", jar_path);
            return invalid_bytecode_content;
        }
    }

    unzClose(jar_file);
    return invalid_bytecode_content;
}

static struct bytecode_content read_class_from_dir(const char *dir_path, const char *class_name)
{
    assert(dir_path != nullptr);
    assert(class_name != nullptr);

    char file_path[PATH_MAX];
    sprintf(file_path, "%s/%s.class", dir_path, class_name);

    FILE *f = fopen(file_path, "rb");
    if (f != NULL) { // find out
        fseek(f, 0, SEEK_END); //定位到文件末
        size_t file_len = (size_t) ftell(f); //文件长度

        u1 *bytecode = (u1 *) vm_malloc(sizeof(u1) * file_len);
        fseek(f, 0, SEEK_SET);
        fread(bytecode, 1, file_len, f);
        fclose(f);
        return (struct bytecode_content) { bytecode, file_len };
    }

    if (errno != ENOFILE) {
        // file is exist, but open failed
        jvm_abort("%s, %s\n", file_path, strerror(errno)); // todo
    }

    // not find
    return invalid_bytecode_content;
}

static struct bytecode_content read_class(const char *class_name)
{
    // search jre/lib
    for (int i = 0; i < jre_lib_jars_count; i++) {
        struct bytecode_content content = read_class_from_jar(jre_lib_jars[i], class_name);
        if (!IS_INVALID(content)) // find out
            return content;
    }

    // search jre/lib/ext
    for (int i = 0; i < jre_ext_jars_count; i++) {
        struct bytecode_content content = read_class_from_jar(jre_ext_jars[i], class_name);
        if (!IS_INVALID(content)) // find out
            return content;
    }

    // search user paths
    for (int i = 0; i < user_dirs_count; i++) {
        struct bytecode_content content = read_class_from_dir(user_dirs[i], class_name);
        if (!IS_INVALID(content)) // find out
            return content;
    }

    // search user jars
    for (int i = 0; i < user_jars_count; i++) {
        struct bytecode_content content = read_class_from_jar(user_jars[i], class_name);
        if (!IS_INVALID(content)) // find out
            return content;
    }

    return invalid_bytecode_content; // not find
}

ClassLoader::ClassLoader(bool is_bootstrap_loader)
{
    if (is_bootstrap_loader) {
        g_bootstrap_loader = this;
    }

    /*
     * 先加载java.lang.Class类，
     * 这又会触发java.lang.Object等类和接口的加载。
     */
    jlClass = loadSysClass(S(java_lang_Class));

    // 加载基本类型（int, float, etc.）的 class
    loadPrimitiveTypes();

    // todo
    //    for (auto &primitiveType : primitiveTypes) {
//        loadPrimitiveClasses(primitiveType.name);
//        loadClass(primitiveType.arrayClassName);
//        loadClass(primitiveType.wrapperClassName);
//    }

    // 给已经加载的每一个类关联类对象。
    for (auto iter : loadedClasses) {
        iter.second->clsobj = ClassObject::newInst(iter.second); // todo ClassObject可不可以重用，每次都要new吗？？
    }

    // 缓存一下常见类
    jlString = loadSysClass(S(java_lang_String));
    jlClassArr = loadArrayClass(S(array_java_lang_Class));
    jlObjectArr = loadArrayClass(S(array_java_lang_Object));
    charArr = loadArrayClass(S(array_C));
}

void ClassLoader::putToPool(const char *className, Class *c)
{
    assert(className != nullptr);
    assert(strlen(className) > 0);
    assert(c != nullptr);
    loadedClasses.insert(make_pair(className, c));
}

Class *ClassLoader::loading(const char *className)
{
    struct bytecode_content content = read_class(className);
    if (IS_INVALID(content)) {
        jvm_abort("class not find: %s", className);
    }

    return new Class(this, content.bytecode, content.len);
}

static Class* verification(Class *c)
{
    if (c->magic != 0xcafebabe) {
        jvm_abort("error. magic = %u(0x%x)", c->magic, c->magic);
    }

    // todo 验证版本号
    /*
     * Class版本号和Java版本对应关系
     * JDK 1.8 = 52
     * JDK 1.7 = 51
     * JDK 1.6 = 50
     * JDK 1.5 = 49
     * JDK 1.4 = 48
     * JDK 1.3 = 47
     * JDK 1.2 = 46
     * JDK 1.1 = 45
     */
    if (c->major_version != 52) {
        jvm_abort("only support jdk 8"); // todo class version
    }

    return c;
}

static Class* preparation(Class *c)
{
//    const struct rtcp* const rtcp = c->rtcp;

    // 如果静态变量属于基本类型或String类型，有final修饰符，
    // 且它的值在编译期已知，则该值存储在class文件常量池中。
    for (size_t i = 0; i < c->fields.size(); i++) {
        if (!c->fields[i]->isStatic()) {
            continue;
        }

//        const int index = c->fields[i]->constant_value_index;

        // 值已经在常量池中了
//        const bool b = (index != INVALID_CONSTANT_VALUE_INDEX);
// todo
//        c->setFieldValue(Field.id, Field.descriptor,
//        [&]() -> jint { return b ? rtcp->getInt(index) : 0; },  // todo byte short 等都是用 int 表示的吗
//        [&]() -> jfloat { return b ? rtcp->getFloat(index) : 0; },
//        [&]() -> jlong { return b ? rtcp->getLong(index) : 0; },
//        [&]() -> jdouble { return b ? rtcp->getDouble(index) : 0; },
//        [&]() -> jref {
//            if (field.descriptor == "Ljava/lang/String;" and b) {
//                // todo
//                const string &str = rtcp->getStr(index);
//                return JStringObj::newJStringObj(class->loader, strToJstr(str));
//            }
//            return nullptr;
//        });
    }

    return c;
}

/*
 * 解析（Resolution）是根据运行时常量池的符号引用来动态决定具体的值的过程。
 */
static Class* resolution(Class *c)
{
    // todo
    return c;
}

static Class* initialization(Class *c)
{
    // todo
    return c;
}

Class *ClassLoader::loadNonArrClass(const char *class_name)
{
// todo 解析，初始化是在这里进行，还是待使用的时候再进行
    Class *c = loading(class_name);
    return initialization(resolution(preparation(verification(c))));
}

Class *ClassLoader::loadClass(const char *class_name)
{
    assert(class_name != nullptr);

    auto iter = loadedClasses.find(class_name);
    if (iter != loadedClasses.end()) {
        return iter->second;
    }

    Class *c = nullptr;
    if (class_name[0] == '[') {
        c = new ArrayClass(class_name);
    } else {
        c = loadNonArrClass(class_name);
    }

    if (c == nullptr) {
        VM_UNKNOWN_ERROR("loader class failed. %s", class_name);
        return nullptr;
    }

    if (jlClass != nullptr) {
        c->clsobj = ClassObject::newInst(c);//clsobj_create(c);
    }

    loadedClasses.insert(make_pair(c->className, c));
    return c;
}

ClassLoader::~ClassLoader()
{

}

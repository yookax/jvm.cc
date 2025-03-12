#include "cabin.h"
#include "runtime/heap.h"
#include "jni.h"

import std.core;
import std.filesystem;

using namespace std;

static char main_class_name[FILENAME_MAX] = { 0 };
static char *main_func_args[MAX_JVM_ARITY];
static int main_func_args_count = 0;

void set_classpath(const char *cp);

static void parse_command_line(int argc, char *argv[]) {
    // 可执行程序的名字为 argv[0]
    // const char *vm_name = argv[0];

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            const char *name = argv[i];
            if (strcmp(name, "-cp") == 0 || strcmp(name, "-classpath") == 0) { // parse Class Path
                if (++i >= argc) {
                    printf("Error: %s requires class path specification\n", name);
                    exit(EXIT_CODE_UNKNOWN_ERROR);
                }
                set_classpath(argv[i]);
            } else {
                printf("Unrecognised option: %s\n", argv[i]);
                printf("Error: Could not create the Java Virtual Machine.\n");
                printf("Error: A fatal exception has occurred. Program will exit.\n");
                exit(EXIT_CODE_UNKNOWN_ERROR);
            }
        } else {
            if (main_class_name[0] == 0) {
                strcpy(main_class_name, argv[i]);
            } else {
                // main function's arguments
                main_func_args[main_func_args_count++] = argv[i];
                if (main_func_args_count > MAX_JVM_ARITY) {
                    panic(" "); // todo many args!!! abort!
                }
            }
        }
    }
}

int run_jvm(int argc, char* argv[], bool is_testing = false) {
    auto start = chrono::high_resolution_clock::now();

    parse_command_line(argc, argv);

    if (main_class_name[0] == 0) {  // empty  todo
        panic("no input file\n");
    }

    for(auto t = main_class_name; *t; t++) {
        if (*t == '.')
            *t = '/';
    }

    JavaVM *vm;
    JNIEnv *env;
    JavaVMInitArgs vm_init_args;
    JNI_CreateJavaVM(&vm, &env, &vm_init_args);

    auto main_class = env->FindClass(main_class_name);
//    Class *main_class = loadClass(g_app_class_loader, utf8::dot_2_slash(main_class_name));
    if (main_class == nullptr) {
        panic("main_class == NULL"); // todo
    }

  //  init_class(main_class);

    auto main_method = env->GetStaticMethodID(main_class, "main", "([Ljava/lang/String;)V");
//    Method *main_method = main_class->lookup_method("main", "([Ljava/lang/String;)V");
    if (main_method == nullptr) {
        if (is_testing) {
            // 是测试代码调用的，没有main函数直接返回即可。
            return 0;
        }
        panic("can't find method main.");
    }

    // 开始在主线程中执行 main 方法
    TRACE("begin to execute main function.\n");

    // Create the String array holding the command line args
//    jarrRef args = Allocator::string_array(main_func_args_count);
//    for (int i = 0; i < main_func_args_count; i++) {
//        args->setRefElt(i, Allocator::string(main_func_args[i]));
//    }
//
//    // Call the main method
//    execJava(main_method, { slot::rslot(args) });

    // Create the String array holding the command line args
    auto string_class = env->FindClass("java/lang/String");
    auto args = env->NewObjectArray(main_func_args_count, string_class, nullptr);
    for (int i = 0; i < main_func_args_count; i++) {
        auto a = env->NewStringUTF(main_func_args[i]);
        env->SetObjectArrayElement(args, i, a);
    }

    jvalue v;
    v.l = (jobject) args;
    // Call the main method
    env->CallStaticVoidMethodA(main_class, main_method, &v);

    // todo 如果有其他的非后台线程在执行，则main线程需要在此wait
    // todo main_thread 退出，做一些清理工作。

    /* Finally, print the running time of JVM and the JDK version. */

//    string jdk_version = g_java_home;
//    size_t last_slash = g_java_home.find_last_of("/\\");
//    if (last_slash != std::string::npos) {
//        jdk_version = g_java_home.substr(last_slash + 1);
//    }
//
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    printf("\n--- Powered by jvmcc(%.2fs) ---\n", duration / 1000.0);
//    printf("\n--- Powered by jvmcc on %s(%.2fs) ---\n", jdk_version.c_str(), duration / 1000.0);

    return 0;
}

#define RUN_GVM true

int main(int argc, char* argv[]) {
#if RUN_GVM
    return run_jvm(argc, argv);
#else
    void run_all_tests();
    run_all_tests();
#endif
}

// ----------------------------------- tests ---------------------------------------------

namespace fs = std::filesystem;

#define RUN_TEST_CASE(func_name) void func_name(); func_name();

void run_all_tests() {
#if 1
    JNI_CreateJavaVM(nullptr, nullptr, nullptr);

    RUN_TEST_CASE(test_sys_info);

    RUN_TEST_CASE(test_convert_int);
    RUN_TEST_CASE(test_convert_long);
    RUN_TEST_CASE(test_convert_float);
    RUN_TEST_CASE(test_convert_double);

    RUN_TEST_CASE(test_slot);

    RUN_TEST_CASE(test_properties);

    RUN_TEST_CASE(test_method_descriptor);

    RUN_TEST_CASE(test_alloc_continuously);
    RUN_TEST_CASE(test_heap1);

    RUN_TEST_CASE(test_load_class)
    RUN_TEST_CASE(test_classloader1);

    RUN_TEST_CASE(test_box);

    RUN_TEST_CASE(test_string1);
    RUN_TEST_CASE(test_intern);
    RUN_TEST_CASE(test_equals);

    RUN_TEST_CASE(test_new_array);
    RUN_TEST_CASE(test_multi_array1);
    RUN_TEST_CASE(test_multi_array2);
    RUN_TEST_CASE(test_string_array);

    RUN_TEST_CASE(test_inject_field);
#endif

//    JavaVM *vm;
//    JNIEnv *env;
//    JavaVMInitArgs vm_init_args;
//    JNI_CreateJavaVM(&vm, &env, &vm_init_args);

    /* Next, test the classes under the specified directory. */

    vector<string> class_files;
    auto class_path = R"(D:\Code\jvm.cc\test\target\classes)"; // Modify according to the requirements.
    fs::path p = class_path;
    if (fs::exists(p) && fs::is_directory(p)) {
        for (const auto& entry : fs::recursive_directory_iterator(p)) {
            if (entry.is_regular_file() && entry.path().extension() == ".class") {
                // 计算相对路径
                fs::path relative_path = fs::relative(entry.path(), p);
                string path_str = relative_path.string();

                // 去掉 .class 后缀
                path_str = path_str.substr(0, path_str.length() - 6);

                // 将路径分隔符替换为 .
                std::replace(path_str.begin(), path_str.end(), '\\', '.');
                std::replace(path_str.begin(), path_str.end(), '/', '.');

                class_files.emplace_back(path_str);
            }
        }
    }
    for (const auto& cf: class_files) {
        std::cout << cf << std::endl;
        const char *argv[4];
        argv[0] = "jvmcc";
        argv[1] = "-cp";
        argv[2] = class_path;
        argv[3] = cf.c_str();
        run_jvm(4, const_cast<char **>(argv), true);
    }

    cout << endl << "Testing is over." << endl;
}

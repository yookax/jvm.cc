#include "vmdef.h"
#include "jni.h"

import std.core;
import std.filesystem;

using namespace std;

static char main_class_name[FILENAME_MAX] = { 0 };
static char *main_func_args[MAX_JVM_ARITY];
static int main_func_args_count = 0;

void set_classpath(const char *cp);

static bool silent_when_no_main = false;

static void parse_command_line(int argc, char *argv[]) {
    // 可执行程序的名字为 argv[0]
    // const char *vm_name = argv[0];

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            const char *name = argv[i];
            if (strcmp(name, "-cp") == 0 || strcmp(name, "-classpath") == 0) {
                if (++i >= argc) {
                    printf("Error: %s requires class path specification\n", name);
                    exit(EXIT_CODE_UNKNOWN_ERROR);
                }
                set_classpath(argv[i]);
            } else if (strcmp(name, "-silent-when-no-main") == 00) {
                silent_when_no_main = true;
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

int run_jvm(int argc, char* argv[]) {
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

    auto main_class = (*env)->FindClass(env, main_class_name);
//    Class *main_class = loadClass(g_app_class_loader, utf8::dot_2_slash(main_class_name));
    if (main_class == nullptr) {
        panic("main_class == NULL"); // todo
    }

  //  init_class(main_class);

    auto main_method = (*env)->GetStaticMethodID(env, main_class, "main", "([Ljava/lang/String;)V");
//    Method *main_method = main_class->lookup_method("main", "([Ljava/lang/String;)V");
    if (main_method == nullptr) {
        if (silent_when_no_main) {
            // 是测试代码调用的，没有main函数直接返回即可。
            return -1;
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
    auto string_class = (*env)->FindClass(env, "java/lang/String");
    auto args = (*env)->NewObjectArray(env, main_func_args_count, string_class, nullptr);
    for (int i = 0; i < main_func_args_count; i++) {
        auto a = (*env)->NewStringUTF(env, main_func_args[i]);
        (*env)->SetObjectArrayElement(env, args, i, a);
    }

    jvalue v;
    v.l = (jobject) args;
    // Call the main method
    (*env)->CallStaticVoidMethodA(env, main_class, main_method, &v);

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

int main(int argc, char* argv[]) {
    return run_jvm(argc, argv);
}
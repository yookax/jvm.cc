#include <chrono>

#include "cabin.h"
#include "jni.h"
#include "classfile/method.h"
#include "object/object.h"
#include "object/allocator.h"
#include "interpreter.h"

using namespace std;

static char main_class_name[FILENAME_MAX] = { 0 };
static char *main_func_args[MAX_JVM_ARITY];
static int main_func_args_count = 0;

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

int main(int argc, char* argv[]) {
    auto start = chrono::high_resolution_clock::now();
    
    parse_command_line(argc, argv);

    if (main_class_name[0] == 0) {  // empty  todo
        panic("no input file\n");
    }

    JavaVM *vm; 
    JNIEnv *env;
    JavaVMInitArgs vm_init_args;
    JNI_CreateJavaVM(&vm, &env, &vm_init_args);

    Class *main_class = loadClass(g_app_class_loader, utf8::dot_2_slash(main_class_name));
    if (main_class == nullptr) {
        panic("main_class == NULL"); // todo
    }

    init_class(main_class);

    Method *main_method = main_class->lookup_method("main", "([Ljava/lang/String;)V");
    if (main_method == nullptr) {
        panic("can't find method main.");
    } else {
        if (!main_method->isPublic()) {
            panic("method main must be public.");
        }
        if (!main_method->isStatic()) {
            panic("method main must be static.");
        }
    }

    // 开始在主线程中执行 main 方法
    TRACE("begin to execute main function.\n");

    // Create the String array holding the command line args
    jarrRef args = Allocator::string_array(main_func_args_count);
    for (int i = 0; i < main_func_args_count; i++) {
        args->setRefElt(i, Allocator::string(main_func_args[i]));
    }

    // Call the main method
    execJava(main_method, { slot::rslot(args) });

    // todo 如果有其他的非后台线程在执行，则main线程需要在此wait
    // todo main_thread 退出，做一些清理工作。

    /* Finally, print the running time of JVM and the JDK version. */

    string jdk_version = g_java_home;
    size_t last_slash = g_java_home.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        jdk_version = g_java_home.substr(last_slash + 1);
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    printf("\n--- Powered by jvmcc on %s(%.2fs) ---\n", jdk_version.c_str(), duration / 1000.0);

    return 0;
}

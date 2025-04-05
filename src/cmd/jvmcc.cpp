#include "../vmdef.h"

import std.core;
import std.filesystem;
import slot;
import encoding;
import classfile;
import object;
import class_loader;
import interpreter;

using namespace std;

static char main_class_name[FILENAME_MAX] = { 0 };
static char *main_func_args[MAX_JVM_ARITY];
static int main_func_args_count = 0;

void set_classpath(const char *cp);
string get_java_version();

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
                    panic("too many args!");
                }
            }
        }
    }
}

void init_jvm(InitArgs *init_args);

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

    init_jvm(nullptr);

    Class *main_class = loadClass(g_app_class_loader, utf8::dot_2_slash(main_class_name));
    if (main_class == nullptr) {
        panic("main_class == NULL"); // todo
    }

  //  init_class(main_class);

    Method *main_method = main_class->lookup_method("main", "([Ljava/lang/String;)V");
    if (main_method == nullptr) {
        if (silent_when_no_main) {
            // 是测试代码调用的，没有main函数直接退出即可。
            exit(-10);
        }
        panic("can't find method main.");
    }

    // 开始在主线程中执行 main 方法
    TRACE("begin to execute main function.\n");

    // Create the String array holding the command line args
    auto args = Allocator::string_array(main_func_args_count);
    for (int i = 0; i < main_func_args_count; i++) {
        auto a = Allocator::string(main_func_args[i]);
        args->setRefElt(i, a);
    }

    slot_t rs = slot::rslot(args);
    execJava(main_method, &rs);

    // todo 如果有其他的非后台线程在执行，则main线程需要在此wait
    // todo main_thread 退出，做一些清理工作。

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    printf("\n--- Powered by jvm.cc(%.2fs) on Java-%s ---\n", duration / 1000.0, get_java_version().c_str());
    return 0;
}

int main(int argc, char* argv[]) {
    return run_jvm(argc, argv);
}
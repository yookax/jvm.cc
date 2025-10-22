#include "vmdef.h"
#ifdef _WIN64
#include <Windows.h>
#endif

import std;
// import std.filesystem;
// import std.threading;
import slot;
import heap;
import convert;
import encoding;
import sysinfo;
import properties;
import classfile;
import object;
import class_loader;
import interpreter;
import jimage;
import raw_classfile;

using namespace std;

static char main_class_name[FILENAME_MAX] = { 0 };
static char *main_func_args[MAX_JVM_ARITY];
static int main_func_args_count = 0;

void set_classpath(const char *cp);
string get_java_version();

static bool silent_when_no_main = false;
static bool run_test = false;

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
            } else if (strcmp(name, "-silent-when-no-main") == 0) {
                silent_when_no_main = true;
            } else if (strcmp(name, "-test") == 0) {
                run_test = true;
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

void init_jvm();

int run_jvm(int argc, char* argv[]) {
    if (main_class_name[0] == 0) {  // empty  todo
        panic("no input file\n");
    }

    for(auto t = main_class_name; *t; t++) {
        if (*t == '.')
            *t = '/';
    }

    init_jvm();

    Class *main_class = load_class(g_app_class_loader, utf8::dot_2_slash(main_class_name));
    if (main_class == nullptr) {
        panic("main_class == NULL"); // todo
    }

    init_class(main_class);

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
    return 0;
}

void test();

int main(int argc, char* argv[]) {
#ifdef _WIN64
   SetConsoleOutputCP(CP_UTF8);
#endif

    auto start = chrono::high_resolution_clock::now();

    parse_command_line(argc, argv);

    int ret = 0;
    if (run_test) {
        test();
    } else {
        ret = run_jvm(argc, argv);
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    printf("\n--- Powered by jvm.cc(%.2fs) on Java-%s ---\n", duration / 1000.0, get_java_version().c_str());
    return ret;
}
#if 0
void run_all_java_tests();

void test() {
    init_jvm();

    // test_sys_info();
    // test_properties();
    //
    // test_convert_int();
    // test_convert_long();
    // test_convert_float();
    // test_convert_double();

    // test_utf8_to_latin1();
    // test_utf8_to_utf16();
    // test_utf16_to_utf8();

    // test_jimage_string();
    // test_jimage();
    // test_jimage1();

    // test_alloc_continuously();
    // test_heap();

    // test_load_class();
    // test_classloader();

    // test_method_descriptor();

    // test_box();

    // test_string();
    // test_string_intern();
    // test_string_equals();

    // test_new_array();
    // test_multi_array1();
    // test_multi_array2();
    // test_string_array();

    // test_inject_field();

    run_all_java_tests();

    cout << endl << "Testing is over." << endl;
}

namespace fs = std::filesystem;

auto jvmcc_path = "./";
auto class_path = "D:/Code/jvm.cc/test-java/target/classes"; // Modify according to the requirements.

static void run_all_java_tests() {
    printf("-----------------------------\n");
    string jvmcc;

    // 首先查找 jvmcc
    fs::path p = jvmcc_path;
    if (fs::exists(p) && fs::is_directory(p)) {
        for (const auto& entry : fs::directory_iterator(p)) {
            if (entry.is_regular_file() && entry.path().stem() == "jvmcc") {
                jvmcc = absolute(entry.path()).string();
                break;
            }
        }
    }

    if (jvmcc.empty()) {
        cout << "没有找到jvmcc" << endl;
        return;
    }

    vector<string> class_files;
    p = class_path;
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

    jvmcc.append(" -silent-when-no-main -cp ").append(class_path).append(" ");

    for (const auto& cf: class_files) {
        if (cf.contains('$')) {
            // inner class, do not test
            continue;
        }
        string s = jvmcc + cf;
        printf("-------------------- %s --------------------\n", cf.c_str());
        std::system(s.c_str());
    }
}
#endif
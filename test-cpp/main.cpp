#include "../src/vmdef.h"
#include "../src/jni.h"

import std.core;
import std.filesystem;

using namespace std;

void run_all_java_tests();
#define RUN_TEST_CASE(func_name) void func_name(); func_name();

int main() {
    RUN_TEST_CASE(test_sys_info)
    RUN_TEST_CASE(test_slot);

    RUN_TEST_CASE(test_convert_int);
    RUN_TEST_CASE(test_convert_long);
    RUN_TEST_CASE(test_convert_float);
    RUN_TEST_CASE(test_convert_double);

    JNI_CreateJavaVM(nullptr, nullptr, nullptr);

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

    run_all_java_tests();

    cout << endl << "Testing is over." << endl;
    return 0;
}

namespace fs = std::filesystem;

auto jvmcc_path = "./";
auto class_path = "D:/Code/jvm.cc/test-java/target/classes"; // Modify according to the requirements.

void run_all_java_tests() {
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

    string exclude[] = {
            "field.FieldAccessTest",
            "method.ArgsPassTest",
            "network.InetAddressTest",
            "network.SocketConnectTest",
            "network.SocketListenTest",
            "network.UrlTest",
            "reflect.ArrayGetTest",
            "reflect.ArrayLengthTest",
            "reflect.ArraySetTest",
            "reflect.GenericTest",
            "thread.AliveTest",
            "thread.DaemonTest",
            "thread.DumpAllThreads",
            "thread.InterruptFlagTest",
            "thread.InterruptionTest",
            "thread.MainThreadTest",
            "thread.RunnableTest",
            "thread.SleepTest",
            "thread.SynchronizedTest",
            "thread.ThreadSubClassTest",
            "thread.WaitTest",
            "ReflectTest",
    };

    jvmcc.append(" -silent-when-no-main -cp ").append(class_path).append(" ");

    for (const auto& cf: class_files) {
        if (cf.contains('$')) {
            // inner class, do not test
            continue;
        }

        bool b = false;
        for (string &s: exclude) {
            if (s == cf) {
                b = true;
                break;
            }
        }
        if (b)
            continue;

        string s = jvmcc + cf;
        printf("jvmcc %s\n", cf.c_str());
        std::system(s.c_str());
    }
}
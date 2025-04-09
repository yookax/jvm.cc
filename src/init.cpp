#include <cstdio>
#include <cstdlib>
#include "vmdef.h"

import std.core;
import std.filesystem;
import slot;
import sysinfo;
import properties;
import runtime;
import object;
import heap;
import classfile;
import invoke;
import class_loader;
import jimage;
import dll;
import native;
import interpreter;

using namespace std;

Heap *g_heap;

Object *g_sys_thread_group;

string g_java_home;
string g_java_version;

string get_java_version() {
    return g_java_version;
}

u2 g_classfile_major_version = 0;
u2 g_classfile_manor_version = 0;

Object *g_app_class_loader;
Object *g_platform_class_loader;

bool g_vm_initing = true;

static void init_heap() {
    g_heap = new Heap(VM_HEAP_SIZE);
    if (g_heap == nullptr) {
        panic("init Heap failed"); // todo
    }
}

namespace fs = std::filesystem;

static bool check_jdk_version(string jdk_path) {
    std::ifstream file(jdk_path + "/release", std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取文件内容到缓冲区
    auto buffer = new char[size];
    file.read(buffer, size);
    file.close();
    auto b = strstr(buffer, "JAVA_VERSION=\"" JAVA_COMPAT_MAJOR_VERSION);
    if (b == nullptr) {
        delete[] buffer;
        return false;
    }
    b += strlen("JAVA_VERSION=\"");
    auto c = strstr(b, "\"");
    if (c == nullptr) {
        delete[] buffer;
        return false;
    }
    *c = 0;
    g_java_version = b;
    delete[] buffer;
    return true;
}

static string find_jdk_dir() {
    // 查找"JAVA_HOME"环境变量
//    if (char buffer[PATH_MAX]; GetEnvironmentVariable("JAVA_HOME", buffer, PATH_MAX) > 0) {
//        if (check_jdk_version(buffer))
//            return std::string(buffer);
//    }
    const char *home = std::getenv("JAVA_HOME");
    if (home != nullptr) {
        if (check_jdk_version(home))
            return std::string(home);
    }

    // 没有设置"JAVA_HOME"环境变量，那么尝试在"PATH"环境变量中查找 java.exe
    // char pathBuffer[MAX_PATH];
    // if (GetEnvironmentVariable("PATH", pathBuffer, MAX_PATH) > 0) {
    // 	std::string pathStr(pathBuffer);
    // 	std::istringstream iss(pathStr);
    // 	std::string token;
    // 	while (std::getline(iss, token, ';')) {
    // 		std::string javaPath = token + "\\java.exe";
    // 		if (GetFileAttributes(javaPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
    // 			return token;
    // 		}
    // 	}
    // }

    // 尝试在以下几个目录查找
    vector<string> common_paths = {
            "C:\\Program Files\\Java",
            "C:\\ProgramData\\Java",
            "C:\\Java",
            "D:\\Java",
            "C:\\",
            "D:\\",
    };

    vector<pair<string, string>> jdk_dirs;

    for (const auto& p : common_paths) {
        if (!filesystem::exists(p))
            continue;

        for (const auto& entry : filesystem::directory_iterator(p)) {
            if (!entry.is_directory())
                continue;
            if (entry.path().filename().string().find("jdk") != string::npos) {
                // There should be a "bin" directory in a JDK directory,
                // and there should be executable files such as "javac" and "java"
                // in the "bin" directory.
                fs::path bin_path = entry.path() / "bin";
                if (fs::exists(bin_path) && fs::is_directory(bin_path)) {
                    if (fs::exists(bin_path / "javac.exe") || exists(bin_path / "javac")) {
                        string jdk_path = entry.path().string();
                        if (check_jdk_version(jdk_path))
                            return jdk_path;
                        //jdk_dirs.emplace_back(entry.path().filename().string(), entry.path().string());
                    }
                }
            }
        }
    }

    return "";
}

void __init_invoke__();

void init_jvm() {
    g_java_home = find_jdk_dir();
    if (g_java_home.empty()) {
        panic("java.lang.InternalError, %s\n", "no java lib");
    }

    /* order is important */
    init_heap();
    init_properties();
    init_jimage();
    init_dll();
    init_classloader();
    init_native();
    init_thread();
    init_invoke();
    __init_invoke__();
    init_polymorphic_method();
    init_module();
    init_reflection();

    // --------------------------------------

    // 设置 jdk/internal/misc/UnsafeConstants 的下列值
    // static final int ADDRESS_SIZE0;
    // static final int PAGE_SIZE;
    // static final boolean BIG_ENDIAN;
    // static final boolean UNALIGNED_ACCESS;
    // static final int DATA_CACHE_LINE_FLUSH_SIZE;
    Class *uc = load_boot_class("jdk/internal/misc/UnsafeConstants");
    init_class(uc);

    uc->lookup_field("ADDRESS_SIZE0", "I")->static_value.i = sizeof(void *);
    uc->lookup_field("PAGE_SIZE", "I")->static_value.i = page_size();
    uc->lookup_field("BIG_ENDIAN", "Z")->static_value.z = std::endian::native == std::endian::big;
    // todo UNALIGNED_ACCESS
    // todo DATA_CACHE_LINE_FLUSH_SIZE

    // --------------------------------------

    Class *ref = load_boot_class("java/lang/ref/Reference");
    init_class(ref);

    // todo
    // AccessibleObject.java中说AccessibleObject类会在initPhase1阶段初始化，
    // 但我没有在initPhase1中找到初始化AccessibleObject的代码
    // 所以`暂时`先在这里初始化一下。待日后研究清楚了再说。
    Class *acc = load_boot_class("java/lang/reflect/AccessibleObject");
    init_class(acc);

    Class *sys = load_boot_class("java/lang/System");
    init_class(sys);
    
    Method *m = sys->lookup_method("initPhase1", "()V");
    assert(m != nullptr);
    execJava(m);

    // private static int initPhase2(boolean printToStderr, boolean printStackTrace);
    // init module system
    m = sys->lookup_method("initPhase2", "(ZZ)I");
    assert(m != nullptr);
    auto v = slot::get<jint>(execJava(m, {slot::islot(1), slot::islot(1)}));
    if (v != 0) { // 等于0表示成功
        ERR("Init VM failed.");
    }

    m = sys->lookup_method("initPhase3", "()V");
    assert(m != nullptr);
    execJava(m);

    // --------------------------------------

    g_platform_class_loader = get_platform_classloader();
    assert(g_platform_class_loader != nullptr);
    
    g_app_class_loader = get_app_classloader();
    assert(g_app_class_loader != nullptr);

    // Main Thread Set ContextClassLoader
    g_main_thread->java_thread->set_field_value<jref>("contextClassLoader",
                                                      "Ljava/lang/ClassLoader;", g_app_class_loader);
    g_vm_initing = false;
    TRACE("init jvm is over.\n");
}
#include <cstdio>
#include <cstdlib>
#include "vmdef.h"
#include "jni.h"

import std.core;
import std.filesystem;
import slot;
import sysinfo;
import runtime;
import object;
import heap;
import classfile;
import invoke;
import class_loader;
import reference;
import dll;
import interpreter;

using namespace std;

Heap *g_heap;

Object *g_sys_thread_group;

string g_java_home;

u2 g_classfile_major_version = 0;
u2 g_classfile_manor_version = 0;

Object *g_app_class_loader;
Object *g_platform_class_loader;

bool g_vm_initing = true;

void init_native();

vector<Property> g_properties;

/*
 * System properties. The following properties are guaranteed to be defined:
 * java.version         Java version number
 * java.vendor          Java vendor specific string
 * java.vendor.url      Java vendor URL
 * java.home            Java installation directory
 * java.class.version   Java class version number
 * java.class.path      Java classpath
 * os.name              Operating System Name
 * os.arch              Operating System Architecture
 * os.version           Operating System Version
 * file.separator       File separator ("/" on Unix)
 * path.separator       Path separator (":" on Unix)
 * line.separator       Line separator ("\n" on Unix)
 * user.name            User account name
 * user.home            User home directory
 * user.dir             User's current working directory
 */
static void init_properties() {
    g_properties.emplace_back("java.vm.name", "CabinVM");
    g_properties.emplace_back("java.vm.version", VM_VERSION); // todo
    g_properties.emplace_back("java.version", VM_VERSION);
    g_properties.emplace_back("java.vendor", "Ka Yo");
    g_properties.emplace_back("java.vendor.url", "doesn't have");
    g_properties.emplace_back("java.home", g_java_home.c_str());
    ostringstream oss;
    oss << JVM_MAX_CLASSFILE_MAJOR_VERSION << "." << JVM_MAX_CLASSFILE_MINOR_VERSION << ends;
    g_properties.emplace_back("java.class.version", oss.str().c_str());
    g_properties.emplace_back("java.class.path", get_classpath());
    g_properties.emplace_back("os.name", os_name());
    g_properties.emplace_back("os.arch", os_arch());
    g_properties.emplace_back("os.version",  ""); // todo
    g_properties.emplace_back("file.separator", file_separator());
    g_properties.emplace_back("path.separator", path_separator());
    g_properties.emplace_back("line.separator", line_separator()); // System.out.println最后输出换行符就会用到这个
    char *p = getenv("USER");
    g_properties.emplace_back("user.name", p != nullptr ? p : "");// todo
    p = getenv("HOME");
    g_properties.emplace_back("user.home", p != nullptr ? p : "");// todo
    char *cwd = get_current_working_directory();
    g_properties.emplace_back("user.dir", cwd);
    g_properties.emplace_back("user.country", "CN"); // todo
    g_properties.emplace_back("file.encoding", "UTF-8");// todo
    g_properties.emplace_back("sun.stdout.encoding", "UTF-8");// todo
    g_properties.emplace_back("sun.stderr.encoding", "UTF-8");// todo
    g_properties.emplace_back("java.io.tmpdir", "");// todo
    p = getenv("LD_LIBRARY_PATH");
    g_properties.emplace_back("java.library.path", p != nullptr ? p : ""); // USER PATHS
    g_properties.emplace_back("sun.boot.library.path", get_boot_lib_path());
    g_properties.emplace_back("jdk.serialFilter", "");// todo
    g_properties.emplace_back("jdk.serialFilterFactory", "");// todo
    g_properties.emplace_back("native.encoding", "UTF-8");// todo
}

static void init_heap() {
    g_heap = new Heap(VM_HEAP_SIZE);
    if (g_heap == nullptr) {
        panic("init Heap failed"); // todo
    }
}

// void set_default_init_args(InitArgs *args) 
// {
//     /* Long longs are used here because with PAE, a 32-bit
//        machine can have more than 4GB of physical memory */
//     // long long phys_mem = nativePhysicalMemory();
    
//     args->asyncgc = false;

//     args->verbosegc = false;
//     args->verbosedll = false;
//     args->verboseclass = false;

//     args->trace_jni_sigs = false;
//     args->compact_specified = false;

//     args->classpath = nullptr;
//     args->bootpath = nullptr;
//     args->bootpath_p = nullptr;
//     args->bootpath_a = nullptr;
//     args->bootpath_c = nullptr;
//     args->bootpath_v = nullptr;

//     // args->java_stack = DEFAULT_STACK;
//     // args->max_heap = phys_mem == 0 ? DEFAULT_MAX_HEAP
//     //                                  : clampHeapLimit(phys_mem/4);
//     // args->min_heap = phys_mem == 0 ? DEFAULT_MIN_HEAP
//     //                                  : clampHeapLimit(phys_mem/64);

//     args->props_count = 0;

//     args->vfprintf = vfprintf;
//     args->abort = abort;
//     args->exit = exit;
// }


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
//	char buffer[size];
    auto buffer = new char[size];
    file.read(buffer, size);
    file.close();
    auto b = strstr(buffer, "JAVA_VERSION=\"17") != nullptr;
    delete[] buffer;
    return b;
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

void init_jvm(InitArgs *init_args) {
    g_java_home = find_jdk_dir();
    if (g_java_home.empty()) {
        panic("java.lang.InternalError, %s\n", "no java lib");
    }

    /* order is important */
    init_heap();
    init_properties();
    init_dll();
    init_classloader();
    init_native();
    init_jni();
    init_reference();
    init_thread();
    init_invoke();
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
    g_main_thread->tobj->set_field_value<jref>("contextClassLoader",
                                     "Ljava/lang/ClassLoader;", g_app_class_loader);
    g_vm_initing = false;
    TRACE("init jvm is over.\n");
}
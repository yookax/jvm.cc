#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include "cabin.h"
#include "runtime//heap.h"
#include "jni.h"
#include "runtime/thread.h"
#include "classfile/invoke.h"
#include "classfile/field.h"
#include "classfile/poly.h"
#include "classfile/module.h"
#include "object/object.h"
#include "object/reflect.h"
#include "reference.h"
#include "sysinfo.h"
#include "interpreter.h"
#include "dll.h"

using namespace std;

Heap *g_heap;

std::vector<Property> g_properties;

Object *g_sys_thread_group;

string g_java_home;

u2 g_classfile_major_version = 0;
u2 g_classfile_manor_version = 0;

Object *g_app_class_loader;
Object *g_platform_class_loader;

bool g_vm_initing = true;

void init_native();

static void *gcLoop(void *dummy) {
    // todo
    // gc(g_heap);
    return nullptr;
}

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
    oss << JVM_MUST_SUPPORT_CLASSFILE_MAJOR_VERSION << "." << JVM_MUST_SUPPORT_CLASSFILE_MINOR_VERSION << ends;
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

string find_jdk_dir();

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
    uc->lookup_field("BIG_ENDIAN", "Z")->static_value.z = is_big_endian();
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
                                     
    // gc thread
    pthread_t tid;
    int ret = pthread_create(&tid, nullptr, gcLoop, nullptr);
    if (ret != 0) {
        panic("create Thread failed"); // todo
    }
    
    g_vm_initing = false;
    TRACE("init jvm is over.\n");
}

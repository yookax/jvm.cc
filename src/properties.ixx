//java.specification.version: 24
//sun.cpu.isalist: amd64
//sun.jnu.encoding: GBK
//java.class.path: D:\code\jvm.cc\test-java\target\classes
//java.vm.vendor: Oracle Corporation
//sun.arch.data.model: 64
//user.variant:
//java.vendor.url: https://java.oracle.com/
//java.vm.specification.version: 24
//os.name: Windows 10
//sun.java.launcher: SUN_STANDARD
//user.country: CN
//sun.boot.library.path: C:\Java\jdk-24\bin
//sun.java.command: SysPropsTest
//jdk.debug: release
//sun.cpu.endian: little
//user.home: C:\Users\admin
//user.language: zh
//sun.stderr.encoding: UTF-8
//java.specification.vendor: Oracle Corporation
//java.version.date: 2025-03-18
//java.home: C:\Java\jdk-24
//file.separator: \
//java.vm.compressedOopsMode: 32-bit
//sun.stdout.encoding: UTF-8
//line.separator:
//
//java.vm.specification.vendor: Oracle Corporation
//java.specification.name: Java Platform API Specification
//user.script:
//sun.management.compiler: HotSpot 64-Bit Tiered Compilers
//java.runtime.version: 24+36-3646
//user.name: admin
//stdout.encoding: UTF-8
//path.separator: ;
//os.version: 10.0
//java.runtime.name: Java(TM) SE Runtime Environment
//file.encoding: UTF-8
//java.vm.name: Java HotSpot(TM) 64-Bit Server VM
//java.vendor.url.bug: https://bugreport.java.com/bugreport/
//java.io.tmpdir: C:\Users\admin\AppData\Local\Temp\
//java.version: 24
//user.dir: D:\code\jvm.cc\test-java
//os.arch: amd64
//java.vm.specification.name: Java Virtual Machine Specification
//sun.os.patch.level:
//native.encoding: GBK
//java.library.path: C:\Java\jdk-24\bin;C:\Windows\Sun\Java\bin;C:\Windows\system32;C:\Windows;C:\Program Files\Common Files\Oracle\Java\javapath;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files\Git\cmd;C:\Program Files\Microsoft SQL Server\150\Tools\Binn\;C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\;C:\Program Files\JetBrains\CLion 2024.3.3\bin;C:\Program Files\JetBrains\CLion 2024.3.3\bin\mingw\bin;D:\Code\jvm.cc\cmake-build-release;D:\Redis-7.4.2-Windows-x64-msys2-with-Service;C:\ProgramData\chocolatey\bin;C:\Program Files\nodejs\;C:\Java\jdk-24\bin;C:\Users\admin\AppData\Local\Programs\Python\Python313\Scripts\;C:\Users\admin\AppData\Local\Programs\Python\Python313\;C:\Users\admin\AppData\Local\Microsoft\WindowsApps;C:\Program Files\JetBrains\IntelliJ IDEA Community Edition 2024.3.2.2\bin;;%RustRover%;C:\Users\admin\AppData\Local\Programs\Microsoft VS Code\bin;C:\Program Files\JetBrains\WebStorm 2024.3.2.1\bin;;C:\Program Files\JetBrains\CLion 2024.3.3\bin;;C:\Program Files\JetBrains\CLion 2024.3.3\bin;C:\Program Files\JetBrains\CLion 2024.3.3\bin\mingw\bin;C:\Program Files\JetBrains\IntelliJ IDEA 2024.2.3\bin;;C:\Users\admin\AppData\Local\Microsoft\WinGet\Packages\Schniz.fnm_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\admin\AppData\Roaming\npm;;.
//java.vm.info: mixed mode, sharing
//stderr.encoding: UTF-8
//java.vendor: Oracle Corporation
//java.vm.version: 24+36-3646
//sun.io.unicode.encoding: UnicodeLittle

module;
#include "vmdef.h"

export module properties;

import std.core;
import sysinfo;
import class_loader;

using namespace std;

// jdk/internal/util/SystemProps.java

enum {
    _display_country_NDX = 0, // "user.country"
    _display_language_NDX, // "user.language"
    _display_script_NDX, // "user.script"
    _display_variant_NDX, // "user.variant"
    _file_encoding_NDX, // 
    _file_separator_NDX, // 
    _format_country_NDX, // 
    _format_language_NDX, // 
    _format_script_NDX, // 
    _format_variant_NDX, // 
    _ftp_nonProxyHosts_NDX, // 
    _ftp_proxyHost_NDX, // 
    _ftp_proxyPort_NDX, // 
    _http_nonProxyHosts_NDX, // 
    _http_proxyHost_NDX, // 
    _http_proxyPort_NDX, // 
    _https_proxyHost_NDX, // 
    _https_proxyPort_NDX, // 
    _java_io_tmpdir_NDX, // 
    _line_separator_NDX, // 
    _os_arch_NDX, // 
    _os_name_NDX, // 
    _os_version_NDX, // 
    _path_separator_NDX, // 
    _socksNonProxyHosts_NDX, // 
    _socksProxyHost_NDX, // 
    _socksProxyPort_NDX, // 
    _stderr_encoding_NDX, // 
    _stdout_encoding_NDX, // 
    _sun_arch_abi_NDX, // 
    _sun_arch_data_model_NDX, // 
    _sun_cpu_endian_NDX, // 
    _sun_cpu_isalist_NDX, // 
    _sun_io_unicode_encoding_NDX, // 
    _sun_jnu_encoding_NDX, //
    _sun_os_patch_level_NDX, // 
    _user_dir_NDX, //
    _user_home_NDX, //
    _user_name_NDX, //
    FIXED_LENGTH
};

static const char *platform_properties[FIXED_LENGTH] = { nullptr };

export pair<const char **, int> get_platform_properties() {
    return make_pair(platform_properties, FIXED_LENGTH);
}

static vector<pair<const char *, const char *>> vm_properties;

export vector<pair<const char *, const char *>>& get_vm_properties() {
    return vm_properties;
}

#if 0
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
    g_properties.emplace_back("os.version",  ""); // todo
    char *p = getenv("USER");
    g_properties.emplace_back("user.name", p != nullptr ? p : "");// todo
    p = getenv("HOME");
    g_properties.emplace_back("user.home", p != nullptr ? p : "");// todo
    char *cwd = get_current_working_directory();
    g_properties.emplace_back("user.dir", cwd);
    //g_properties.emplace_back("user.country", "CN"); // todo
    g_properties.emplace_back("sun.jnu.encoding", "UTF-8");
    //g_properties.emplace_back("sun.stdout.encoding", "UTF-8");// todo
    //g_properties.emplace_back("sun.stderr.encoding", "UTF-8");// todo
    //g_properties.emplace_back("java.io.tmpdir", "");// todo
//    p = getenv("LD_LIBRARY_PATH");
    //g_properties.emplace_back("java.library.path", p != nullptr ? p : ""); // USER PATHS
    g_properties.emplace_back("sun.boot.library.path", get_boot_lib_path());
    g_properties.emplace_back("jdk.serialFilter", "");// todo
    g_properties.emplace_back("jdk.serialFilterFactory", "");// todo
    g_properties.emplace_back("native.encoding", "UTF-8");// todo
#endif
export void init_properties() {
    vm_properties.emplace_back("java.home", g_java_home.c_str());
    vm_properties.emplace_back("java.class.path", get_classpath());

    // ------------------------------------------------------------------------

    platform_properties[_os_name_NDX] = os_name();
    platform_properties[_os_arch_NDX] = os_arch();
    platform_properties[_os_version_NDX] = strdup(os_version().c_str());
    platform_properties[_user_name_NDX] = strdup(get_user_name().c_str());
    platform_properties[_user_home_NDX] = strdup(get_user_home_dir().c_str());
    platform_properties[_user_dir_NDX] = get_current_working_directory();
    platform_properties[_line_separator_NDX] = line_separator();
    platform_properties[_file_encoding_NDX] = "UTF-8";
    platform_properties[_sun_jnu_encoding_NDX] = "UTF-8";
    platform_properties[_file_separator_NDX] = file_separator();
    platform_properties[_path_separator_NDX] = path_separator();
    platform_properties[_java_io_tmpdir_NDX] = ""; // todo
}

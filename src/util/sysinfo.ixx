module;
#include <cassert>
#ifdef _WIN64
#include <Windows.h>
#include <direct.h>
#include <VersionHelpers.h>
#include <shlobj.h>
#elifdef __linux__
#include <unistd.h>
#include <pwd.h>
#include <sys/time.h>
#endif
#include "../vmdef.h"

export module sysinfo;

import std.core;

using namespace std;

export int processor_number() {
#ifdef _WIN64
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwNumberOfProcessors;
#elifdef __linux__
    return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

export int page_size() {
#ifdef _WIN64
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwPageSize;
#elifdef __linux__
    unreachable();
#endif
}

// 返回操作系统的名称。e.g. window 10
export const char *os_name() {
#ifdef _WIN64
    if (IsWindows10OrGreater()) {
        return "Windows 10 or later";
    } else if (IsWindows8Point1OrGreater()) {
        return "Windows 8.1";
    } else if (IsWindows8OrGreater()) {
        return "Windows 8";
    } else if (IsWindows7SP1OrGreater()) {
        return "Windows 7 SP1 or later";
    } else {
        return "Older Windows version";
    }
#elifdef __linux__
    std::ifstream file("/proc/version");
    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        size_t pos = line.find(' ');
        if (pos != std::string::npos) {
            std::string os_name = line.substr(0, pos);
            std::cout << os_name << std::endl;
        }
        file.close();
    } else {
        std::cout << "Unable to open /proc/version" << std::endl;
    }
#endif
}

// 返回操作系统的架构。e.g. amd64
export const char *os_arch() {
#ifdef _WIN64
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            return "INTEL";
        case PROCESSOR_ARCHITECTURE_MIPS:
            return "MIPS";
        case PROCESSOR_ARCHITECTURE_ALPHA:
            return "ALPHA";
        case PROCESSOR_ARCHITECTURE_PPC:
            return "PPC";
        case PROCESSOR_ARCHITECTURE_SHX:
            return "SHX";
        case PROCESSOR_ARCHITECTURE_ARM:
            return "ARM";
        case PROCESSOR_ARCHITECTURE_IA64:
            return "IA64";
        case PROCESSOR_ARCHITECTURE_ALPHA64:
            return "ALPHA64";
        case PROCESSOR_ARCHITECTURE_MSIL:
            return "MSIL";
        case PROCESSOR_ARCHITECTURE_AMD64:
            return "AMD64";
        case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
            return "IA32_ON_WIN64";
        case PROCESSOR_ARCHITECTURE_NEUTRAL:
            return "NEUTRAL";
        case PROCESSOR_ARCHITECTURE_ARM64:
            return "ARM64";
        case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64:
            return "ARM32_ON_WIN64";
        case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64:
            return "IA32_ON_ARM64";
        default:
            return "UNKNOWN ARCHITECTURE";
    }
#elifdef __linux__
    struct utsname uname_data;
    if (uname(&uname_data) != 0) {
        return "Unknown";
    }
    return uname_data.machine;
#endif
}

export string os_version() {
#ifdef _WIN64
    OSVERSIONINFOEX x;
    ZeroMemory(&x, sizeof(OSVERSIONINFOEX));
    x.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((OSVERSIONINFO*)&x)) {
        string s;
        s.append(to_string(x.dwMajorVersion)).append(".")
         .append(to_string(x.dwMinorVersion)).append(".")
         .append(to_string(x.dwBuildNumber));
        return s;
    } else {
        return "";
    }
#elifdef __linux__
#error
#endif
}

export const char *file_separator() {
#ifdef _WIN64
    return "\\";
#elifdef __linux__
    return "/";
#endif
}

export const char *path_separator() {
#ifdef _WIN64
    return ";";
#elifdef __linux__
    return ":";
#endif
}

export const char *line_separator() {
#ifdef _WIN64
    return "\r\n";
#elifdef __linux__
    return "\n";
#endif
}

export string get_user_name() {
    std::string username;
#ifdef _WIN64
    TCHAR buffer[MAX_PATH];
    DWORD len = MAX_PATH;
    if (GetUserName(buffer, &len)) {
        username = string(buffer, buffer + len - 1);
    }
#elif __linux__
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw != nullptr) {
        username = pw->pw_name;
    }
#endif
    return username;
}

export string get_user_home_dir() {
    string home_dir;
#ifdef _WIN32
    TCHAR buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, 0, buffer))) {
        home_dir = string(buffer);
    }
#elif __linux__
    const char* home = getenv("HOME");
    if (home != nullptr) {
        home_dir = home;
    }
#endif
    return home_dir;
}

export char *get_current_working_directory() {
    char *cwd = nullptr;
    int size = 256;

    while (true) {
        cwd = new char[size];

        if(_getcwd(cwd, size) != nullptr) {
            return cwd;
        }
        if(errno == ERANGE) {
            delete[] cwd;
            size *= 2;
        } else {
            unreachable(); // todo Couldn't get cwd
        }
    }
}

export streamsize get_file_size(const string &filename) {
    ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file) {
        std::streamsize size = file.tellg();
        file.close();
        return size;
    }
    return -1;
}

export struct MemMapping {
#ifdef _WIN64
    void *address = nullptr;

    MemMapping(const char *file_path) {
        assert(file_path != nullptr);
        // 打开文件
        HANDLE file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) {
            std::cerr << "Unable to open the file. " << file_path << std::endl;
            throw GetLastError();
        }

        // 创建文件映射对象
        HANDLE mapping = CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (mapping == nullptr) {
            std::cerr << "Unable to create the file mapping. " << file_path << std::endl;
            CloseHandle(file);
            throw GetLastError();
        }

        // 将文件映射到内存
        address = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);

        // 关闭打开的jimage file，只留下内存映射即可。
        // 如果这里不关闭文件，jdk内部打开jimage file时就会出错。
        CloseHandle(mapping);
        CloseHandle(file);

        if (address == nullptr) {
            std::cerr << "Unable to map the file to memory. " << file_path << std::endl;
            throw GetLastError();
        }
    }

    ~MemMapping() {
        if (address != nullptr)
            UnmapViewOfFile(address);
    }
#elifdef __linux__
#error
#endif
};

//export MappingInfo mem_mapping(const char *file_path) {
//    assert(file_path != nullptr);
//    MappingInfo info;
//#ifdef _WIN64
//    // 打开文件
//    info.file = CreateFileA(file_path, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//    if (info.file == INVALID_HANDLE_VALUE) {
//        std::cerr << "无法打开文件" << std::endl;
//        return info;
//    }
//
//    // 创建文件映射对象
//    info.mapping = CreateFileMappingA(info.file, 0, PAGE_READONLY, 0, 0, nullptr);
//    if (info.mapping == nullptr) {
//        std::cerr << "无法创建文件映射对象" << std::endl;
//        CloseHandle(info.file);
//        return info;
//    }
//
//    // 将文件映射到内存
//    info.address = MapViewOfFile(info.mapping, FILE_MAP_READ, 0, 0, 0);
//    if (info.address == nullptr) {
//        std::cerr << "无法将文件映射到内存" << std::endl;
//        CloseHandle(info.mapping);
//        CloseHandle(info.file);
//        return info;
//    }
//
//    assert(info.file != nullptr);
//    assert(info.mapping != nullptr);
//    assert(info.address != nullptr);
//    return info;
//#elifdef __linux__
//#error
//#endif
//}
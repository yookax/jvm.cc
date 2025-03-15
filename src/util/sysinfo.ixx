module;
#ifdef _WIN64
#include <Windows.h>
#include <direct.h>
#include <VersionHelpers.h>
#elifdef __linux__
#include <unistd.h>
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
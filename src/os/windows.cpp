#ifdef _WIN64

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <windows.h>
#include "../cabin.h"
#include "../dll.h"

using namespace std;

int processor_number() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwNumberOfProcessors;
}

int page_size() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwPageSize;
}

const char *os_name() {
	SYSTEM_INFO info;        //用SYSTEM_INFO结构判断64位AMD处理器
	GetSystemInfo(&info);    //调用GetSystemInfo函数填充结构
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (GetVersionEx((OSVERSIONINFO *)&os)) {
		//下面根据版本信息判断操作系统名称
		switch (os.dwMajorVersion) { //判断主版本号
		case 4:
			switch (os.dwMinorVersion) { //判断次版本号
			case 0:
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
					return "Microsoft Windows NT 4.0"; //1996年7月发布
				else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
					return "Microsoft Windows 95";
				break;
			case 10:
				return "Microsoft Windows 98";
			case 90:
				return "Microsoft Windows Me";
			default:
				break;
			}
			break;
		case 5:
			switch (os.dwMinorVersion) { //再比较dwMinorVersion的值
			case 0:
				return "Microsoft Windows 2000"; //1999年12月发布
			case 1:
				return "Microsoft Windows XP"; //2001年8月发布
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION
					&& info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					return ("Microsoft Windows XP Professional x64 Edition");
				else if (GetSystemMetrics(SM_SERVERR2) == 0)
					return ("Microsoft Windows Server 2003"); //2003年3月发布
				else if (GetSystemMetrics(SM_SERVERR2) != 0)
					return "Microsoft Windows Server 2003 R2";
				break;
			default:
				break;
			}
			break;
		case 6:
			switch (os.dwMinorVersion) {
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					return "Microsoft Windows Vista";
				else
					return "Microsoft Windows Server 2008";//服务器版本
				break;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
					return "Microsoft Windows 7";
				else
					return "Microsoft Windows Server 2008 R2";
				break;
			case 2:
				if (os.wProductType != VER_NT_WORKSTATION)
					return "Windows Server 2012";
				else
					return "Windows 8";
				break;
			case 3:
				if (os.wProductType == VER_NT_WORKSTATION)
					return "Windows 8.1";
				else
					return "Windows Server 2012 R2";
				break;
			default:
				break;
			}
			break;
		case 10:
			switch (os.dwMinorVersion) {
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)
					return "Windows 10";
				else
					return "Windows Server 2016";
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	return "unknown OperatingSystem";
}

const char *os_arch() {
    SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
		return "INTEL";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_MIPS) {
		return "MIPS";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA) {
		return "ALPHA";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_PPC) {
		return "PPC";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_SHX) {
		return "SHX";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM) {
		return "ARM";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
		return "IA64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64) {
		return "ALPHA64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_MSIL) {
		return "MSIL";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
		return "AMD64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64) {
		return "IA32_ON_WIN64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_NEUTRAL) {
		return "NEUTRAL";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) {
		return "ARM64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64) {
		return "ARM32_ON_WIN64";
	} else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_ARM64) {
		return "IA32_ON_ARM64";
	} else {
        return "UNKNOWN ARCHITECTURE";
    }
}

const char *file_separator() { return "\\"; }

const char *path_separator() { return ";"; }

const char *line_separator() { return "\r\n"; }

#include <unistd.h>  // todo

char *get_current_working_directory() {
	char *cwd = nullptr;
    int size = 256;

    while (true) {
        cwd = new char[size];

        if(getcwd(cwd, size) != nullptr) {
			return cwd;
		}

		if(errno == ERANGE) {
			delete[] cwd;
			size *= 2;
		} else {
			panic("Couldn't get cwd");
		}
    }

	// // 定义一个足够大的缓冲区来存储当前工作目录
	// const int bufferSize = MAX_PATH;
	// char currentDirectory[bufferSize];
	//
	// // 调用 GetCurrentDirectory 函数获取当前工作目录
	// DWORD result = GetCurrentDirectory(bufferSize, currentDirectory);
	//
	// if (result != 0) {
	// 	// 成功获取当前工作目录
	// 	std::cout << "当前工作目录是: " << currentDirectory << std::endl;
	// } else {
	// 	// 获取失败，输出错误信息
	// 	std::cerr << "无法获取当前工作目录。错误代码: " << GetLastError() << std::endl;
	// }
	//
	// return 0;
}

void *find_library_entry(void *handle, const char *name) {
    assert(handle != nullptr && name != nullptr);
    return (void *) GetProcAddress((HMODULE) handle, name);
}

void *open_library_os_depend(const char *name) {
    assert(name != nullptr);
    void *handle = LoadLibrary(name);
    if (handle == nullptr) {
        DWORD e = GetLastError();
        ERR("Load dll failed. %ld, %s\n", e, name); // todo
        return nullptr;
    }

    return handle;
}

namespace fs = std::filesystem;

bool check_jdk_version(string jdk_path) {
	std::ifstream file(jdk_path + "/release", std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// 读取文件内容到缓冲区
	char buffer[size];
	file.read(buffer, size);
	file.close();
	return strstr(buffer, "JAVA_VERSION=\"17") != nullptr;
}

string find_jdk_dir() {
	// 查找"JAVA_HOME"环境变量
	if (char buffer[MAX_PATH]; GetEnvironmentVariable("JAVA_HOME", buffer, MAX_PATH) > 0) {
		if (check_jdk_version(buffer))
			return std::string(buffer);
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

#endif
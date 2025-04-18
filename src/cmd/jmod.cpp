//#include "../../lib/minizip/unzip.h"
//
//import std.core;
//
//using namespace std;
//
//static void list0(const char *jmod) {
//    unzFile zip_file = unzOpen(jmod);
//    if (zip_file == nullptr) {
//        fprintf(stderr, "Unable to open the jmod file: %s\n", jmod);
//        return;
//    }
//
//    // 获取ZIP文件信息
//    unz_global_info global_info;
//    if (unzGetGlobalInfo(zip_file, &global_info) != UNZ_OK) {
//        fprintf(stderr, "无法获取ZIP文件全局信息\n");
//        unzClose(zip_file);
//        return;
//    }
//
//    // 遍历ZIP文件中的每个文件
//    for (uLong i = 0; i < global_info.number_entry; i++) {
//        unz_file_info file_info;
//        char filename[256];
//
//        // 获取当前文件信息
//        if (unzGetCurrentFileInfo(zip_file, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) {
//            fprintf(stderr, "无法获取文件信息\n");
//            break;
//        }
//
//        // 打印文件名
//        printf("%s\n", filename);
//
//        // 移动到下一个文件
//        if (i < global_info.number_entry - 1) {
//            if (unzGoToNextFile(zip_file) != UNZ_OK) {
//                fprintf(stderr, "无法移动到下一个文件\n");
//                break;
//            }
//        }
//    }
//
//    // 关闭ZIP文件
//    unzClose(zip_file);
//}
//
//static void describe(const char *jmod) {
//
//}
//
//int main(int argc, char* argv[]) {
//    list0("C:/Java/jdk-24/jmods/java.base.jmod");
//    return 0;
//}

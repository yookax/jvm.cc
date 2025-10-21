#include "test.h"

using namespace std;

// TEST_CASE(test_encoding1)
//     pair<u8string /* utf8 */, wstring /* unicode */> pairs[] = {
//             { u8"Hello, World!", L"Hello, World!" },
//             { u8"你好，世界！", L"你好，世界！" },
//             { u8"Hello, 你好😀", L"Hello, 你好😀" },
//     };
//
//     for (auto &p: pairs) {
//         u8string utf8_str = unicode_to_utf8(p.second);
//         if (utf8_str != p.first)
//             cout << "failed." << endl;
//     }
// }

void test_utf8_to_latin1() {
//    for (auto &a: strings_for_testing) {
//        auto x = utf8_to_latin1(a.s8);
//        if (x.has_value()) {
//            std::cout << (char *) a.s8.c_str() << " <---> "<< x.value() << std::endl;
//        }
//    }
}

void test_utf8_to_utf16() {
    bool failed = false;
    for (auto &a: strings_for_testing) {
        if (utf8_to_utf16(a.s8) != a.s16) {
            failed = true;
            std::cerr << "failed. " << (const char *) a.s8.c_str() << std::endl;
        }
    }

    if(!failed)
        cout << "passed." << endl;
}

void test_utf16_to_utf8() {
    bool failed = false;
    for (auto &a: strings_for_testing) {
        if (utf16_to_utf8(a.s16) != a.s8) {
            failed = true;
            std::cerr << "failed. " << (const char *) a.s8.c_str() << std::endl;
        }
    }

    if(!failed)
        cout << "passed." << endl;
}
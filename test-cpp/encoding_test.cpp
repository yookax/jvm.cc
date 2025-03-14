#include "test.h"

using namespace std;

TEST_CASE(test_encoding1)
    pair<u8string /* utf8 */, wstring /* unicode */> pairs[] = {
            { u8"Hello, World!", L"Hello, World!" },
            { u8"你好，世界！", L"你好，世界！" },
            { u8"Hello, 你好😀", L"Hello, 你好😀" },
    };

    for (auto &p: pairs) {
        u8string utf8_str = unicode_to_utf8(p.second);
        if (utf8_str != p.first)
            cout << "failed." << endl;
    }
}
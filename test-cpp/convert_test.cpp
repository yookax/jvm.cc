#include "test.h"

using namespace std;

import convert;

TEST_CASE(test_convert_int)
    vector <jint> ints;
    ints.push_back(0);
    ints.push_back(1234567);
    ints.push_back(std::numeric_limits<int32_t>::max());

    uint8_t bytes[sizeof(jint)];
    for (auto i: ints) {
        int32_to_bytes(i, bytes);
        auto x = bytes_to_int32(bytes);
        if (i != x) {
            printf("failed. %d, %d\n", i, x);
        }
    }
}

TEST_CASE(test_convert_long)
    vector <jlong> longs;
    longs.push_back(0L);
    longs.push_back(1234567L);
    longs.push_back(std::numeric_limits<int64_t>::max());

    uint8_t bytes[sizeof(jlong)];
    for (auto l: longs) {
        int64_to_bytes(l, bytes);
        auto x = bytes_to_int64(bytes);
        if (l != x) {
            printf("failed. %lld, %lld\n", l, x);
        }
    }
}

TEST_CASE(test_convert_float)
    vector<float> floats;
    floats.push_back(0);
    floats.push_back(23232.716986828);
    floats.push_back(1112.4985495834085);
    floats.push_back(0.71828);

    unsigned char floatBytes[sizeof(float)];

    for (auto f: floats) {
// Convert float to big-endian bytes
        float_to_bytes(f, floatBytes);
//    std::cout << "Float to big-endian bytes: ";
//    for (size_t i = 0; i < sizeof(float); ++i) {
//        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(floatBytes[i]) << " ";
//    }
//    std::cout << std::endl;

// Convert big-endian bytes back to float
        auto result = bytes_to_float(floatBytes);
        std::cout << std::setprecision(20) << f << ", " << result << std::endl;
    }
}

TEST_CASE(test_convert_double)
    vector<double> doubles;
    doubles.push_back(0);
    doubles.push_back(23232.716986828);
    doubles.push_back(1112.4985495834085);
    doubles.push_back(0.71828);

    unsigned char doubleBytes[sizeof(double)];

    for (auto d: doubles) {
// Convert double to big-endian bytes
        double_to_bytes(d, doubleBytes);
//        std::cout << "Double to big-endian bytes: ";
//        for (size_t i = 0; i < sizeof(double); ++i) {
//            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(doubleBytes[i]) << " ";
//        }
//        std::cout << std::endl;

// Convert big-endian bytes back to double
        auto result = bytes_to_double(doubleBytes);
        std::cout << std::setprecision(20) << d << ", " << result << std::endl;
    }
}

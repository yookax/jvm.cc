#include "test.h"

using namespace std;

import convert;

TEST_CASE(test_convert_int)
    vector <int32_t> ints;
    ints.push_back(numeric_limits<int32_t>::min());
    ints.push_back(-1234567);
    ints.push_back(0);
    ints.push_back(1234567);
    ints.push_back(numeric_limits<int32_t>::max());

    bool failed = false;
    uint8_t bytes[sizeof(int32_t)];
    for (auto i: ints) {
        int32_to_bytes(i, bytes, endian::big);
        int32_t x = bytes_to_int32(bytes, endian::big);
        if (i != x) {
            failed = true;
            printf("failed-1. %d, %d\n", i, x);
        }

        int32_to_bytes(i, bytes, endian::little);
        x = bytes_to_int32(bytes, endian::little);
        if (i != x) {
            failed = true;
            printf("failed-2. %d, %d\n", i, x);
        }
    }

    if (!failed)
        printf("passed.\n");
}

TEST_CASE(test_convert_long)
    vector <int64_t> longs;
    longs.push_back(numeric_limits<int64_t>::min());
    longs.push_back(-1234567L);
    longs.push_back(0L);
    longs.push_back(1234567L);
    longs.push_back(numeric_limits<int64_t>::max());

    bool failed = false;
    uint8_t bytes[sizeof(int64_t)];
    for (auto l: longs) {
        int64_to_bytes(l, bytes, endian::big);
        int64_t x = bytes_to_int64(bytes, endian::big);
        if (l != x) {
            failed = true;
            printf("failed-1. %lld, %lld\n", l, x);
        }

        int64_to_bytes(l, bytes, endian::little);
        x = bytes_to_int64(bytes, endian::little);
        if (l != x) {
            failed = true;
            printf("failed-1. %lld, %lld\n", l, x);
        }
    }

    if (!failed)
        printf("passed.\n");
}

TEST_CASE(test_convert_float)
    vector<float> floats;
    floats.push_back(0);
    floats.push_back(23232.716986828f);
    floats.push_back(1112.4985495834085f);
    floats.push_back(0.71828f);

    bool failed = false;
    uint8_t float_bytes[sizeof(float)];
    for (auto f: floats) {
        float_to_bytes(f, float_bytes, endian::big);
        float x = bytes_to_float(float_bytes, endian::big);
        if (f != x) {
            failed = true;
            cout << "failed-1. "<< setprecision(20) << f << ", " << x << endl;
        }

        float_to_bytes(f, float_bytes, endian::little);
        x = bytes_to_float(float_bytes, endian::little);
        if (f != x) {
            failed = true;
            cout << "failed-2. "<< setprecision(20) << f << ", " << x << endl;
        }
    }

    if (!failed)
        printf("passed.\n");
}

TEST_CASE(test_convert_double)
    vector<double> doubles;
    doubles.push_back(0);
    doubles.push_back(23232.716986828);
    doubles.push_back(1112.4985495834085);
    doubles.push_back(0.71828);
    doubles.push_back(3.141592653589793);

    bool failed = false;
    uint8_t double_bytes[sizeof(double)];
    for (auto d: doubles) {
        double_to_bytes(d, double_bytes, endian::big);
        double x = bytes_to_double(double_bytes, endian::big);
        if (d != x) {
            failed = true;
            cout << "failed-1. "<< setprecision(20) << d << ", " << x << endl;
        }

        double_to_bytes(d, double_bytes, endian::little);
        x = bytes_to_double(double_bytes, endian::little);
        if (d != x) {
            failed = true;
            cout << "failed-2. "<< setprecision(20) << d << ", " << x << endl;
        }
    }

    if (!failed)
        printf("passed.\n");
}

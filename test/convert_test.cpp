#include "test.h"

using namespace std;

import convert;

void test_convert_int() {
    vector <int32_t> ints;
    ints.push_back(numeric_limits<int32_t>::min());
    ints.push_back(-1234567);
    ints.push_back(0);
    ints.push_back(1234567);
    ints.push_back(numeric_limits<int32_t>::max());

    bool failed = false;
    uint8_t bytes[sizeof(int32_t)];
    for (auto i: ints) {
        int32ToBytes(i, bytes, endian::big);
        int32_t x = bytesToInt32(bytes, endian::big);
        if (i != x) {
            failed = true;
            printf("failed-1. %d, %d\n", i, x);
        }

        int32ToBytes(i, bytes, endian::little);
        x = bytesToInt32(bytes, endian::little);
        if (i != x) {
            failed = true;
            printf("failed-2. %d, %d\n", i, x);
        }
    }

    if (!failed)
        printf("passed.\n");
}

void test_convert_long() {
    vector <int64_t> longs;
    longs.push_back(numeric_limits<int64_t>::min());
    longs.push_back(-1234567L);
    longs.push_back(0L);
    longs.push_back(1234567L);
    longs.push_back(numeric_limits<int64_t>::max());

    bool failed = false;
    uint8_t bytes[sizeof(int64_t)];
    for (auto l: longs) {
        int64ToBytes(l, bytes, endian::big);
        int64_t x = bytesToInt64(bytes, endian::big);
        if (l != x) {
            failed = true;
            printf("failed-1. %lld, %lld\n", l, x);
        }

        int64ToBytes(l, bytes, endian::little);
        x = bytesToInt64(bytes, endian::little);
        if (l != x) {
            failed = true;
            printf("failed-1. %lld, %lld\n", l, x);
        }
    }

    if (!failed)
        printf("passed.\n");
}

void test_convert_float() {
    vector<float> floats;
    floats.push_back(0);
    floats.push_back(23232.716986828f);
    floats.push_back(1112.4985495834085f);
    floats.push_back(0.71828f);

    bool failed = false;
    uint8_t float_bytes[sizeof(float)];
    for (auto f: floats) {
        floatToBytes(f, float_bytes, endian::big);
        float x = bytesToFloat(float_bytes, endian::big);
        if (f != x) {
            failed = true;
            cout << "failed-1. "<< setprecision(20) << f << ", " << x << endl;
        }

        floatToBytes(f, float_bytes, endian::little);
        x = bytesToFloat(float_bytes, endian::little);
        if (f != x) {
            failed = true;
            cout << "failed-2. "<< setprecision(20) << f << ", " << x << endl;
        }
    }

    if (!failed)
        printf("passed.\n");
}

void test_convert_double() {
    vector<double> doubles;
    doubles.push_back(0);
    doubles.push_back(23232.716986828);
    doubles.push_back(1112.4985495834085);
    doubles.push_back(0.71828);
    doubles.push_back(3.141592653589793);

    bool failed = false;
    uint8_t double_bytes[sizeof(double)];
    for (auto d: doubles) {
        doubleToBytes(d, double_bytes, endian::big);
        double x = bytesToDouble(double_bytes, endian::big);
        if (d != x) {
            failed = true;
            cout << "failed-1. "<< setprecision(20) << d << ", " << x << endl;
        }

        doubleToBytes(d, double_bytes, endian::little);
        x = bytesToDouble(double_bytes, endian::little);
        if (d != x) {
            failed = true;
            cout << "failed-2. "<< setprecision(20) << d << ", " << x << endl;
        }
    }

    if (!failed)
        printf("passed.\n");
}
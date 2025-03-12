module;
#include "cabin.h"

module vmstd;

import std.core;

using namespace std;

// 将大端字节数组转换为整数
template <typename T>
static T big_endian_bytes_to_int(const unsigned char bytes[]) {
    T value = 0;
    if constexpr (std::endian::native == std::endian::big) {
        // 如果本地字节序是大端，直接复制
        std::memcpy(&value, bytes, sizeof(T));
    } else {
        // 如果本地字节序是小端，需要进行字节序转换
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            value <<= 8;
            value |= static_cast<T>(bytes[i]);
        }
    }
    return value;
}

int32_t bytes_to_int32(const uint8_t *bytes) {
    return big_endian_bytes_to_int<int32_t>(bytes);
}

//int64_t bytes_to_int64(const uint8_t *bytes) {
//    int64_t high = ((int64_t) bytes_to_int32(bytes)) << 32;
//    int64_t low = bytes_to_int32(bytes + 4) & 0x00000000ffffffff;
//    return high | low;
//}

int64_t bytes_to_int64(const uint8_t *bytes) {
    return big_endian_bytes_to_int<int64_t>(bytes);
}

//jfloat bytes_to_float(const uint8_t *bytes) {
//    return int_bits_to_float(bytes_to_int32(bytes));
//}
jfloat bytes_to_float(const uint8_t *bytes) {
    auto k = bytes_to_int32(bytes);
    return *(jfloat *)&k;
}

//jdouble bytes_to_double(const uint8_t *bytes) {
//    return long_bits_to_double(bytes_to_int64(bytes));
//}
jdouble bytes_to_double(const uint8_t *bytes) {
    auto k = bytes_to_int64(bytes);
    return *(jdouble *)&k;
}

// 将整数转换为大端字节数组
template <typename T>
static void int_to_big_endian_bytes(T value, unsigned char bytes[]) {
    if constexpr (std::endian::native == std::endian::big) {
        // 如果本地字节序是大端，直接复制
        std::memcpy(bytes, &value, sizeof(T));
    } else {
        // 如果本地字节序是小端，需要进行字节序转换
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            bytes[sizeof(T) - 1 - i] = static_cast<std::uint8_t>(value & 0xFF);
            value >>= 8;
        }
    }
}

// Convert a floating-point number (either float or double) to a big-endian byte array
template <typename T>
static void floating_point_to_big_endian_bytes(T num, unsigned char bytes[]) {
    static_assert(std::is_floating_point<T>::value, "Template argument must be a floating-point type");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType rawValue = *reinterpret_cast<IntType*>(&num);
    size_t size = sizeof(T);

    if (std::endian::native == std::endian::big) {
        // If the system is big-endian, just copy the bytes directly
        for (size_t i = 0; i < size; ++i) {
            bytes[i] = static_cast<unsigned char>((rawValue >> ((size - 1 - i) * 8)) & 0xFF);
        }
    } else {
        // If the system is little-endian, reverse the byte order
        for (size_t i = 0; i < size; ++i) {
            bytes[i] = static_cast<unsigned char>((rawValue >> (i * 8)) & 0xFF);
        }
    }
}

TEST_CASE(test_convert_int, {
    vector<jint> ints;
    ints.push_back(0);
    ints.push_back(1234567);
    ints.push_back(std::numeric_limits<int32_t>::max());

    uint8_t bytes[sizeof(jint)];
    for (auto i: ints) {
        int_to_big_endian_bytes(i, bytes);
        auto x = bytes_to_int32(bytes);
        if (i != x) {
            printf("failed. %d, %d\n", i, x);
        }
    }
})

TEST_CASE(test_convert_long, {
    vector<jlong> longs;
    longs.push_back(0L);
    longs.push_back(1234567L);
    longs.push_back(std::numeric_limits<int64_t>::max());

    uint8_t bytes[sizeof(jlong)];
    for (auto l: longs) {
        int_to_big_endian_bytes(l, bytes);
        auto x = bytes_to_int64(bytes);
        if (l != x) {
            printf("failed. %lld, %lld\n", l, x);
        }
    }
})

TEST_CASE(test_convert_float, {
    vector<float> floats;
    floats.push_back(0);
    floats.push_back(23232.716986828);
    floats.push_back(1112.4985495834085);
    floats.push_back(0.71828);

    unsigned char floatBytes[sizeof(float)];

    for (auto f: floats) {
        // Convert float to big-endian bytes
        floating_point_to_big_endian_bytes(f, floatBytes);
//    std::cout << "Float to big-endian bytes: ";
//    for (size_t i = 0; i < sizeof(float); ++i) {
//        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(floatBytes[i]) << " ";
//    }
//    std::cout << std::endl;

        // Convert big-endian bytes back to float
        auto result = bytes_to_float(floatBytes);
        std::cout << std::setprecision(20) << f << ", " << result << std::endl;
    }
})

TEST_CASE(test_convert_double, {
    vector<double> doubles;
    doubles.push_back(0);
    doubles.push_back(23232.716986828);
    doubles.push_back(1112.4985495834085);
    doubles.push_back(0.71828);

    unsigned char doubleBytes[sizeof(double)];

    for (auto d: doubles) {
        // Convert double to big-endian bytes
        floating_point_to_big_endian_bytes(d, doubleBytes);
//        std::cout << "Double to big-endian bytes: ";
//        for (size_t i = 0; i < sizeof(double); ++i) {
//            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(doubleBytes[i]) << " ";
//        }
//        std::cout << std::endl;

        // Convert big-endian bytes back to double
        auto result = bytes_to_double(doubleBytes);
        std::cout << std::setprecision(20) << d << ", " << result << std::endl;
    }
})

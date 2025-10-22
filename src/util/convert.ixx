module;
#include "../vmdef.h"

export module convert;

import std;

using namespace std;

//template <typename T>
//T swapEndian(T value) {
//    static_assert(std::is_integral_v<T>, "swapEndian only supports integral types");
//    T result = 0;
//    for (size_t i = 0; i < sizeof(T); ++i) {
//        result |= ( static_cast<T>((((uint8_t*)(&value))[i]) << ((sizeof(T) - 1 - i) * 8)) );
//    }
//    return result;
//}
template <typename T>
T swapEndian(T value) {
    union {
        T value;
        unsigned char bytes[sizeof(T)];
    } source, dest;

    source.value = value;
    for (size_t i = 0; i < sizeof(T); ++i) {
        dest.bytes[i] = source.bytes[sizeof(T) - 1 - i];
    }
    return dest.value;
}


//// 交换 32 位整数的字节序
//uint32_t swapEndian(uint32_t value) {
//    return ((value & 0xFF) << 24) |
//           ((value & 0xFF00) << 8) |
//           ((value & 0xFF0000) >> 8) |
//           ((value & 0xFF000000) >> 24);
//}
//
//// 交换 64 位整数的字节序
//uint64_t swapEndian(uint64_t value) {
//    return ((value & 0xFF) << 56) |
//           ((value & 0xFF00) << 40) |
//           ((value & 0xFF0000) << 24) |
//           ((value & 0xFF000000) << 8) |
//           ((value & 0xFF00000000) >> 8) |
//           ((value & 0xFF0000000000) >> 24) |
//           ((value & 0xFF000000000000) >> 40) |
//           ((value & 0xFF00000000000000) >> 56);
//}

template <typename T>
T bytesToInt(const uint8_t *bytes, std::endian e) {
    T value = 0;
    std::memcpy(&value, bytes, sizeof(T));
    if (std::endian::native != e) {
        value = swapEndian(value);
    }
//    else {
//        // 进行字节序转换
//        value = swapEndian(value);
////        for (std::size_t i = 0; i < sizeof(T); ++i) {
////            value <<= 8;
////            value |= static_cast<T>(bytes[i]);
////        }
//    }
    return value;
}

export int16_t bytesToInt16(const uint8_t *bytes, std::endian e) {
    return bytesToInt<int16_t>(bytes, e);
}

export int32_t bytesToInt32(const uint8_t *bytes, std::endian e) {
    return bytesToInt<int32_t>(bytes, e);
}

export int64_t bytesToInt64(const uint8_t *bytes, std::endian e) {
    return bytesToInt<int64_t>(bytes, e);
}

// 将整数转换为大端字节数组
template <typename T>
void intToBytes(T value, uint8_t *bytes, std::endian e) {
    if (std::endian::native != e) {
        value = swapEndian(value);
    }
    std::memcpy(bytes, &value, sizeof(T));

//    if (std::endian::native == e) {
//        // 如果本地字节序和bytes的字节序相同，直接复制
//        std::memcpy(bytes, &value, sizeof(T));
//    } else {
//        // 进行字节序转换
//        for (std::size_t i = 0; i < sizeof(T); ++i) {
//            bytes[sizeof(T) - 1 - i] = static_cast<std::uint8_t>(value & 0xFF);
//            value >>= 8;
//        }
//    }
}

export void int32ToBytes(int32_t value, uint8_t *bytes, std::endian e) {
    return intToBytes<int32_t>(value, bytes, e);
}

export void int64ToBytes(int64_t value, uint8_t *bytes, std::endian e) {
    return intToBytes<int64_t>(value, bytes, e);
}

// Convert a byte array to a floating-point number (either float or double)
template <typename T>
T bytesToFloatingPointNumber(const uint8_t* bytes, std::endian target_endian) {
    static_assert(std::is_floating_point_v<T>, "only supports floating-point types");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType raw_value;
    std::memcpy(&raw_value, bytes, sizeof(T));

    if (std::endian::native != target_endian) {
        raw_value = swapEndian(raw_value);
    }

    return *reinterpret_cast<T*>(&raw_value);
}

export float bytesToFloat(const uint8_t* bytes, std::endian bytes_endian) {
    return bytesToFloatingPointNumber<float>(bytes, bytes_endian);
}

export double bytesToDouble(const uint8_t* bytes, std::endian bytes_endian) {
    return bytesToFloatingPointNumber<double>(bytes, bytes_endian);
}

// Convert a floating-point number (either float or double) to a byte array
template <typename T>
void floatingPointNumberToBytes(T num, uint8_t* bytes, std::endian bytes_endian) {
    static_assert(std::is_floating_point_v<T>, "only supports floating-point types");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType raw_value = *reinterpret_cast<IntType*>(&num);

    if (std::endian::native != bytes_endian) {
        raw_value = swapEndian(raw_value);
    }

    std::memcpy(bytes, &raw_value, sizeof(T));
}

export void floatToBytes(float num, uint8_t* bytes, std::endian bytes_endian) {
    floatingPointNumberToBytes<float>(num, bytes, bytes_endian);
}

export void doubleToBytes(double num, uint8_t* bytes, std::endian bytes_endian) {
    floatingPointNumberToBytes<double>(num, bytes, bytes_endian);
}
//
///*
// * 将字节数组转换为32位整形.
// * 字节数组bytes按大端存储，长度4.
// */
//export int32_t big_bytes_to_int32(const uint8_t *bytes) {
//    return bytes_to_int<int32_t>(bytes, std::endian::big);
////    return big_endian_bytes_to_int<int32_t>(bytes);
//}
//
////export void int32_to_bytes(int32_t value, unsigned char bytes[]);
//
///*
// * 将字节数组转换为64位整形.
// * 字节数组bytes按大端存储，长度8.
// */
////export int64_t bytes_to_int64(const uint8_t *bytes);
////export void int64_to_bytes(int64_t value, unsigned char bytes[]);
//
///*
// * 将字节数组转换为32位浮点数.
// * 字节数组bytes按大端存储，长度4.
// */
//export jfloat bytes_to_float(const uint8_t *bytes);
//export void float_to_bytes(float f, unsigned char bytes[]);
//
///*
// * 将字节数组转换为64位浮点数.
// * 字节数组bytes按大端存储，长度8.
// */
//export jdouble bytes_to_double(const uint8_t *bytes);
//export void double_to_bytes(double d, unsigned char bytes[]);
//
////module : private;
//
//// 将大端字节数组转换为整数
//template <typename T>
//T big_endian_bytes_to_int(const unsigned char bytes[]) {
//    T value = 0;
//    if constexpr (std::endian::native == std::endian::big) {
//        // 如果本地字节序是大端，直接复制
//        std::memcpy(&value, bytes, sizeof(T));
//    } else {
//        // 如果本地字节序是小端，需要进行字节序转换
//        for (std::size_t i = 0; i < sizeof(T); ++i) {
//            value <<= 8;
//            value |= static_cast<T>(bytes[i]);
//        }
//    }
//    return value;
//}



//int64_t bytes_to_int64(const uint8_t *bytes) {
//    int64_t high = ((int64_t) big_bytes_to_int32(bytes)) << 32;
//    int64_t low = big_bytes_to_int32(bytes + 4) & 0x00000000ffffffff;
//    return high | low;
//}

//int64_t bytes_to_int64(const uint8_t *bytes) {
//    return bytes_to_int<int64_t>(bytes, std::endian::big);
////    return big_endian_bytes_to_int<int64_t>(bytes);
//}

//jfloat bytes_to_float(const uint8_t *bytes) {
//    return int_bits_to_float(big_bytes_to_int32(bytes));
//}
//jfloat bytes_to_float(const uint8_t *bytes) {
//    auto k = big_bytes_to_int32(bytes);
//    return *(jfloat *)&k;
//}

//jdouble bytes_to_double(const uint8_t *bytes) {
//    return long_bits_to_double(bytes_to_int64(bytes));
//}
//jdouble bytes_to_double(const uint8_t *bytes) {
//    auto k = bytes_to_int64(bytes);
//    return *(jdouble *)&k;
//}

//// 将整数转换为大端字节数组
//template <typename T>
//void int_to_big_endian_bytes(T value, unsigned char bytes[]) {
//    if constexpr (std::endian::native == std::endian::big) {
//        // 如果本地字节序是大端，直接复制
//        std::memcpy(bytes, &value, sizeof(T));
//    } else {
//        // 如果本地字节序是小端，需要进行字节序转换
//        for (std::size_t i = 0; i < sizeof(T); ++i) {
//            bytes[sizeof(T) - 1 - i] = static_cast<std::uint8_t>(value & 0xFF);
//            value >>= 8;
//        }
//    }
//}

//void int32_to_bytes(int32_t value, unsigned char bytes[]) {
//    int_to_big_endian_bytes(value, bytes);
//}

//void int64_to_bytes(int64_t value, unsigned char bytes[]) {
//    int_to_big_endian_bytes(value, bytes);
//}

// Convert a floating-point number (either float or double) to a big-endian byte array
//template <typename T>
//void floating_point_to_big_endian_bytes(T num, unsigned char bytes[]) {
//    static_assert(std::is_floating_point<T>::value, "Template argument must be a floating-point type");
//    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
//    IntType rawValue = *reinterpret_cast<IntType*>(&num);
//    size_t size = sizeof(T);
//
//    if (std::endian::native == std::endian::big) {
//        // If the system is big-endian, just copy the bytes directly
//        for (size_t i = 0; i < size; ++i) {
//            bytes[i] = static_cast<unsigned char>((rawValue >> ((size - 1 - i) * 8)) & 0xFF);
//        }
//    } else {
//        // If the system is little-endian, reverse the byte order
//        for (size_t i = 0; i < size; ++i) {
//            bytes[i] = static_cast<unsigned char>((rawValue >> (i * 8)) & 0xFF);
//        }
//    }
//}
//
//void float_to_bytes(float f, unsigned char bytes[]) {
//    floating_point_to_big_endian_bytes(f, bytes);
//}
//
//void double_to_bytes(double d, unsigned char bytes[]) {
//    floating_point_to_big_endian_bytes(d, bytes);
//}

// export TEST_CASE(test_convert_int)
//     vector <int32_t> ints;
//     ints.push_back(numeric_limits<int32_t>::min());
//     ints.push_back(-1234567);
//     ints.push_back(0);
//     ints.push_back(1234567);
//     ints.push_back(numeric_limits<int32_t>::max());
//
//     bool failed = false;
//     uint8_t bytes[sizeof(int32_t)];
//     for (auto i: ints) {
//         int32ToBytes(i, bytes, endian::big);
//         int32_t x = bytesToInt32(bytes, endian::big);
//         if (i != x) {
//             failed = true;
//             printf("failed-1. %d, %d\n", i, x);
//         }
//
//         int32ToBytes(i, bytes, endian::little);
//         x = bytesToInt32(bytes, endian::little);
//         if (i != x) {
//             failed = true;
//             printf("failed-2. %d, %d\n", i, x);
//         }
//     }
//
//     if (!failed)
//         printf("passed.\n");
// }
//
// export TEST_CASE(test_convert_long)
//     vector <int64_t> longs;
//     longs.push_back(numeric_limits<int64_t>::min());
//     longs.push_back(-1234567L);
//     longs.push_back(0L);
//     longs.push_back(1234567L);
//     longs.push_back(numeric_limits<int64_t>::max());
//
//     bool failed = false;
//     uint8_t bytes[sizeof(int64_t)];
//     for (auto l: longs) {
//         int64ToBytes(l, bytes, endian::big);
//         int64_t x = bytesToInt64(bytes, endian::big);
//         if (l != x) {
//             failed = true;
//             printf("failed-1. %lld, %lld\n", l, x);
//         }
//
//         int64ToBytes(l, bytes, endian::little);
//         x = bytesToInt64(bytes, endian::little);
//         if (l != x) {
//             failed = true;
//             printf("failed-1. %lld, %lld\n", l, x);
//         }
//     }
//
//     if (!failed)
//         printf("passed.\n");
// }
//
// export TEST_CASE(test_convert_float)
//     vector<float> floats;
//     floats.push_back(0);
//     floats.push_back(23232.716986828f);
//     floats.push_back(1112.4985495834085f);
//     floats.push_back(0.71828f);
//
//     bool failed = false;
//     uint8_t float_bytes[sizeof(float)];
//     for (auto f: floats) {
//         floatToBytes(f, float_bytes, endian::big);
//         float x = bytesToFloat(float_bytes, endian::big);
//         if (f != x) {
//             failed = true;
//             cout << "failed-1. "<< setprecision(20) << f << ", " << x << endl;
//         }
//
//         floatToBytes(f, float_bytes, endian::little);
//         x = bytesToFloat(float_bytes, endian::little);
//         if (f != x) {
//             failed = true;
//             cout << "failed-2. "<< setprecision(20) << f << ", " << x << endl;
//         }
//     }
//
//     if (!failed)
//         printf("passed.\n");
// }
//
// export TEST_CASE(test_convert_double)
//     vector<double> doubles;
//     doubles.push_back(0);
//     doubles.push_back(23232.716986828);
//     doubles.push_back(1112.4985495834085);
//     doubles.push_back(0.71828);
//     doubles.push_back(3.141592653589793);
//
//     bool failed = false;
//     uint8_t double_bytes[sizeof(double)];
//     for (auto d: doubles) {
//         doubleToBytes(d, double_bytes, endian::big);
//         double x = bytesToDouble(double_bytes, endian::big);
//         if (d != x) {
//             failed = true;
//             cout << "failed-1. "<< setprecision(20) << d << ", " << x << endl;
//         }
//
//         doubleToBytes(d, double_bytes, endian::little);
//         x = bytesToDouble(double_bytes, endian::little);
//         if (d != x) {
//             failed = true;
//             cout << "failed-2. "<< setprecision(20) << d << ", " << x << endl;
//         }
//     }
//
//     if (!failed)
//         printf("passed.\n");
// }
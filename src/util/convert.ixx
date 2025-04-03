module;
#include "../vmdef.h"

export module convert;

import std.core;

//template <typename T>
//T swap_endian(T value) {
//    static_assert(std::is_integral_v<T>, "swap_endian only supports integral types");
//    T result = 0;
//    for (size_t i = 0; i < sizeof(T); ++i) {
//        result |= ( static_cast<T>((((uint8_t*)(&value))[i]) << ((sizeof(T) - 1 - i) * 8)) );
//    }
//    return result;
//}
template <typename T>
T swap_endian(T value) {
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
//uint32_t swap_endian(uint32_t value) {
//    return ((value & 0xFF) << 24) |
//           ((value & 0xFF00) << 8) |
//           ((value & 0xFF0000) >> 8) |
//           ((value & 0xFF000000) >> 24);
//}
//
//// 交换 64 位整数的字节序
//uint64_t swap_endian(uint64_t value) {
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
T bytes_to_int(const uint8_t *bytes, std::endian e) {
    T value = 0;
    std::memcpy(&value, bytes, sizeof(T));
    if (std::endian::native != e) {
        value = swap_endian(value);
    }
//    else {
//        // 进行字节序转换
//        value = swap_endian(value);
////        for (std::size_t i = 0; i < sizeof(T); ++i) {
////            value <<= 8;
////            value |= static_cast<T>(bytes[i]);
////        }
//    }
    return value;
}

export int16_t bytes_to_int16(const uint8_t *bytes, std::endian e) {
    return bytes_to_int<int16_t>(bytes, e);
}

export int32_t bytes_to_int32(const uint8_t *bytes, std::endian e) {
    return bytes_to_int<int32_t>(bytes, e);
}

export int64_t bytes_to_int64(const uint8_t *bytes, std::endian e) {
    return bytes_to_int<int64_t>(bytes, e);
}

// 将整数转换为大端字节数组
template <typename T>
void int_to_bytes(T value, uint8_t *bytes, std::endian e) {
    if (std::endian::native != e) {
        value = swap_endian(value);
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

export void int32_to_bytes(int32_t value, uint8_t *bytes, std::endian e) {
    return int_to_bytes<int32_t>(value, bytes, e);
}

export void int64_to_bytes(int64_t value, uint8_t *bytes, std::endian e) {
    return int_to_bytes<int64_t>(value, bytes, e);
}

// Convert a byte array to a floating-point number (either float or double)
template <typename T>
T bytes_to_floating_point_number(const uint8_t* bytes, std::endian target_endian) {
    static_assert(std::is_floating_point_v<T>, "only supports floating-point types");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType raw_value;
    std::memcpy(&raw_value, bytes, sizeof(T));

    if (std::endian::native != target_endian) {
        raw_value = swap_endian(raw_value);
    }

    return *reinterpret_cast<T*>(&raw_value);
}

export float bytes_to_float(const uint8_t* bytes, std::endian bytes_endian) {
    return bytes_to_floating_point_number<float>(bytes, bytes_endian);
}

export double bytes_to_double(const uint8_t* bytes, std::endian bytes_endian) {
    return bytes_to_floating_point_number<double>(bytes, bytes_endian);
}

// Convert a floating-point number (either float or double) to a byte array
template <typename T>
void floating_point_number_to_bytes(T num, uint8_t* bytes, std::endian bytes_endian) {
    static_assert(std::is_floating_point_v<T>, "only supports floating-point types");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType raw_value = *reinterpret_cast<IntType*>(&num);

    if (std::endian::native != bytes_endian) {
        raw_value = swap_endian(raw_value);
    }

    std::memcpy(bytes, &raw_value, sizeof(T));
}

export void float_to_bytes(float num, uint8_t* bytes, std::endian bytes_endian) {
    floating_point_number_to_bytes<float>(num, bytes, bytes_endian);
}

export void double_to_bytes(double num, uint8_t* bytes, std::endian bytes_endian) {
    floating_point_number_to_bytes<double>(num, bytes, bytes_endian);
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
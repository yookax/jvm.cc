#ifndef CABIN_CONVERT_H
#define CABIN_CONVERT_H

#include <cstdint>
#include <cstring>
#include <span>
#include <bit>
#include "cabin.h"

/*
 * 将字节数组转换为32位整形.
 * 字节数组bytes按大端存储，长度4.
 */
int32_t bytes_to_int32(const uint8_t *bytes);

/*
 * 将字节数组转换为64位整形.
 * 字节数组bytes按大端存储，长度8.
 */
int64_t bytes_to_int64(const uint8_t *bytes);

jfloat int_bits_to_float(jint i);

/*
 * 将字节数组转换为32位浮点数.
 * 字节数组bytes按大端存储，长度4.
 */
jfloat bytes_to_float(const uint8_t *bytes);

jdouble long_bits_to_double(jlong l);

/*
 * 将字节数组转换为64位浮点数.
 * 字节数组bytes按大端存储，长度8.
 */
jdouble bytes_to_double(const uint8_t *bytes);

jint float_to_raw_int_bits(jfloat f);

jlong double_to_raw_long_bits(jdouble d);

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

#define big_endian_bytes_to_int32 big_endian_bytes_to_int<int32_t>
#define big_endian_bytes_to_int64 big_endian_bytes_to_int<int64_t>

// Convert a big-endian byte array to a floating-point number (either float or double)
template <typename T>
static T big_endian_bytes_to_floating_point(const unsigned char bytes[]) {
    static_assert(std::is_floating_point<T>::value, "Template argument must be a floating-point type");
    using IntType = typename std::conditional<sizeof(T) == 4, uint32_t, uint64_t>::type;
    IntType rawValue = 0;
    size_t size = sizeof(T);

    if (std::endian::native == std::endian::big) {
        // If the system is big-endian, just copy the bytes directly
        for (size_t i = 0; i < size; ++i) {
            rawValue |= static_cast<IntType>(bytes[i]) << ((size - 1 - i) * 8);
        }
    } else {
        // If the system is little-endian, reverse the byte order
        for (size_t i = 0; i < size; ++i) {
            rawValue |= static_cast<IntType>(bytes[i]) << (i * 8);
        }
    }
    return *reinterpret_cast<T*>(&rawValue);
}

#define big_endian_bytes_to_float big_endian_bytes_to_floating_point<float>
#define big_endian_bytes_to_double big_endian_bytes_to_floating_point<double>

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

#endif // CABIN_CONVERT_H
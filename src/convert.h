#ifndef CABIN_CONVERT_H
#define CABIN_CONVERT_H

#include <cstdint>
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

#endif // CABIN_CONVERT_H
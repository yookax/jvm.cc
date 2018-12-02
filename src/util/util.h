/*
 * Author: Jia Yang
 */

#ifndef JVM_UTIL_H
#define JVM_UTIL_H

#include <stdbool.h>

/*
 * 判断 long_str 是不是以 short_str 结尾。
 */
bool strend(const char *long_str, const char *short_str);

/*
 * 将字符串 @str 中所有的字符 @s 替换成 @d
 */
void strreplace(char str[static 1], char s, char d);

#endif //JVM_UTIL_H

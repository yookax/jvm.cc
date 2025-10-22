export module vmstd;

import std;

using namespace std;

/*
 * Java虚拟机中的整型类型的取值范围如下：
 * 1. 对于byte类型， 取值范围[-2e7, 2e7 - 1]。
 * 2. 对于short类型，取值范围[-2e15, 2e15 - 1]。
 * 3. 对于int类型，  取值范围[-2e31, 2e31 - 1]。
 * 4. 对于long类型， 取值范围[-2e63, 2e63 - 1]。
 * 5. 对于char类型， 取值范围[0, 65535]。
 */
// export using jbyte    = int8_t;
// export using jboolean = jbyte; // 本虚拟机实现，byte 和 boolean 用同一类型
// export using jbool    = jboolean;
// export using jchar    = uint16_t;
// export using jshort   = int16_t;
// export using jint     = int32_t;
// export using jlong    = int64_t;
// export using jfloat   = float;
// export using jdouble  = double;
//
// export using jsize = jint;
//
// export jbool jtrue = 1;
// export jbool jfalse = 0;
//
// export jbool jint2jbool(jint i) { return i != 0 ? jtrue : jfalse; }
// export jbool jint2jbyte(jint i) { return (jbyte)(i & 0xff); }
// export jbool jint2jchar(jint i) { return (jchar)(i & 0xffff); }
// export jbool jint2jshort(jint i) { return (jshort)(i & 0xffff); }
//
// // s: signed
// export using s1 = int8_t;
// export using s2 = int16_t;
// export using s4 = int32_t;
//
// // u: unsigned
// export using u1 = uint8_t;
// export using u2 = uint16_t;
// export using u4 = uint32_t;
// export using u8 = uint64_t;
//
// class Object;
//
// export using jref       = Object*; // JVM 中的引用类型
// export using jstrRef    = jref;    // java.lang.String 的引用
// export using jarrRef    = jref;    // Array 的引用
// export using jobjArrRef = jref;    // java.lang.Object Array 的引用
// export using jclsRef    = jref;    // java.lang.Class 的引用
//
// export using utf8_t = char;
// export using unicode_t = jchar;

// export template <typename T> concept
// JavaValueType = std::is_same_v<T, jint>
//                 || std::is_same_v<T, jbyte> || std::is_same_v<T, jbool>
//                 || std::is_same_v<T, jchar> || std::is_same_v<T, jshort>
//                 || std::is_same_v<T, jfloat> || std::is_same_v<T, jlong>
//                 || std::is_same_v<T, jdouble> || std::is_same_v<T, jref>;

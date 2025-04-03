module;
#include <cassert>
#include "../vmdef.h"

export module bytes_reader;

import std.core;
import convert;

export class BytesReader {
    const u1 *bytes;
    size_t len; // bytes len
    std::endian endian; // bytes endian

    size_t saved_pc = 0;
public:
    size_t pc = 0; // program count

    void save_pc() { saved_pc = pc; }
    void recover_pc() { pc = saved_pc; }

    BytesReader(const u1 *bytes0, size_t len0, std::endian e): bytes(bytes0), len(len0), endian(e) {
        assert(bytes != nullptr);
    }

    const u1 *curr_pos() const { return bytes + pc; }

    bool has_more() const { return pc < len; }

    void skip(int offset) {
        pc += offset;
        assert(pc <= len);
    }

    /*
    * todo 函数干什么用的
    */
    void align4() {
        while (pc % 4 != 0) {
            pc++;
        }
        assert(pc < len);
    }

    void read_bytes(u1 *buf, size_t _len) {
        assert(buf != nullptr);

        memcpy(buf, bytes + pc, _len);
        pc += _len;
        assert(pc < len);
    }

    u1 peeku1() {
        assert(pc < len);
        return bytes[pc];
    }

    s1 reads1() {
        assert(pc < len);
        return bytes[pc++];
    }

    u1 readu1() {
        assert(pc < len);
        return (u1) bytes[pc++];
    }

    u2 readu2() {
        assert(pc < len);
        u1 buf[2];
        read_bytes(buf, 2);
        return (u4) bytes_to_int16(buf, endian);
    }

    u2 peeku2() {
        assert(pc < len);
        u2 data = readu2();
        pc -= 2;
        return data;
    }

    s2 reads2() {
        assert(pc < len);
        u1 buf[2];
        read_bytes(buf, 2);
        return bytes_to_int16(buf, endian);
    }

    u4 readu4() {
        assert(pc < len);
        u1 buf[4];
        read_bytes(buf, 4);
        return (u4) bytes_to_int32(buf, endian);  // should be bytesToUint32  todo
    }

    s4 reads4() {
        assert(pc < len);
        u1 buf[4];
        read_bytes(buf, 4);
        return (s4) bytes_to_int32(buf, endian);
    }

    u8 readu8() {
        assert(pc < len);
        u1 buf[8];
        read_bytes(buf, 8);
        return (u8) bytes_to_int64(buf, endian);

//        const u1 *p = bytes;
//        u8 v = ((u8)(p)[0]<<56)
//               |((u8)(p)[1]<<48)
//               |((u8)(p)[2]<<40)
//               |((u8)(p)[3]<<32)
//               |((u8)(p)[4]<<24)
//               |((u8)(p)[5]<<16)
//               |((u8)(p)[6]<<8)
//               |(u8)(p)[7];
//        pc += 8;
//        return v;
    }

    /*
     * 读 @n 个s4数据到 @s4s 数组中
     */
    void reads4s(int n, s4 *s4s) {
        assert(pc < len);
        for (int i = 0; i < n; i++) {
            u1 buf[4];
            read_bytes(buf, 4);
            s4s[i] = (s4) bytes_to_int32(buf, endian);
        }
    }
};
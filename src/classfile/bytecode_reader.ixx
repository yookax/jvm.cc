module;
#include <cassert>
#include "../cabin.h"

export module bytecode_reader;

import vmstd;

export class BytecodeReader {
    const u1 *bytecode;
    size_t len; // bytecode len

    size_t saved_pc = 0;

public:
    size_t pc = 0; // program count

    void savePC() { saved_pc = pc; }
    void recoverPC() { pc = saved_pc; }

    BytecodeReader(const u1 *bytecode0, size_t len0): bytecode(bytecode0), len(len0) {
        assert(bytecode != nullptr);
    }

    const u1 *curr_pos() const { return bytecode + pc; }

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

        memcpy(buf, bytecode + pc, _len);
        pc += _len;
    }

    u1 peeku1() {
        assert(pc < len);
        return bytecode[pc];
    }

    s1 reads1() {
        assert(pc < len);
        return bytecode[pc++];
    }

    u1 readu1() {
        assert(pc < len);
        return (u1) bytecode[pc++];
    }

    u2 readu2() {
        assert(pc < len);
        u2 x = readu1();
        u2 y = (u2) (readu1() & 0x00ff);

        return x << 8 | y;
    }

    u2 peeku2() {
        assert(pc < len);
        u2 data = readu2();
        pc -= 2;
        return data;
    }

    s2 reads2() {
        assert(pc < len);
        return readu2();
    }

    u4 readu4() {
        assert(pc < len);
        u1 buf[4];
        read_bytes(buf, 4);

        return (u4) bytes_to_int32(buf);  // should be bytesToUint32  todo
    }

    u8 readu8() {
        assert(pc < len);
        const u1 *p = bytecode;
        u8 v = ((u8)(p)[0]<<56)
               |((u8)(p)[1]<<48)
               |((u8)(p)[2]<<40)
               |((u8)(p)[3]<<32)
               |((u8)(p)[4]<<24)
               |((u8)(p)[5]<<16)
               |((u8)(p)[6]<<8)
               |(u8)(p)[7];
        pc += 8;
        return v;
    }

    s4 reads4() {
        assert(pc < len);
        u1 buf[4];
        read_bytes(buf, 4);

        return (s4) bytes_to_int32(buf);
    }

    /*
     * 读 @n 个s4数据到 @s4s 数组中
     */
    void reads4s(int n, s4 *s4s) {
        assert(pc < len);
        for (int i = 0; i < n; i++) {
            u1 buf[4];
            read_bytes(buf, 4);

            s4s[i] = (s4) bytes_to_int32(buf);
        }
    }
};

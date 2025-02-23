#include <cmath>
#include <sstream>
//#include <ffi.h>
#include "cabin.h"
#include "encoding.h"
#include "classfile/bytecode_reader.h"
#include "classfile/constants.h"
#include "classfile/class.h"
#include "classfile/array_class.h"
#include "classfile/method.h"
#include "interpreter.h"
#include "object/allocator.h"

#include <functional>

#include "jni.h"
#include "runtime/frame.h"
#include "runtime/thread.h"
#include "classfile/invoke.h"
#include "object/object.h"
#include "exception.h"
#include "dll.h"

using namespace std;
using namespace slot;
using namespace utf8;

#if (LOG_LEVEL >= LOG_LEVEL_VERBOSE)
// The mapping of instructions' code and name
static const char *instruction_names[] = JVM_OPCODE_NAME_INITIALIZER;
#endif

#define PRINT_OPCODE VERBOSE("%d(0x%x), %s, pc = %d", \
                        opcode, opcode, instruction_names[opcode], (int)frame->reader.pc);

static const u1 opcode_len[JVM_OPC_MAX + 1] = JVM_OPCODE_LENGTH_INITIALIZER;

static void call_native_method(Frame *frame);

/*
 * 得到当前指令前面最近一条 aload 指令的 index 值。
 * 失败返回小于0
 */
static int get_prev_aload_opc_index(BytecodeReader &r) {
    // 五种指令（aload, aload_0, aload_1, aload_2, aload_3）
    // 其中（aload_0, aload_1, aload_2, aload_3）指令长度为1，
    // aload 指令长度为2，如果 aload 前面有 wide指令，这aload 指令长度为3

    r.savePC();
    
    int index_in_lvars = -1;
    auto curr = r.pc;
    for (r.pc = 0; r.pc < curr;) {
        u1 opc = r.readu1();
        if (opc == JVM_OPC_aload_0) {
            index_in_lvars = 0; 
        } else if (opc == JVM_OPC_aload_1) {
            index_in_lvars = 1; 
        } else if (opc == JVM_OPC_aload_2) { 
            index_in_lvars = 2; 
        } else if (opc == JVM_OPC_aload_3) { 
            index_in_lvars = 3; 
        } else if (opc == JVM_OPC_aload) {
            index_in_lvars = r.readu1();
        } else if (opc == JVM_OPC_wide) {
            opc = r.readu1();
            u2 index = r.readu2();
            if (opc == JVM_OPC_aload) {
                index_in_lvars = index;
            }
        } else {
            // 当前指令不是 aload 指令镞，跳过
            // 减一是因为前面已经读取了一个字节
            r.skip(opcode_len[opc]-1);
        }
    }
    
    r.recoverPC(); 
    return index_in_lvars;
}

// 得到当前指令正在操作的对象名
static const utf8_t *get_opc_operating_obj_name(Frame *frame, int opc) {
    // 读取对象属性指令，这类指令的特点是其前面必有一个 aload 指令镞来 load 需要读取的对象，
    // 但此load指令不一定是其前一个指令，中间可能夹杂了几个指令。
    // 但肯定是距离当前位置前面最近的一个load指令
    static const int read_obj_opcs[] = {
        JVM_OPC_getfield, JVM_OPC_arraylength, JVM_OPC_monitorenter, JVM_OPC_monitorexit,
        JVM_OPC_iaload, JVM_OPC_laload, JVM_OPC_faload, JVM_OPC_daload, 
        JVM_OPC_aaload, JVM_OPC_baload, JVM_OPC_caload, JVM_OPC_saload,
    };

    for (int c : read_obj_opcs) {
        if (c == opc) {
            ostringstream oss;
        
            int index_in_lvars = get_prev_aload_opc_index(frame->reader);
            if (index_in_lvars >= 0) {
                auto [name, descriptor] = frame->method->find_local_variable(
                        frame->reader.pc - opcode_len[opc], (u2) index_in_lvars);
                return name;
            }        
            
            return ""; // not find
        }
    }

    // todo 其他指令
    return "";
}

/*
 * 执行当前线程栈顶的frame
 * 特殊的，如果异常对象 `excep` 不为空，则处理异常。
 */
static slot_t *exec(jref &excep) {
    Thread *thread = get_current_thread();
    Frame *frame = thread->top_frame;
    TRACE("executing frame: %s", frame->toString().c_str());

    Method *resolved_method = nullptr;
    int index;

    int ret_value_slots_count;

    BytecodeReader *reader = &frame->reader;
    Class *clazz = frame->method->clazz;
    ConstantPool *cp = frame->method->clazz->cp;
    // slot_t *ostack = frame->ostack;
    slot_t *lvars = frame->lvars;

    jref _this = frame->method->isStatic() ? (jref) clazz : slot::get<jref>(lvars);

#define NULL_POINTER_CHECK(ref) \
do { \
    if ((ref) == nullptr) \
        throw java_lang_NullPointerException(FILE_LINE_STR); \
} while(false) 

#define CHANGE_FRAME(new_frame) \
do { \
    /*frame->ostack = ostack;  stack指针在变动，需要设置一下 todo */ \
    frame = new_frame; \
    reader = &frame->reader; \
    clazz = frame->method->clazz; \
    cp = frame->method->clazz->cp; \
    /*ostack = frame->ostack; */ \
    lvars = frame->lvars; \
    _this = frame->method->isStatic() ? (jref) clazz : slot::get<jref>(lvars); \
    TRACE("executing frame: %s", frame->toString().c_str()); \
} while(false)

    if (excep != nullptr) {
        frame->pushr(excep);
        excep = nullptr;
        goto opc_athrow;
    }

    u1 opcode;
    while (true) {
        opcode = reader->readu1(); 
        PRINT_OPCODE

        switch (opcode) {
            case JVM_OPC_nop: break;
            case JVM_OPC_aconst_null: frame->pushr(nullptr); break;
            case JVM_OPC_iconst_m1: frame->pushi(-1); break;
            case JVM_OPC_iconst_0: frame->pushi(0); break;
            case JVM_OPC_iconst_1: frame->pushi(1); break;
            case JVM_OPC_iconst_2: frame->pushi(2); break;
            case JVM_OPC_iconst_3: frame->pushi(3); break;
            case JVM_OPC_iconst_4: frame->pushi(4); break;
            case JVM_OPC_iconst_5: frame->pushi(5); break;
            case JVM_OPC_lconst_0: frame->pushl(0); break;
            case JVM_OPC_lconst_1: frame->pushl(1); break;
            case JVM_OPC_fconst_0: frame->pushf(0); break;
            case JVM_OPC_fconst_1: frame->pushf(1); break;
            case JVM_OPC_fconst_2: frame->pushf(2); break;
            case JVM_OPC_dconst_0: frame->pushd(0); break;
            case JVM_OPC_dconst_1: frame->pushd(1); break;
            case JVM_OPC_bipush: // Byte Integer push
                frame->pushi(reader->reads1());
                break;
            case JVM_OPC_sipush: // Short Integer push
                frame->pushi(reader->reads2());
                break;
            case JVM_OPC_ldc:
                index = reader->readu1();
                goto _ldc;
            case JVM_OPC_ldc_w:
                index = reader->readu2();
_ldc:
                switch (u1 type = cp->get_type(index)) {
                    case JVM_CONSTANT_Integer:
                        frame->pushi(cp->get_int(index));
                        break;
                    case JVM_CONSTANT_Float:
                        frame->pushf(cp->get_float(index));
                        break;
                    case JVM_CONSTANT_String:
                    case JVM_CONSTANT_ResolvedString:
                        frame->pushr(cp->resolve_string(index));
                    break;
                    case JVM_CONSTANT_Class:
                    case JVM_CONSTANT_ResolvedClass:
                        frame->pushr(cp->resolve_class(index)->java_mirror);
                        break;
                    default:
                        UNREACHABLE("unknown type: %d", type);
                        break;
                }

                break;
            case JVM_OPC_ldc2_w:
                index = reader->readu2();
                switch (u1 type = cp->get_type(index)) {
                    case JVM_CONSTANT_Long:
                        frame->pushl(cp->get_long(index));
                        break;
                    case JVM_CONSTANT_Double:
                        frame->pushd(cp->get_double(index));
                        break;
                    default:
                        UNREACHABLE("unknown type: %d", type);
                        break;
                }

                break;
            case JVM_OPC_iload:
            case JVM_OPC_fload:
            case JVM_OPC_aload:
                index = reader->readu1();
            _iload:
            _fload:
            _aload:
                *frame->ostack++ = lvars[index];
                break;
            case JVM_OPC_lload:
            case JVM_OPC_dload:
                index = reader->readu1();
            _lload:
            _dload:
                *frame->ostack++ = lvars[index];
                *frame->ostack++ = lvars[index + 1];
                break;
            case JVM_OPC_iload_0:
            case JVM_OPC_fload_0:
            case JVM_OPC_aload_0:
                *frame->ostack++ = lvars[0];
                break;
            case JVM_OPC_iload_1:
            case JVM_OPC_fload_1:
            case JVM_OPC_aload_1:
                *frame->ostack++ = lvars[1];
                break;
            case JVM_OPC_iload_2:
            case JVM_OPC_fload_2:
            case JVM_OPC_aload_2:
                *frame->ostack++ = lvars[2];
                break;
            case JVM_OPC_iload_3:
            case JVM_OPC_fload_3:
            case JVM_OPC_aload_3:
                *frame->ostack++ = lvars[3];
                break;
            case JVM_OPC_lload_0:
            case JVM_OPC_dload_0:
                *frame->ostack++ = lvars[0];
                *frame->ostack++ = lvars[1];
                break;
            case JVM_OPC_lload_1:
            case JVM_OPC_dload_1:
                *frame->ostack++ = lvars[1];
                *frame->ostack++ = lvars[2];
                break;
            case JVM_OPC_lload_2:
            case JVM_OPC_dload_2:
                *frame->ostack++ = lvars[2];
                *frame->ostack++ = lvars[3];
                break;
            case JVM_OPC_lload_3:
            case JVM_OPC_dload_3:
                *frame->ostack++ = lvars[3];
                *frame->ostack++ = lvars[4];
                break;

#define GET_AND_CHECK_ARRAY \
    index = frame->popi(); \
    jarrRef arr = frame->popr(); \
    NULL_POINTER_CHECK(arr); \
    if (!arr->check_bounds(index)) \
       throw java_lang_ArrayIndexOutOfBoundsException("index is " + to_string(index));

            case JVM_OPC_iaload: {
                GET_AND_CHECK_ARRAY
                jint value = arr->getElt<jint>(index);
                frame->pushi(value);
                break;
            }
            case JVM_OPC_faload: {
                GET_AND_CHECK_ARRAY
                auto value = arr->getElt<jfloat>(index);
                frame->pushf(value);
                break;
            }
            case JVM_OPC_aaload: {
                GET_AND_CHECK_ARRAY
                jref value = arr->getElt<jref>(index);
                frame->pushr(value);
                break;
            }
            case JVM_OPC_baload: {
                GET_AND_CHECK_ARRAY
                jint value = arr->getElt<jbyte>(index);
                frame->pushi(value);
                break;
            }
            case JVM_OPC_caload: {
                GET_AND_CHECK_ARRAY
                jint value = arr->getElt<jchar>(index);
                frame->pushi(value);
                break;
            }
            case JVM_OPC_saload: {
                GET_AND_CHECK_ARRAY
                jint value = arr->getElt<jshort>(index);
                frame->pushi(value);
                break;
            }
            case JVM_OPC_laload: {
                GET_AND_CHECK_ARRAY
                auto value = arr->getElt<jlong>(index);
                frame->pushl(value);
                break;
            }
            case JVM_OPC_daload: {
                GET_AND_CHECK_ARRAY
                auto value = arr->getElt<jdouble>(index);
                frame->pushd(value);
                break;
            }
            case JVM_OPC_istore:
            case JVM_OPC_fstore:
            case JVM_OPC_astore:
                index = reader->readu1();
_istore:
_fstore:
_astore:
                lvars[index] = *--frame->ostack;
                break;
            case JVM_OPC_lstore:
            case JVM_OPC_dstore:
                index = reader->readu1();
_lstore:
_dstore: 
                lvars[index + 1] = *--frame->ostack;
                lvars[index] = *--frame->ostack;
                break;
            case JVM_OPC_istore_0:
            case JVM_OPC_fstore_0:
            case JVM_OPC_astore_0:
                lvars[0] = *--frame->ostack;
                break;
            case JVM_OPC_istore_1:
            case JVM_OPC_fstore_1:
            case JVM_OPC_astore_1:
                lvars[1] = *--frame->ostack;
                break;
            case JVM_OPC_istore_2:
            case JVM_OPC_fstore_2:
            case JVM_OPC_astore_2:
                lvars[2] = *--frame->ostack;
                break;
            case JVM_OPC_istore_3:
            case JVM_OPC_fstore_3:
            case JVM_OPC_astore_3:
                lvars[3] = *--frame->ostack;
                break;
            case JVM_OPC_lstore_0:
            case JVM_OPC_dstore_0:
                lvars[1] = *--frame->ostack;
                lvars[0] = *--frame->ostack;
                break;
            case JVM_OPC_lstore_1:
            case JVM_OPC_dstore_1:
                lvars[2] = *--frame->ostack;
                lvars[1] = *--frame->ostack;
                break;
            case JVM_OPC_lstore_2:
            case JVM_OPC_dstore_2:
                lvars[3] = *--frame->ostack;
                lvars[2] = *--frame->ostack;
                break;
            case JVM_OPC_lstore_3:
            case JVM_OPC_dstore_3:
                lvars[4] = *--frame->ostack;
                lvars[3] = *--frame->ostack;
                break;
            case JVM_OPC_iastore: {
                jint value = frame->popi();
                GET_AND_CHECK_ARRAY
                arr->setIntElt(index, value);
                break;
            }
            case JVM_OPC_fastore: {
                jfloat value = frame->popf();
                GET_AND_CHECK_ARRAY
                arr->setFloatElt(index, value);
                break;
            }
            case JVM_OPC_aastore: {
                jref value = frame->popr();
                GET_AND_CHECK_ARRAY
                arr->setRefElt(index, value);
                break;
            }
            case JVM_OPC_bastore: {
                jint value = frame->popi();
                GET_AND_CHECK_ARRAY
                if (arr->clazz->is_byte_array_class()) {
                    arr->setByteElt(index, JINT_TO_JBYTE(value));
                } else if (arr->clazz->is_bool_array_class()) {
                    arr->setBoolElt(index, JINT_TO_JBOOL(value));
                } else {
                    UNREACHABLE(arr->clazz->name);
                }
                break;
            }
            case JVM_OPC_castore: {
                jint value = frame->popi();
                GET_AND_CHECK_ARRAY
                arr->setCharElt(index, JINT_TO_JCHAR(value));
                break;
            }
            case JVM_OPC_sastore: {
                jint value = frame->popi();
                GET_AND_CHECK_ARRAY
                arr->setShortElt(index, JINT_TO_JSHORT(value));
                break;
            }
            case JVM_OPC_lastore: {
                jlong value = frame->popl();
                GET_AND_CHECK_ARRAY
                arr->setLongElt(index, value);
                break;
            }
            case JVM_OPC_dastore: {
                jdouble value = frame->popd();
                GET_AND_CHECK_ARRAY
                arr->setDoubleElt(index, value);
                break;
            }
#undef GET_AND_CHECK_ARRAY

            case JVM_OPC_pop: frame->ostack--; break;
            case JVM_OPC_pop2: frame->ostack -= 2; break;
            case JVM_OPC_dup:
                frame->ostack[0] = frame->ostack[-1];
                frame->ostack++;
                break;
            case JVM_OPC_dup_x1:
                frame->ostack[0] = frame->ostack[-1];
                frame->ostack[-1] = frame->ostack[-2];
                frame->ostack[-2] = frame->ostack[0];
                frame->ostack++;
                break;
            case JVM_OPC_dup_x2:
                frame->ostack[0] = frame->ostack[-1];
                frame->ostack[-1] = frame->ostack[-2];
                frame->ostack[-2] = frame->ostack[-3];
                frame->ostack[-3] = frame->ostack[0];
                frame->ostack++;
                break;
            case JVM_OPC_dup2:
                frame->ostack[0] = frame->ostack[-2];
                frame->ostack[1] = frame->ostack[-1];
                frame->ostack += 2;
                break;
            case JVM_OPC_dup2_x1:
                // ..., value3, value2, value1 →
                // ..., value2, value1, value3, value2, value1
                frame->ostack[1] = frame->ostack[-1];
                frame->ostack[0] = frame->ostack[-2];
                frame->ostack[-1] = frame->ostack[-3];
                frame->ostack[-2] = frame->ostack[1];
                frame->ostack[-3] = frame->ostack[0];
                frame->ostack += 2;
                break;
            case JVM_OPC_dup2_x2:
                // ..., value4, value3, value2, value1 →
                // ..., value2, value1, value4, value3, value2, value1
                frame->ostack[1] = frame->ostack[-1];
                frame->ostack[0] = frame->ostack[-2];
                frame->ostack[-1] = frame->ostack[-3];
                frame->ostack[-2] = frame->ostack[-4];
                frame->ostack[-3] = frame->ostack[1];
                frame->ostack[-4] = frame->ostack[0];
                frame->ostack += 2;
                break;
            case JVM_OPC_swap: {
                slot_t tmp = frame->ostack[-1];
                frame->ostack[-1] = frame->ostack[-2];
                frame->ostack[-2] = tmp;
                // swap(frame->ostack[-1], frame->ostack[-2]);
                break;
            }

#define BINARY_OP(type, t, oper) \
{ \
    type v2 = frame->pop##t(); \
    type v1 = frame->pop##t(); \
    frame->push##t(v1 oper v2); \
    break; \
}

            case JVM_OPC_iadd: BINARY_OP(jint, i, +);
            case JVM_OPC_ladd: BINARY_OP(jlong, l, +);
            case JVM_OPC_fadd: BINARY_OP(jfloat, f, +);
            case JVM_OPC_dadd: BINARY_OP(jdouble, d, +);
            case JVM_OPC_isub: BINARY_OP(jint, i, -);
            case JVM_OPC_lsub: BINARY_OP(jlong, l, -);
            case JVM_OPC_fsub: BINARY_OP(jfloat, f, -);
            case JVM_OPC_dsub: BINARY_OP(jdouble, d, -);
            case JVM_OPC_imul:
                //BINARY_OP(jint, i, *);
                frame->ostack--;
                slot::set<jint>(frame->ostack-1, slot::get<jint>(frame->ostack-1) * slot::get<jint>(frame->ostack));
                break;
            case JVM_OPC_lmul: BINARY_OP(jlong, l, *);
            case JVM_OPC_fmul: BINARY_OP(jfloat, f, *);
            case JVM_OPC_dmul: BINARY_OP(jdouble, d, *);

#define ZERO_DIVISOR_CHECK(value) \
do { \
    if (value == 0) \
        throw java_lang_ArithmeticException("division by zero"); \
} while(false)

            case JVM_OPC_idiv:
                ZERO_DIVISOR_CHECK(slot::get<jint>(frame->ostack - 1));
                BINARY_OP(jint, i, /);
            case JVM_OPC_ldiv:
                ZERO_DIVISOR_CHECK(slot::get<jlong>(frame->ostack - 2));
                BINARY_OP(jlong, l, /);
            case JVM_OPC_fdiv:
                ZERO_DIVISOR_CHECK(slot::get<jfloat>(frame->ostack - 1));
                BINARY_OP(jfloat, f, /);
            case JVM_OPC_ddiv:
                ZERO_DIVISOR_CHECK(slot::get<jdouble>(frame->ostack - 2));
                BINARY_OP(jdouble, d, /);
            case JVM_OPC_irem:
                // ZERO_DIVISOR_CHECK(slot::get<jint>(frame->ostack - 1));
                // BINARY_OP(jint, i, %);
                frame->ostack--;
                ZERO_DIVISOR_CHECK(slot::get<jint>(frame->ostack));
                slot::set<jint>(frame->ostack-1,
                    slot::get<jint>(frame->ostack-1) % slot::get<jint>(frame->ostack));
                break;
            case JVM_OPC_lrem:
                ZERO_DIVISOR_CHECK(slot::get<jlong>(frame->ostack - 2));
                BINARY_OP(jlong, l, %);
#undef ZERO_DIVISOR_CHECK

            case JVM_OPC_frem: {
                jfloat v2 = frame->popf();
                jfloat v1 = frame->popf();
                frame->pushf(fmod(v1, v2));
                break;
            }
            case JVM_OPC_drem: {
                jdouble v2 = frame->popd();
                jdouble v1 = frame->popd();
                frame->pushd(fmod(v1, v2));
                break;
            }
            case JVM_OPC_ineg:
                frame->pushi(-frame->popi());
                break;
            case JVM_OPC_lneg:
                frame->pushl(-frame->popl());
                break;
            case JVM_OPC_fneg:
                frame->pushf(-frame->popf());
                break;
            case JVM_OPC_dneg:
                frame->pushd(-frame->popd());
                break;
            case JVM_OPC_ishl: {
                // 与0x1f是因为低5位表示位移距离，位移距离实际上被限制在0到31之间。
                jint shift = frame->popi() & 0x1f;
                assert(0 <= shift && shift <= 31);
                jint x = frame->popi();
                frame->pushi(x << shift);
                break;
            }
            case JVM_OPC_lshl: {
                // 与0x3f是因为低6位表示位移距离，位移距离实际上被限制在0到63之间。
                jint shift = frame->popi() & 0x3f;
                assert(0 <= shift && shift <= 63);
                jlong x = frame->popl();
                frame->pushl(x << shift);
                break;
            }
            case JVM_OPC_ishr: {
                // 算术右移 shift arithmetic right
                // 带符号右移。正数右移高位补0，负数右移高位补1。
                // 对应于Java中的 >>
                jint shift = frame->popi() & 0x1f;
                assert(0 <= shift && shift <= 31);
                jint x = frame->popi();
                frame->pushi(x >> shift);
                break;
            }
            case JVM_OPC_lshr: {
                jint shift = frame->popi() & 0x3f;
                assert(0 <= shift && shift <= 63);
                jlong x = frame->popl();
                frame->pushl(x >> shift);
                break;
            }
            case JVM_OPC_iushr: {
                // 逻辑右移 shift logical right
                // 无符号右移。无论是正数还是负数，高位通通补0。
                // 对应于Java中的 >>>
                // https://stackoverflow.com/questions/5253194/implementing-logical-right-shift-in-c/
                jint shift = frame->popi() & 0x1f;
                assert(0 <= shift && shift <= 31);
                jint x = frame->popi();
                int size = sizeof(jint) * 8 - 1; // bits count
                frame->pushi((x >> shift) & ~(((((jint)1) << size) >> shift) << 1));
                break;
            }
            case JVM_OPC_lushr: {
                jint shift = frame->popi() & 0x3f;
                assert(0 <= shift && shift <= 63);
                jlong x = frame->popl();
                int size = sizeof(jlong) * 8 - 1; // bits count
                frame->pushl((x >> shift) & ~(((((jlong)1) << size) >> shift) << 1));
                break;
            }
            case JVM_OPC_iand: BINARY_OP(jint,  i, &);
            case JVM_OPC_land: BINARY_OP(jlong, l, &);
            case JVM_OPC_ior:  BINARY_OP(jint,  i, |);
            case JVM_OPC_lor:  BINARY_OP(jlong, l, |);
            case JVM_OPC_ixor: BINARY_OP(jint,  i, ^);
            case JVM_OPC_lxor: BINARY_OP(jlong, l, ^);
#undef BINARY_OP

            case JVM_OPC_iinc:
                index = reader->readu1();
                slot::set<jint>(lvars + index, slot::get<jint>(lvars + index) + reader->reads1());
                break;
            _wide_iinc:
                slot::set<jint>(lvars + index, slot::get<jint>(lvars + index) + reader->reads2());
                break;
            case JVM_OPC_i2l:
                frame->pushl(frame->popi());
                break;
            case JVM_OPC_i2f:
                frame->pushf(frame->popi());
                break;
            case JVM_OPC_i2d:
                frame->pushd(frame->popi());
                break;
            case JVM_OPC_l2i:
                frame->pushi((jint) frame->popl());
                break;
            case JVM_OPC_l2f:
                frame->pushf((jfloat) frame->popl());
                break;
            case JVM_OPC_l2d:
                frame->pushd(frame->popl());
                break;
            case JVM_OPC_f2i:
               frame->pushi((jint) frame->popf());
                break;
            case JVM_OPC_f2l:
                frame->pushl((jlong) frame->popf());
                break;
            case JVM_OPC_f2d:
                frame->pushd(frame->popf());
                break;
            case JVM_OPC_d2i:
                frame->pushi((jint) frame->popd());
                break;
            case JVM_OPC_d2l:
                frame->pushl((jlong) frame->popd());
                break;
            case JVM_OPC_d2f:
                frame->pushf((jfloat) frame->popd());
                break;
            case JVM_OPC_i2b:
                frame->pushi(JINT_TO_JBYTE(frame->popi()));
                break;
            case JVM_OPC_i2c:
                frame->pushi(JINT_TO_JCHAR(frame->popi()));
                break;
            case JVM_OPC_i2s:
                frame->pushi(JINT_TO_JSHORT(frame->popi()));
                break;
    
/*
 * NAN 与正常的的浮点数无法比较，即 即不大于 也不小于 也不等于。
 * 两个 NAN 之间也无法比较，即 即不大于 也不小于 也不等于。
 */
#define DO_CMP(v1, v2, default_value) \
            (jint)((v1) > (v2) ? 1 : ((v1) == (v2) ? 0 : ((v1) < (v2) ? -1 : (default_value))))

#define CMP(type, t, cmp_result) \
{ \
    type v2 = frame->pop##t(); \
    type v1 = frame->pop##t(); \
    frame->pushi(cmp_result); \
    break; \
}
            case JVM_OPC_lcmp:  CMP(jlong,   l, DO_CMP(v1, v2, -1))
            case JVM_OPC_fcmpl: CMP(jfloat,  f, DO_CMP(v1, v2, -1))
            case JVM_OPC_fcmpg: CMP(jfloat,  f, DO_CMP(v1, v2,  1))
            case JVM_OPC_dcmpl: CMP(jdouble, d, DO_CMP(v1, v2, -1))
            case JVM_OPC_dcmpg: CMP(jdouble, d, DO_CMP(v1, v2,  1))
#undef CMP
#undef DO_CMP

#define IF_COND(cond, opc) \
{ \
    jint offset = reader->reads2(); \
    if (slot::get<jint>(--frame->ostack) cond 0) \
        reader->skip(offset - opcode_len[opc]); \
    break; \
}
            case JVM_OPC_ifeq: IF_COND(==, JVM_OPC_ifeq)
            case JVM_OPC_ifne: IF_COND(!=, JVM_OPC_ifne)
            case JVM_OPC_iflt: IF_COND(<, JVM_OPC_iflt)
            case JVM_OPC_ifge: IF_COND(>=, JVM_OPC_ifge)
            case JVM_OPC_ifgt: IF_COND(>, JVM_OPC_ifgt)
            case JVM_OPC_ifle: IF_COND(<=, JVM_OPC_ifle)
#undef IF_COND

#define IF_CMP_COND(type, t, cond, opc) \
{ \
    s2 offset = reader->reads2(); \
    type v2 = frame->pop##t(); \
    type v1 = frame->pop##t(); \
    if (v1 cond v2) \
        reader->skip(offset - opcode_len[opc]); \
    break; \
}
            case JVM_OPC_if_icmpeq: IF_CMP_COND(jint, i, ==, JVM_OPC_if_icmpeq)
            case JVM_OPC_if_acmpeq: IF_CMP_COND(jref, r, ==, JVM_OPC_if_acmpeq)
            case JVM_OPC_if_icmpne: IF_CMP_COND(jint, i, !=, JVM_OPC_if_icmpne)
            case JVM_OPC_if_acmpne: IF_CMP_COND(jref, r, !=, JVM_OPC_if_acmpne)
            case JVM_OPC_if_icmplt: IF_CMP_COND(jint, i, <,  JVM_OPC_if_icmplt)
            case JVM_OPC_if_icmpge: IF_CMP_COND(jint, i, >=, JVM_OPC_if_icmpge)
            case JVM_OPC_if_icmpgt: IF_CMP_COND(jint, i, >,  JVM_OPC_if_icmpgt)
            case JVM_OPC_if_icmple: {
                // IF_CMP_COND(jint, i, <=, JVM_OPC_if_icmple)
                s2 offset = reader->reads2();
                frame->ostack -= 2;
                if (slot::get<jint>(frame->ostack) <= slot::get<jint>(frame->ostack + 1))
                    reader->skip(offset - opcode_len[JVM_OPC_if_icmple]);
                break;
            }
#undef IF_CMP_COND

            case JVM_OPC_goto: {
                s2 offset = reader->reads2();
                reader->skip(offset - opcode_len[JVM_OPC_goto]);
                break;
            }

            // 在Java 6之前，Oracle的Java编译器使用 jsr, jsr_w 和 ret 指令来实现 finally 子句。
            // 从Java 6开始，已经不再使用这些指令
            case JVM_OPC_jsr:
                UNREACHABLE("jsr doesn't support after jdk 6.");
            case JVM_OPC_ret:
opc_ret:
                UNREACHABLE("ret doesn't support after jdk 6.");
            case JVM_OPC_tableswitch: {
                // 实现当各个case值跨度比较小时的 switch 语句
                size_t saved_pc = reader->pc - 1; // save the pc before 'tableswitch' instruction
                reader->align4();

                // 默认情况下执行跳转所需字节码的偏移量
                // 对应于 switch 中的 default 分支。
                s4 default_offset = reader->reads4();

                // low 和 height 标识了 case 的取值范围。
                s4 low = reader->reads4();
                s4 height = reader->reads4();

                // 跳转偏移量表，对应于各个 case 的情况
                s4 jump_offset_count = height - low + 1;
                s4 jump_offsets[jump_offset_count];
                reader->reads4s(jump_offset_count, jump_offsets);

                // 弹出要判断的值
                index = frame->popi();
                s4 offset;
                if (index < low || index > height) {
                    offset = default_offset; // 没在 case 标识的范围内，跳转到 default 分支。
                } else {
                    offset = jump_offsets[index - low]; // 找到对应的case了
                }

                // The target address that can be calculated from each jump table
                // offset, as well as the one that can be calculated from default,
                // must be the address of an opcode of an instruction within the method
                // that contains this tableswitch instruction.
                reader->pc = saved_pc + offset;
                break;
            }
            case JVM_OPC_lookupswitch: {
                // 实现当各个case值跨度比较大时的 switch 语句
                size_t saved_pc = reader->pc - 1; // save the pc before 'lookupswitch' instruction
                reader->align4();

                // 默认情况下执行跳转所需字节码的偏移量
                // 对应于 switch 中的 default 分支。
                s4 default_offset = reader->reads4();

                // case的个数
                s4 npairs = reader->reads4();
                assert(npairs >= 0); // The npairs must be greater than or equal to 0.

                // match_offsets 有点像 Map，它的 key 是 case 值，value 是跳转偏移量。
                s4 match_offsets[npairs * 2];
                reader->reads4s(npairs * 2, match_offsets);

                // 弹出要判断的值
                jint key = frame->popi();
                s4 offset = default_offset;
                for (int i = 0; i < npairs * 2; i += 2) {
                    if (match_offsets[i] == key) { // 找到 case
                        offset = match_offsets[i + 1];
                        break;
                    }
                }

                // The target address is calculated by adding the corresponding offset
                // to the address of the opcode of this lookupswitch instruction.
                reader->pc = saved_pc + offset;
                break;
            }
            case JVM_OPC_ireturn:
            case JVM_OPC_freturn:
            case JVM_OPC_areturn:
                ret_value_slots_count = 1;
                goto _method_return;
            case JVM_OPC_lreturn:
            case JVM_OPC_dreturn:
                ret_value_slots_count = 2;
                goto _method_return;
            case JVM_OPC_return: {
                ret_value_slots_count = 0;
_method_return:
                TRACE("will return: %s", frame->toString().c_str());
                thread->pop_frame();
                Frame *invoke_frame = thread->top_frame;
                TRACE("invoke frame: %s", invoke_frame == nullptr ? "NULL" : get_frame_info(invoke_frame));
                frame->ostack -= ret_value_slots_count;
                slot_t *ret_value = frame->ostack;
                if (frame->vm_invoke || invoke_frame == nullptr) {
                    if (frame->method->is_synchronized()) {
            //                        _this->unlock();
                    }
                    return ret_value;
                }

                for (int i = 0; i < ret_value_slots_count; i++) {
                    *invoke_frame->ostack++ = *ret_value++;
                }
                if (frame->method->is_synchronized()) {
            //                    _this->unlock();
                }
                CHANGE_FRAME(invoke_frame);
                break;
            }
            case JVM_OPC_getstatic: {
                index = reader->readu2();
                Field *field = cp->resolve_field(index);
                if (!field->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(field->toString());
                }

                init_class(field->clazz);

                *frame->ostack++ = field->static_value.data[0];
                if (field->category_two) {
                    *frame->ostack++ = field->static_value.data[1];
                }
                break;
            }
            case JVM_OPC_putstatic: {
                index = reader->readu2();
                Field *field = cp->resolve_field(index);
                if (!field->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(field->toString());
                }

                init_class(field->clazz);

                if (field->category_two) {
                    frame->ostack -= 2;
                    field->static_value.data[0] = frame->ostack[0];
                    field->static_value.data[1] = frame->ostack[1];
                } else {
                    field->static_value.data[0] = *--frame->ostack;
                }

                break;
            }
            case JVM_OPC_getfield: {
                index = reader->readu2();
                Field *field = cp->resolve_field(index);
                if (field->isStatic()) {
                    // cout << cp->toString().c_str() << endl;
                    // cout << frame->method->getBytecodeString().c_str() << endl;
                    throw java_lang_IncompatibleClassChangeError(field->toString());

                        // init_class(field->clazz);

                        //     *frame->ostack++ = field->static_value.data[0];
                        //     if (field->category_two) {
                        //         *frame->ostack++ = field->static_value.data[1];
                        //     }
                        //     break;
                }

                jref obj = frame->popr();
                if (obj == nullptr) {
                    auto obj_name = get_opc_operating_obj_name(frame, JVM_OPC_getfield);
                    ostringstream oss;
                    oss << "Cannot read \"" << field->name
                        << "\" because " << "\"" << obj_name << "\"" << " is null";
                    throw java_lang_NullPointerException(oss.str());
                }

                *frame->ostack++ = obj->data[field->id];
                if (field->category_two) {
                    *frame->ostack++ = obj->data[field->id + 1];
                }
                break;
            }
            case JVM_OPC_putfield: {
                index = reader->readu2();
                Field *field = cp->resolve_field(index);
                if (field->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(field->toString());
                }

                // 如果是final字段，则只能在构造函数中初始化，否则抛出java.lang.IllegalAccessError。
                if (field->isFinal()) {
                    if (!clazz->equals(field->clazz) || !equals(frame->method->name, "<init>")) {
                        throw java_lang_IllegalAccessError(field->toString());
                    }
                }

                if (field->category_two) {
                    frame->ostack -= 2;
                } else {
                    frame->ostack--;
                }
                slot_t *value = frame->ostack;

                jref obj = frame->popr();
                NULL_POINTER_CHECK(obj);

                obj->set_field_value_raw(field, value);
                break;
            }
            case JVM_OPC_invokevirtual: {
                // invokevirtual指令用于调用对象的实例方法，根据对象的实际类型进行分派（虚方法分派）。
                index = reader->readu2();
                Method *m = cp->resolve_method(index);

                if (m->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(m->toString());
                }

                frame->ostack -= m->arg_slots_count;

                jref obj = slot::get<jref>(frame->ostack);
                NULL_POINTER_CHECK(obj);

                if (m->isPrivate()) {
                    resolved_method = m;
                } else {
                    // assert(m->vtable_index >= 0);
                    // assert(m->vtable_index < (int) obj->clazz->vtable.size());
                    // resolved_method = obj->clazz->vtable[m->vtable_index];
                    resolved_method = obj->clazz->lookup_method(m->name, m->descriptor);
                    if (resolved_method == nullptr) {
                        if (m->is_signature_polymorphic())
                            resolved_method = obj->clazz->generate_poly_method(m->name, m->descriptor);
                    }
                }

                // assert(resolved_method == obj->clazz->lookupMethod(m->name, m->descriptor));
                if (resolved_method == nullptr) {
                    UNREACHABLE("%s\n%s\n", obj->toString().c_str(), m->toString().c_str());
                }
                goto _invoke_method;
            }
            case JVM_OPC_invokespecial: {
                // invokespecial指令用于调用一些需要特殊处理的实例方法， 包括：
                // 1. 构造函数
                // 2. 私有方法
                // 3. 通过super关键字调用的超类方法，或者超接口中的默认方法。
                index = reader->readu2();
                // 存在调用超接口中的默认方法，所以用 resolveMethodOrInterfaceMethod 解析
                Method *m = cp->resolve_method_or_interface_method(index);

                /*
                 * 如果调用的中超类中的函数，但不是构造函数，不是private 函数，且当前类的ACC_SUPER标志被设置，
                 * 需要一个额外的过程查找最终要调用的方法；否则前面从方法符号引用中解析出来的方法就是要调用的方法。
                 * todo 详细说明
                 */
                if (m->clazz->is_super()
                    && !m->isPrivate()
                    && clazz->is_subclass_of(m->clazz) // todo
                    && !equals(m->name, "<init>")) {
                    m = clazz->super_class->lookup_method(m->name, m->descriptor);
                    if (m == nullptr) {
                        // todo  raise_exception0(S(java_lang_NoSuchMethodError), "%s~%s~%s", c->name, name, descriptor);
                    }
                }

                if (m->is_abstract()) {
                    throw java_lang_AbstractMethodError(m->toString());
                }
                if (m->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(m->toString());
                }

                frame->ostack -= m->arg_slots_count;
                jref obj = slot::get<jref>(frame->ostack);
                NULL_POINTER_CHECK(obj);

                resolved_method = m;
                assert(resolved_method);
                goto _invoke_method;
            }
            case JVM_OPC_invokestatic: {
                // invokestatic指令用来调用静态方法。
                // 如果类还没有被初始化，会触发类的初始化。
                index = reader->readu2();
                // 接口中也有 static 方法，所以下面用 resolveMethodOrInterfaceMethod 解析
                Method *m = cp->resolve_method_or_interface_method(index);
                if (m->is_abstract()) {
                    throw java_lang_AbstractMethodError(m->toString());
                }
                if (!m->isStatic()) {
                    throw java_lang_IncompatibleClassChangeError(m->toString());
                }

                init_class(m->clazz);

                frame->ostack -= m->arg_slots_count;
                resolved_method = m;
                assert(resolved_method);
                goto _invoke_method;
            }
            case JVM_OPC_invokeinterface: {
                index = reader->readu2();

                /*
                 * 此字节的值是给方法传递参数需要的slot数，
                 * 其含义和给method结构体定义的arg_slot_count字段相同。
                 * 这个数是可以根据方法描述符计算出来的，它的存在仅仅是因为历史原因。
                 */
                reader->readu1();
                /*
                 * 此字节是留给Oracle的某些Java虚拟机实现用的，它的值必须是0。
                 * 该字节的存在是为了保证Java虚拟机可以向后兼容。
                 */
                reader->readu1();

                Method *m = cp->resolve_interface_method(index);
                assert(m->clazz->is_interface());

                /* todo 本地方法 */

                frame->ostack -= m->arg_slots_count;
                jref obj = slot::get<jref>(frame->ostack);
                NULL_POINTER_CHECK(obj);

                // itable的实现还不对 todo
                // resolved_method = obj->clazz->findFromITable(m->clazz, m->itable_index);
                // assert(resolved_method != nullptr);
                // assert(resolved_method == obj->clazz->lookupMethod(m->name, m->descriptor));
                resolved_method = obj->clazz->lookup_method(m->name, m->descriptor);
                if (resolved_method->is_abstract()) {
                    throw java_lang_AbstractMethodError(resolved_method->toString());
                }

                if (!resolved_method->isPublic()) {
                    throw java_lang_IllegalAccessError(resolved_method->toString());
                }

                assert(resolved_method);
                goto _invoke_method;
            }
            case JVM_OPC_invokedynamic: {
                u2 i = reader->readu2(); // point to JVM_CONSTANT_InvokeDynamic
                reader->readu1(); // this byte must always be zero.
                reader->readu1(); // this byte must always be zero.

                auto inv_dyn = cp->resolve_invoke_dynamic(i);
                jref appendix = nullptr;
                Method *invoker = find_invoke_dynamic_invoker(clazz, inv_dyn, appendix);
                if(invoker == nullptr) {
                    panic(" ");
                        // goto throwException;  todo
                }

                if (appendix != nullptr) {
                    frame->pushr(appendix);
                }

                frame->ostack -= invoker->arg_slots_count;
                resolved_method = invoker;
                goto _invoke_method;
            }
            case JVM_OPC_invokenative: {
                TRACE("%s", frame->toString().c_str());

                // todo 不需要在这里做任何同步的操作

                call_native_method(frame);

                // JNI 函数执行完毕，清空其局部引用表。
                for (int i = 0; i < frame->jni_local_ref_count; i++) {
                    frame->jni_local_ref_table[i] = nullptr;
                }
                frame->jni_local_ref_count = 0;

                // 处理JNI异常
                jref jni_excep = Thread::jniExceptionOccurred();
                if (jni_excep != nullptr) {
                    throw jni_excep;
                }

                //    if (frame->method->isSynchronized()) {
                //        _this->unlock();
                //    }
                break;
            }
//opc_invokehandle: {
//    assert(resolved_method);
//    Frame *new_frame = thread->allocFrame(resolved_method, false);
//    TRACE("Alloc new frame: %s", new_frame->toString().c_str());
//
//    new_frame->lvars = frame->ostack; // todo 什么意思？？？？？？？？
//    CHANGE_FRAME(new_frame)
//    if (IS_SYNCHRONIZED(resolved_method)) {
////        _this->unlock(); // todo why unlock 而不是 lock ................................................
//    }
//    goto opc_invokenative;
//}
_invoke_method: {
                assert(resolved_method);
                Frame *new_frame = thread->alloc_frame(resolved_method, false);
                TRACE("Alloc new frame: %s\n", new_frame->toString().c_str());

                // 准备被调用函数的参数，必须保证frame->ostack已经回退到合适的位置
                new_frame->lvars = frame->ostack;

                CHANGE_FRAME(new_frame);
                if (resolved_method->is_synchronized()) {
            //        _this->unlock(); // todo why unlock 而不是 lock ................................................
                }
                break;
            }

            case JVM_OPC_new: {
                // new指令专门用来创建类实例。数组由专门的指令创建
                // 如果类还没有被初始化，会触发类的初始化。
                Class *c = cp->resolve_class(reader->readu2());
                init_class(c);

                if (c->is_interface() || c->is_abstract()) {
                    throw java_lang_InstantiationException(c->name);
                }

                frame->pushr(Allocator::object(c));
                break;
            }
            case JVM_OPC_newarray: {
                // 创建一维基本类型数组。
                // 包括 boolean[], byte[], char[], short[], int[], long[], float[] 和 double[] 8种。
                jint arr_len = frame->popi();
                if (arr_len < 0) {
                    throw java_lang_NegativeArraySizeException("len is " + to_string(arr_len));
                }

                u1 arr_type = reader->readu1();
                ArrayClass *c = load_type_array_class((ArrayType) arr_type);
                frame->pushr(Allocator::array(c, arr_len));
                break;
            }
            case JVM_OPC_anewarray: {
                // 创建一维引用类型数组
                jint arr_len = frame->popi();
                if (arr_len < 0) {
                    throw java_lang_NegativeArraySizeException("len is " + to_string(arr_len));
                }

                index = reader->readu2();
                ArrayClass *ac = cp->resolve_class(index)->generate_array_class();
                frame->pushr(Allocator::array(ac, arr_len));
                break;
            }
            case JVM_OPC_multianewarray: {
                // 创建多维数组
                index = reader->readu2();
                auto ac = (ArrayClass *) cp->resolve_class(index);

                u1 dim = reader->readu1(); // 多维数组的维度
                if (dim < 1) { // 必须大于或等于1
                    throw java_lang_UnknownError("The dimensions must be greater than or equal to 1.");
                }

                jint lens[dim];
                for (int i = dim - 1; i >= 0; i--) {
                    lens[i] = frame->popi();
                    if (lens[i] < 0) {
                        throw java_lang_NegativeArraySizeException("len is %d" + to_string(lens[i]));
                        // HANDLE_EXCEPTION(S(java_lang_NegativeArraySizeException), nullptr);  // todo msg
                    }
                }
                frame->pushr(Allocator::multi_array(ac, dim, lens));
                break;
            }
            case JVM_OPC_arraylength: {
                Object *o = frame->popr();
                if (o == nullptr) {
                    auto obj_name = get_opc_operating_obj_name(frame, JVM_OPC_arraylength);
                    throw java_lang_NullPointerException(
                                "Cannot read the array length because \"" + string(obj_name) + "\" is null");
                }

                if (!o->is_array_object()) {
                    throw java_lang_UnknownError("not a array");
                }

                frame->pushi(o->arr_len);
                break;
            }
            case JVM_OPC_athrow: {
opc_athrow:
                jref eo = frame->popr(); // exception object
                if (eo == nullptr) {
                    // 异常对象有可能为空
                    // 比如下面的Java代码:
                    // try {
                    //     Exception x = null;
                    //     throw x;
                    // } catch (NullPointerException e) {
                    //     e.printStackTrace();
                    // }

                    // int opc_readed_offset = 1;
                    // int index_in_lvars = getPrevAloadOpcIndex(reader, opc_readed_offset);
                    // if (index_in_lvars < 0) {
                    //     ShouldNotReachHere(" "); // todo  throw java_lang_NullPointerException(FILE_LINE_STR); ???
                    // }
                    // auto [name, descriptor] = frame->method->findLocalVariable(reader->pc - opc_readed_offset, (u2) index_in_lvars);
                    // throw java_lang_NullPointerException(
                    //         "Cannot throw exception because \"" + string(name) + "\" is null");


                    throw java_lang_NullPointerException(FILE_LINE_STR);
                }

                // 遍历虚拟机栈找到可以处理此异常的方法
                while (true) {
                    int handler_pc = frame->method->find_exception_handler(
                            eo->clazz, reader->pc - 1); // instruction length todo 好像是错的
                    if (handler_pc >= 0) {  // todo 可以等于0吗
                        /*
                         * 找到可以处理的代码块了
                         * 操作数栈清空 // todo 为啥要清空操作数栈
                         * 把异常对象引用推入栈顶
                         * 跳转到异常处理代码之前
                         */
                        frame->clear_operand_stack();
                        frame->pushr(eo);
                        reader->pc = (size_t) handler_pc;

                        TRACE("athrow: find exception handler: %s", frame->toString().c_str());
                        break;
                    }

                    if (frame->vm_invoke) {
                        // frame 由虚拟机调用，将异常交由虚拟机处理
                        excep = eo;
                        // return nullptr;
                        throw UncaughtJavaException(eo);
                    }

                    // frame 无法处理异常，弹出
                    thread->pop_frame();

                    if (frame->prev == nullptr) {
                        // 虚拟机栈已空，还是无法处理异常，交由虚拟机处理
                        excep = eo;
                        // return nullptr;
                        throw UncaughtJavaException(eo);
                    }

                    TRACE("athrow: pop frame: %s", frame->toString().c_str());
                    CHANGE_FRAME(frame->prev);
                }
                break;
            }
            case JVM_OPC_checkcast: {
                jref obj = slot::get<jref>(frame->ostack - 1); // 不改变操作数栈
                index = reader->readu2();

                // 如果引用是null，则指令执行结束。也就是说，null 引用可以转换成任何类型
                if (obj != nullptr) {
                    Class *c = cp->resolve_class(index);
                    if (!obj->clazz->check_cast(c)) {
                        throw java_lang_ClassCastException(
                                string(obj->clazz->name) + " cannot be cast to " + c->name);
                    }
                }
                break;
            }
            case JVM_OPC_instanceof: {
                index =  reader->readu2();
                Class *c = cp->resolve_class(index);

                jref obj = frame->popr();
                if (obj == nullptr) {
                    frame->pushi(0);
                } else {
                    frame->pushi(obj->clazz->check_cast(c) ? 1 : 0);
                }
                break;
            }
            case JVM_OPC_monitorenter: {
                jref o = frame->popr();
                if (o == nullptr) {
                    const utf8_t *obj_name = get_opc_operating_obj_name(frame, JVM_OPC_monitorenter);
                    throw java_lang_NullPointerException(
                            "Cannot enter synchronized block because \"" + string(obj_name) + "\" is null");
                }
            //                o->lock();
                break;
            }
            case JVM_OPC_monitorexit: {
                jref o = frame->popr();
                if (o == nullptr) {
                    const utf8_t *obj_name = get_opc_operating_obj_name(frame, JVM_OPC_monitorexit);
                    throw java_lang_NullPointerException(
                            "Cannot exit synchronized block because \"" + string(obj_name) + "\" is null");
                }

            //                o->unlock();
                break;
            }
            case JVM_OPC_wide:
                opcode = reader->readu1();
                PRINT_OPCODE
                index = reader->readu2();
                switch (opcode) {
                    case JVM_OPC_iload:  goto _iload;
                    case JVM_OPC_fload:  goto _fload;
                    case JVM_OPC_aload:  goto _aload;
                    case JVM_OPC_lload:  goto _lload;
                    case JVM_OPC_dload:  goto _dload;
                    case JVM_OPC_istore: goto _istore;
                    case JVM_OPC_fstore: goto _fstore;
                    case JVM_OPC_astore: goto _astore;
                    case JVM_OPC_lstore: goto _lstore;
                    case JVM_OPC_dstore: goto _dstore;
                    case JVM_OPC_ret:    goto opc_ret;
                    case JVM_OPC_iinc:   goto _wide_iinc;
                    default:
                        UNREACHABLE(" "); // todo msg
                }
            case JVM_OPC_ifnull: {
                s2 offset = reader->reads2();
                if (frame->popr() == nullptr) {
                    reader->skip(offset - opcode_len[JVM_OPC_ifnull]);
                }
                break;
            }
            case JVM_OPC_ifnonnull: {
                s2 offset = reader->reads2();
                if (frame->popr() != nullptr) {
                    reader->skip(offset - opcode_len[JVM_OPC_ifnonnull]);
                }
                break;
            }
            case JVM_OPC_goto_w: {
                s4 offset = reader->reads4();
                reader->skip(offset - opcode_len[JVM_OPC_goto_w]);
                break;
            }
            case JVM_OPC_jsr_w: UNREACHABLE("jsr_w doesn't support after jdk 6."); break;
            case JVM_OPC_breakpoint: UNREACHABLE("breakpoint doesn't support in this jvm.");  break;
            case JVM_OPC_impdep2: UNREACHABLE("opc_impdep2 isn't used."); break;
            default: UNREACHABLE(("This instruction isn't used. " + to_string(opcode)).c_str()); break;
        }
    }
}

slot_t *execJava(Method *method, const slot_t *args)
{
    assert(method != nullptr);
    assert(method->arg_slots_count > 0 ? args != nullptr : true);

    Frame *frame = get_current_thread()->alloc_frame(method, true);

    // 准备参数
    for (int i = 0; i < method->arg_slots_count; i++) {
        // 传递参数到被调用的函数。
        frame->lvars[i] = args[i];
    }

    jref excep = nullptr;

    while (true) {
        try {
            slot_t *result = exec(excep);
            return result;
        } catch(jref e) {
            excep = e;
        } catch (JavaException &e) {
            excep = e.get_excep();
        } catch (UncaughtJavaException &e) {
            print_stack_trace(e.java_excep);
            get_current_thread()->terminate(EXIT_CODE_UNCAUGHT_JAVA_EXCEPTION);
        } 
        // catch (...) {
        //     ShouldNotReachHere(" "); // todo msg
        // }
    }
}

jref execJavaR(Method *method, const slot_t *args)
{
    assert(method != nullptr);
    slot_t *slots = execJava(method, args);
    assert(slots != nullptr);
    return slot::get<jref>(slots);
}

// variant<jint, jfloat, jlong, jdouble, jref, monostate>
// exec_java_method(Method *method, const slot_t *args) {
//     jref r = execJavaR(method, args);
//     constexpr variant<jint, jfloat, jlong, jdouble, jref, monostate> va(std::in_place_type<jref>, r);
//     return va;
// }

slot_t *execJava(Method *method, std::initializer_list<slot_t> args)
{
    assert(method != nullptr);
    assert(method->arg_slots_count == args.size());

    slot_t slots[args.size()];
    int i = 0;
    for (slot_t arg: args) {
        slots[i++] = arg;
    }

    return execJava(method, slots);
}

jref execJavaR(Method *m, std::initializer_list<slot_t> args)
{
    assert(m != nullptr);

    slot_t slots[args.size()];

    int i = 0;
    for (slot_t arg: args) {
        slots[i++] = arg;
    }

    return execJavaR(m, slots);
}

slot_t *execJava(Method *m, jref _this, jarrRef args)
{
    assert(m != nullptr);

    // If m is static, this is NULL.
    if (args == nullptr) {
        if (_this != nullptr) {
            assert(!m->isStatic());
            return execJava(m, { rslot(_this) });
        } else {
            assert(m->isStatic());
            return execJava(m, nullptr);
        } 
    }

    // Class[]
    jarrRef types = m->get_parameter_types();
    assert(types != nullptr);
    assert(types->arr_len == args->arr_len);

    // 因为有 category two 的存在，result 的长度最大为 types_len * 2 + this_obj
    auto real_args = new slot_t[2*types->arr_len + 1];
    int k = 0;
    if (_this != nullptr) {
        assert(!m->isStatic());
        slot::set<jref>(real_args, _this);
        k++;
    }
    for (int i = 0; i < types->arr_len; i++) {
        Class *c = types->getElt<jref>(i)->jvm_mirror;
        jref o = args->getElt<jref>(i);

        if (c->is_prim_class()) {
            const slot_t *unbox = o->unbox();
            real_args[k++] = *unbox;
            if (strcmp(c->name, "long") == 0
                || strcmp(c->name, "double") == 0) // category_two
                real_args[k++] = *++unbox;
        } else {
            slot::set<jref>(real_args + k, o);
            k++;
        }
    }

    return execJava(m, real_args);
}

jref execJavaR(Method *m, jref _this, jarrRef args)
{
    const slot_t *slots = execJava(m, _this, args);
    return slot::get<jref>(slots);
}


#undef ET
#undef ARG_LIST
#undef B
#undef Z
#undef C
#undef I
#undef F
#undef R
#undef J
#undef D

#define ET JNIEnv *, jref /*env and this*/
#define ARG_LIST void *nm /*native method*/, JNIEnv *env, jref _this, slot_t *args, Frame *frame

#define B(name) jbyte name = slot::get<jbyte>(args++);
#define Z(name) jbool name = slot::get<jbool>(args++);
#define C(name) jchar name = slot::get<jchar>(args++);
#define I(name) jint name = slot::get<jint>(args++);
#define F(name) jfloat name = slot::get<jfloat>(args++);
#define R(name) jref name = slot::get<jref>(args++);
#define J(name) jlong name = slot::get<jlong>(args); args += 2;
#define D(name) jdouble name = slot::get<jdouble>(args); args += 2;

#define INVOKER_0(sim_desc, func_type, pusher) \
    { sim_desc, [](ARG_LIST) { \
        pusher(((func_type) nm)(env, _this)); \
    }},

#define INVOKER_1(sim_desc, func_type, arg, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg(a) \
        pusher(((func_type) nm)(env, _this, a)); \
    }},

#define INVOKER_2(sim_desc, func_type, arg1, arg2, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b)\
        pusher(((func_type) nm)(env, _this, a, b)); \
    }},

#define INVOKER_3(sim_desc, func_type, arg1, arg2, arg3, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) \
        pusher(((func_type) nm)(env, _this, a, b, c)); \
    }},

#define INVOKER_4(sim_desc, func_type, arg1, arg2, arg3, arg4, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) arg4(d) \
        pusher(((func_type) nm)(env, _this, a, b, c, d)); \
    }},

#define INVOKER_5(sim_desc, func_type, arg1, arg2, arg3, arg4, arg5, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) arg4(d) arg5(e) \
        pusher(((func_type) nm)(env, _this, a, b, c, d, e)); \
    }},

#define INVOKER_6(sim_desc, func_type, arg1, arg2, arg3, arg4, arg5, arg6, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) arg4(d) arg5(e) arg6(f) \
        pusher(((func_type) nm)(env, _this, a, b, c, d, e, f)); \
    }},

#define INVOKER_7(sim_desc, func_type, arg1, arg2, arg3, arg4, arg5, arg6, arg7, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) arg4(d) arg5(e) arg6(f) arg7(g) \
        pusher(((func_type) nm)(env, _this, a, b, c, d, e, f, g)); \
    }},

#define INVOKER_10(sim_desc, func_type, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, pusher) \
    { sim_desc, [](ARG_LIST) { \
        arg1(a) arg2(b) arg3(c) arg4(d) arg5(e) arg6(f) arg7(g) arg8(h) arg9(i) arg10(j)\
        pusher(((func_type) nm)(env, _this, a, b, c, d, e, f, g, h, i, j)); \
    }},

static constexpr struct {
    const char *simplified_descriptor;
    void (* native_invoker)(void *, JNIEnv *, jref , slot_t *, Frame *);
} invoker_map[] {
    INVOKER_0("()V", void(*)(ET), )
    INVOKER_0("()Z", jbool(*)(ET), frame->pushi)
    INVOKER_0("()I", jint(*)(ET), frame->pushi)
    INVOKER_0("()R", jref(*)(ET), frame->pushr)
    INVOKER_0("()J", jlong(*)(ET), frame->pushl)

    INVOKER_1("(I)V", void (*)(ET, jint), I, )
    INVOKER_1("(R)V", void (*)(ET, jref), R, )
    INVOKER_1("(J)V", void (*)(ET, jlong), J, )
    INVOKER_1("(Z)R", jref (*)(ET, jbool), Z, frame->pushr)
    INVOKER_1("(I)R", jref (*)(ET, jint), I, frame->pushr)
    INVOKER_1("(I)I", jint (*)(ET, jint), I, frame->pushi)
    INVOKER_1("(I)Z", jbool(*)(ET, jint), I, frame->pushi)
    INVOKER_1("(I)F", jfloat (*)(ET, jint), I, frame->pushf)
    INVOKER_1("(I)J", jlong(*)(ET, jint), I, frame->pushl)
    INVOKER_1("(D)J", jlong(*)(ET, jdouble), D, frame->pushl)
    INVOKER_1("(J)D", jdouble(*)(ET, jlong), J, frame->pushd)
    INVOKER_1("(J)J", jlong(*)(ET, jlong), J, frame->pushl)
    INVOKER_1("(J)B", jbyte(*)(ET, jlong), J, frame->pushi)
    INVOKER_1("(F)I", jint(*)(ET, jfloat), F, frame->pushi)
    INVOKER_1("(R)R", jref(*)(ET, jref), R, frame->pushr)
    INVOKER_1("(R)Z", jbool(*)(ET, jref), R, frame->pushi)
    INVOKER_1("(R)I", jint(*)(ET, jref), R, frame->pushi)
    INVOKER_1("(R)J", jlong(*)(ET, jref), R, frame->pushl)
    INVOKER_1("(D)D", jdouble(*)(ET, jdouble), J, frame->pushd)

    INVOKER_2("(JJ)V", void(*)(ET, jlong, jlong), J, J, )
    INVOKER_2("(JR)V", void(*)(ET, jlong, jref), J, R, )
    INVOKER_2("(RZ)V", void(*)(ET, jref, jbool), R, Z, )
    INVOKER_2("(RI)V", void(*)(ET, jref, jint), R, I, )
    INVOKER_2("(RR)V", void(*)(ET, jref, jref), R, R, )
    INVOKER_2("(RJ)V", void(*)(ET, jref, jlong), R, J, )
    INVOKER_2("(ZJ)V", void(*)(ET, jboolean, jlong), Z, J, )
    INVOKER_2("(IJ)J", jlong(*)(ET, jint, jlong), I, J, frame->pushl)
    INVOKER_2("(JJ)R", jref(*)(ET, jlong, jlong), J, J, frame->pushr)
    INVOKER_2("(RI)R", jref(*)(ET, jref, jint), R, I, frame->pushr)
    INVOKER_2("(RI)J", jlong(*)(ET, jref, jint), R, I, frame->pushl)
    INVOKER_2("(RZ)Z", jbool(*)(ET, jref, jbool), R, Z, frame->pushi)
    INVOKER_2("(RR)Z", jbool(*)(ET, jref, jref), R, R, frame->pushi)
    INVOKER_2("(RR)I", jint(*)(ET, jref, jref), R, R, frame->pushi)
    INVOKER_2("(RR)J", jlong(*)(ET, jref, jref), R, R, frame->pushl)
    INVOKER_2("(RJ)B", jbyte(*)(ET, jref, jlong), R, J, frame->pushi)
    INVOKER_2("(RJ)S", jshort(*)(ET, jref, jlong), R, J, frame->pushi)
    INVOKER_2("(RJ)I", jint(*)(ET, jref, jlong), R, J, frame->pushi)
    INVOKER_2("(RJ)R", jref(*)(ET, jref, jlong), R, J, frame->pushr)
    INVOKER_2("(RJ)J", jlong(*)(ET, jref, jlong), R, J, frame->pushl)
    INVOKER_2("(RZ)R", jref(*)(ET, jref, jbool), R, Z, frame->pushr)
    INVOKER_2("(RR)R", jref(*)(ET, jref, jref), R, R, frame->pushr)

    INVOKER_3("(RIR)V", void(*)(ET, jref, jint, jref), R, I, R, )
    INVOKER_3("(RJI)V", void(*)(ET, jref, jlong, jint), R, J, I, )
    INVOKER_3("(RJJ)V", void(*)(ET, jref, jlong, jlong), R, J, J, )
    INVOKER_3("(RJR)V", void(*)(ET, jref, jlong, jref), R, J, R, )
    INVOKER_3("(RJC)V", void(*)(ET, jref, jlong, jchar), R, J, C, )
    INVOKER_3("(RRZ)V", void(*)(ET, jref, jref, jbool), R, R, Z, )
    INVOKER_3("(RRR)V", void(*)(ET, jref, jref, jref), R, R, R, )
    INVOKER_3("(RRZ)R", jref(*)(ET, jref, jref, jbool), R, R, Z, frame->pushr)
    INVOKER_3("(RRR)R", jref(*)(ET, jref, jref, jref), R, R, R, frame->pushr)
    INVOKER_3("(RRR)Z", jbool(*)(ET, jref, jref, jref), R, R, R, frame->pushi)
    INVOKER_3("(RRR)J", jlong(*)(ET, jref, jref, jref), R, R, R, frame->pushl)
    INVOKER_3("(RRJ)R", jref(*)(ET, jref, jref, jlong), R, R, J, frame->pushr)
    INVOKER_3("(RII)I", jint(*)(ET, jref, jint, jint), R, I, I, frame->pushi)
    INVOKER_3("(RRJ)I", jint(*)(ET, jref, jref, jlong), R, R, J, frame->pushi)

    INVOKER_4("(RIIZ)V", void(*)(ET, jref, jint, jint, jbool), R, I, I, Z, )
    INVOKER_4("(RRJR)V", void(*)(ET, jref, jref, jlong, jref), R, R, J, R, )
    INVOKER_4("(RRZZ)Z", jbool(*)(ET, jref, jref, jbool, jbool), R, R, Z, Z, frame->pushi)
    INVOKER_4("(RJII)Z", jbool(*)(ET, jref, jlong, jint, jint), R, J, I, I, frame->pushi)
    INVOKER_4("(RRII)I", jint(*)(ET, jref, jref, jint, jint), R, R, I, I, frame->pushi)
    INVOKER_4("(IRII)I", jint(*)(ET, jint, jref, jint, jint), I, R, I, I, frame->pushi)
    INVOKER_4("(RJJJ)Z", jbool(*)(ET, jref, jlong, jlong, jlong), R, J, J, J, frame->pushi)
    INVOKER_4("(RJRR)Z", jbool(*)(ET, jref, jlong, jref, jref), R, J, R, R, frame->pushi)
    INVOKER_4("(RZRR)R", jref(*)(ET, jref, jbool, jref, jref), R, Z, R, R, frame->pushr)
    INVOKER_4("(RRIZ)R", jref(*)(ET, jref, jref, jint, jbool), R, R, I, Z, frame->pushr)
    INVOKER_4("(RRRR)R", jref(*)(ET, jref, jref, jref, jref), R, R, R, R, frame->pushr)
    INVOKER_4("(IJJZ)J", jlong(*)(ET, jint, jlong, jlong, jbool), I, J, J, Z, frame->pushl)

    INVOKER_5("(RRIIZ)V", void(*)(ET, jref, jref, jint, jint, jbool), R, R, I, I, Z, )
    INVOKER_5("(RIRII)V", void(*)(ET, jref, jint, jref, jint, jint), R, I, R, I, I, )
    INVOKER_5("(RJRJJ)V", void(*)(ET, jref, jlong, jref, jlong, jlong), R, J, R, J, J, )
    INVOKER_5("(RZRRR)V", void(*)(ET, jref, jbool, jref, jref, jref), R, Z, R, R, R, )
    INVOKER_5("(RRJJJ)Z", jbool(*)(ET, jref, jref, jlong, jlong, jlong), R, R, J, J, J, frame->pushi)
    INVOKER_5("(RRJRR)Z", jbool(*)(ET, jref, jref, jlong, jref, jref), R, R, J, R, R, frame->pushi)
    INVOKER_5("(RRIIJ)R", jref(*)(ET, jref, jref, jint, jint, jlong), R, R, I, I, J, frame->pushr)
    INVOKER_5("(RRJII)Z", jbool(*)(ET, jref, jref, jlong, jint, jint), R, R, J, I, I, frame->pushi)
    INVOKER_5("(RRZZZ)Z", jbool(*)(ET, jref, jref, jbool, jbool, jbool), R, R, Z, Z, Z, frame->pushi)

    INVOKER_6("(JIIJII)J", jlong(*)(ET, jlong, jint, jint, jlong, jint, jint), J, I, I, J, I, I, frame->pushl)

    INVOKER_7("(RRRIIRR)R", jref(*)(ET, jref, jref, jref, jint, jint, jref, jref),
             R, R, R, I, I, R, R, frame->pushr)
    INVOKER_7("(RRRIRIR)I", jint(*)(ET, jref, jref, jref, jint, jref, jint, jref),
             R, R, R, I, R, I, R, frame->pushi)

    INVOKER_10("(RRRRIIRZIR)R", jref(*)(ET, jref, jref, jref, jref, jint, jint, jref, jbool, jint, jref),
            R, R, R, R, I, I, R, Z, I, R, frame->pushr)
};

static void (* find_native_invoker(const utf8_t *descriptor))(void *, JNIEnv *, jref , slot_t *, Frame *) {
    auto d = descriptor;
    assert(d != nullptr && *d == '(');

    const auto simplified = new char[strlen(d) + 1];
    auto s = simplified;
    for (; *d != 0; d++, s++) {
        switch (*d) {
        case '[':
            while (*++d == '[') {}
            if (*d != 'L') { // 基本类型的数组
                goto __ref;
            }
        case 'L':
            while(*++d != ';') {}
        __ref:
            *s = 'R';
            break;
        default:
            *s = *d;
            break;
        }
    }
    *s = 0;

    for (auto &[simplified_descriptor, native_invoker] : invoker_map) {
        if (strcmp(simplified, simplified_descriptor) == 0) {
            delete[] simplified;
            return native_invoker;
        }
    }

    delete[] simplified;
    return nullptr; // not find
}

static void call_native_method(Frame *frame) {
    assert(frame != nullptr && frame->method != nullptr);
    Method *m = frame->method;
    if (!m->is_native()) {
        // todo
        unimplemented
    }

    slot_t *args = frame->lvars; 
    
    if (m->native_method == nullptr) {
        m->native_method = find_native_from_boot_lib(m->clazz->name, m->name);
        if (m->native_method == nullptr) {
            panic("Native method [%s~%s~%s] was not found.", m->clazz->name, m->name, m->descriptor); // todo
        }
    }

    if(m->is_signature_polymorphic()) {
        const slot_t *v = ((slot_t *(*)(const slot_t *, u2)) m->native_method)(args, m->arg_slots_count);

        switch (m->ret_type) {
            case Method::RET_VOID:      break;
            case Method::RET_BYTE:      frame->pushi(slot::get<jbyte>(v));   break;
            case Method::RET_BOOL:      frame->pushi(slot::get<jbool>(v));   break;
            case Method::RET_CHAR:      frame->pushi(slot::get<jchar>(v));   break;
            case Method::RET_SHORT:     frame->pushi(slot::get<jshort>(v));  break;
            case Method::RET_INT:       frame->pushi(slot::get<jint>(v));    break;
            case Method::RET_FLOAT:     frame->pushf(slot::get<jfloat>(v));  break;
            case Method::RET_LONG:      frame->pushl(slot::get<jlong>(v));   break;
            case Method::RET_DOUBLE:    frame->pushd(slot::get<jdouble>(v)); break;
            case Method::RET_REFERENCE: frame->pushr(slot::get<jref>(v));    break;
            case Method::RET_INVALID:
            default:                    UNREACHABLE("%d", m->ret_type); break;
        }
        
        return;
    }

    if (m->native_invoker == nullptr) {
        const auto invoker = find_native_invoker(m->descriptor);
        if  (invoker == nullptr) {
            panic("The invoker of native method %s~%s~%s was not found. "
                    "It is necessary to add the invoker of this function to the 'invoker_map' array.\n",
                m->clazz->name, m->name, m->descriptor); // todo
        }
        m->native_invoker = invoker;
    }

    jref _this;
    if (m->isStatic()) {
        _this = m->clazz->java_mirror;
    } else {
        _this = slot::get<jref>(args); // this
        args++;
    }

    JNIEnv *env = getJNIEnv();

    m->native_invoker(m->native_method, env, _this, args, frame);

#if 0
    ffi_type *arg_types[args_count_max];
    void *arg_values[args_count_max];

    /* 准备参数 */
    arg_types[0] = &ffi_type_pointer;
    arg_values[0] = &env; 

    arg_types[1] = &ffi_type_pointer;
    if (m->isStatic()) {
        arg_values[1] = &(m->clazz->java_mirror);
    } else {        
        arg_values[1] = (void *) args; // this
        args++;
    }

    int argc = 2;

    const char *p = m->descriptor;
    assert(*p == JVM_SIGNATURE_FUNC);
    p++; // skip start (

    for (; *p != JVM_SIGNATURE_ENDFUNC; args++, p++, argc++) {
        switch (*p) {
            case JVM_SIGNATURE_BOOLEAN:
            case JVM_SIGNATURE_BYTE: {
                jbyte b = getByte(args);
                *(jbyte *) args = b;
                arg_types[argc] = &ffi_type_sint8;
                arg_values[argc] = (void *) args;
                break;
            }
            case JVM_SIGNATURE_CHAR: {
                jchar c = getChar(args);
                *(jchar *) args = c;
                arg_types[argc] = &ffi_type_uint16;
                arg_values[argc] = (void *) args;
                break;
            }
            case JVM_SIGNATURE_SHORT: {
                jshort s = getShort(args);
                *(jshort *) args = s;
                arg_types[argc] = &ffi_type_sint16;
                arg_values[argc] = (void *) args;
                break;
            }
            case JVM_SIGNATURE_INT:
                arg_types[argc] = &ffi_type_sint32;
                arg_values[argc] = (void *) args;
                break;
            case JVM_SIGNATURE_FLOAT:
                arg_types[argc] = &ffi_type_float;
                arg_values[argc] = (void *) args;
                break;
            case JVM_SIGNATURE_LONG:
                arg_types[argc] = &ffi_type_sint64;
                arg_values[argc] = (void *) args;
                args++;
                break;
            case JVM_SIGNATURE_DOUBLE:
                arg_types[argc] = &ffi_type_double;
                arg_values[argc] = (void *) args;
                args++;
                break;
            case JVM_SIGNATURE_ARRAY:
                while (*++p == JVM_SIGNATURE_ARRAY);
                if (*p != JVM_SIGNATURE_CLASS) { // 基本类型的数组
                    goto __ref;
                }
            case JVM_SIGNATURE_CLASS:
                while(*++p != JVM_SIGNATURE_ENDCLASS);
            __ref:
                arg_types[argc] = &ffi_type_pointer;
                arg_values[argc] = (void *) args;
                break;
            default:
                ShouldNotReachHere("%c", *p);
                break;
        }
    }

#define ffi_apply(func, argc, rtype, arg_types, rvalue, arg_values) \
do { \
    ffi_cif cif; \
    ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, rtype, arg_types); \
    ffi_call(&cif, (void (*)(void)) func, rvalue, arg_values); \
} while(0)

    switch (m->ret_type) {
        case Method::RET_VOID:
            ffi_apply(m->native_method, argc, &ffi_type_void, arg_types, nullptr, arg_values);
            break;
        case Method::RET_BYTE:
        case Method::RET_BOOL: {
            jbyte ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_sint8, arg_types, &ret_value, arg_values);
            frame->pushi(ret_value);
            break;
        }
        case Method::RET_CHAR: {
            jchar ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_uint16, arg_types, &ret_value, arg_values);
            frame->pushi(ret_value);
            break;
        }
        case Method::RET_SHORT: {
            jshort ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_sint16, arg_types, &ret_value, arg_values);
            frame->pushi(ret_value);
            break;
        }
        case Method::RET_INT: {
            jint ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_sint32, arg_types, &ret_value, arg_values);
            frame->pushi(ret_value);
            break;
        }
        case Method::RET_FLOAT: {
            jfloat ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_float, arg_types, &ret_value, arg_values);
            frame->pushf(ret_value);
            break;
        }
        case Method::RET_LONG: {
            jlong ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_sint64, arg_types, &ret_value, arg_values);
            frame->pushl(ret_value);
            break;
        }
        case Method::RET_DOUBLE: {
            jdouble ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_double, arg_types, &ret_value, arg_values);
            frame->pushd(ret_value);
            break;
        }
        case Method::RET_REFERENCE: {
            jref ret_value;
            ffi_apply(m->native_method, argc, &ffi_type_pointer, arg_types, &ret_value, arg_values);
            frame->pushr(ret_value);
            break;
        }
        case Method::RET_INVALID:
        default:
            ShouldNotReachHere("%d", m->ret_type);
            break;
    }
#endif
}

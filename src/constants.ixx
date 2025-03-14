export module constants;

/* Used in newarray instruction. */

export enum ArrayType {
    JVM_AT_BOOLEAN = 4,
    JVM_AT_CHAR    = 5,
    JVM_AT_FLOAT   = 6,
    JVM_AT_DOUBLE  = 7,
    JVM_AT_BYTE    = 8,
    JVM_AT_SHORT   = 9,
    JVM_AT_INT     = 10,
    JVM_AT_LONG    = 11
};

/* Constant Pool Entries */
// NOTE: replicated in SA in vm/agent/sun/jvm/hotspot/utilities/ConstantTag.java

export enum {
    JVM_CONSTANT_Invalid = 0, // invalid constant
    JVM_CONSTANT_Utf8 = 1,
    JVM_CONSTANT_Unicode = 2, // unused
    JVM_CONSTANT_Integer = 3,
    JVM_CONSTANT_Float = 4,
    JVM_CONSTANT_Long = 5,
    JVM_CONSTANT_Double = 6,
    JVM_CONSTANT_Class = 7,
    JVM_CONSTANT_String = 8,
    JVM_CONSTANT_Fieldref = 9,
    JVM_CONSTANT_Methodref = 10,
    JVM_CONSTANT_InterfaceMethodref = 11,
    JVM_CONSTANT_NameAndType = 12,
    JVM_CONSTANT_MethodHandle = 15, // JSR 292
    JVM_CONSTANT_MethodType = 16, // JSR 292
    JVM_CONSTANT_Dynamic = 17,
    JVM_CONSTANT_InvokeDynamic = 18, // JSR 292
    JVM_CONSTANT_Module = 19,
    JVM_CONSTANT_Package = 20,

    // Internal constants，数值不同于以上定义的常量即可。
    // JVM_CONSTANT_ResolvedMethodHandle
    // JVM_CONSTANT_ResolvedPolyMethod
    JVM_CONSTANT_ResolvedInvokeDynamic = 80,
    JVM_CONSTANT_ResolvedInterfaceMethod = 81,
    JVM_CONSTANT_ResolvedMethod = 82,
    JVM_CONSTANT_ResolvedField = 83,
    JVM_CONSTANT_ResolvedClass = 84,
    JVM_CONSTANT_ResolvedString = 85,
    JVM_CONSTANT_Placeholder = 86, // long 和 double 的占位符
};

/* JVM_CONSTANT_MethodHandle subtypes */

export enum {
    JVM_REF_NONE                    = 0,
    JVM_REF_getField                = 1,
    JVM_REF_getStatic               = 2,
    JVM_REF_putField                = 3,
    JVM_REF_putStatic               = 4,
    JVM_REF_invokeVirtual           = 5,
    JVM_REF_invokeStatic            = 6,
    JVM_REF_invokeSpecial           = 7,
    JVM_REF_newInvokeSpecial        = 8,
    JVM_REF_invokeInterface         = 9,
    JVM_REF_LIMIT                   = 10
};

/* StackMapTable type item numbers */

export enum {
    JVM_ITEM_Top                = 0,
    JVM_ITEM_Integer            = 1,
    JVM_ITEM_Float              = 2,
    JVM_ITEM_Double             = 3,
    JVM_ITEM_Long               = 4,
    JVM_ITEM_Null               = 5,
    JVM_ITEM_UninitializedThis  = 6,
    JVM_ITEM_Object             = 7,
    JVM_ITEM_Uninitialized      = 8
};


/* Opcodes */

export enum {
    JVM_OPC_nop                 = 0,
    JVM_OPC_aconst_null         = 1,
    JVM_OPC_iconst_m1           = 2,
    JVM_OPC_iconst_0            = 3,
    JVM_OPC_iconst_1            = 4,
    JVM_OPC_iconst_2            = 5,
    JVM_OPC_iconst_3            = 6,
    JVM_OPC_iconst_4            = 7,
    JVM_OPC_iconst_5            = 8,
    JVM_OPC_lconst_0            = 9,
    JVM_OPC_lconst_1            = 10,
    JVM_OPC_fconst_0            = 11,
    JVM_OPC_fconst_1            = 12,
    JVM_OPC_fconst_2            = 13,
    JVM_OPC_dconst_0            = 14,
    JVM_OPC_dconst_1            = 15,
    JVM_OPC_bipush              = 16,
    JVM_OPC_sipush              = 17,
    JVM_OPC_ldc                 = 18,
    JVM_OPC_ldc_w               = 19,
    JVM_OPC_ldc2_w              = 20,
    JVM_OPC_iload               = 21,
    JVM_OPC_lload               = 22,
    JVM_OPC_fload               = 23,
    JVM_OPC_dload               = 24,
    JVM_OPC_aload               = 25,
    JVM_OPC_iload_0             = 26,
    JVM_OPC_iload_1             = 27,
    JVM_OPC_iload_2             = 28,
    JVM_OPC_iload_3             = 29,
    JVM_OPC_lload_0             = 30,
    JVM_OPC_lload_1             = 31,
    JVM_OPC_lload_2             = 32,
    JVM_OPC_lload_3             = 33,
    JVM_OPC_fload_0             = 34,
    JVM_OPC_fload_1             = 35,
    JVM_OPC_fload_2             = 36,
    JVM_OPC_fload_3             = 37,
    JVM_OPC_dload_0             = 38,
    JVM_OPC_dload_1             = 39,
    JVM_OPC_dload_2             = 40,
    JVM_OPC_dload_3             = 41,
    JVM_OPC_aload_0             = 42,
    JVM_OPC_aload_1             = 43,
    JVM_OPC_aload_2             = 44,
    JVM_OPC_aload_3             = 45,
    JVM_OPC_iaload              = 46,
    JVM_OPC_laload              = 47,
    JVM_OPC_faload              = 48,
    JVM_OPC_daload              = 49,
    JVM_OPC_aaload              = 50,
    JVM_OPC_baload              = 51,
    JVM_OPC_caload              = 52,
    JVM_OPC_saload              = 53,
    JVM_OPC_istore              = 54,
    JVM_OPC_lstore              = 55,
    JVM_OPC_fstore              = 56,
    JVM_OPC_dstore              = 57,
    JVM_OPC_astore              = 58,
    JVM_OPC_istore_0            = 59,
    JVM_OPC_istore_1            = 60,
    JVM_OPC_istore_2            = 61,
    JVM_OPC_istore_3            = 62,
    JVM_OPC_lstore_0            = 63,
    JVM_OPC_lstore_1            = 64,
    JVM_OPC_lstore_2            = 65,
    JVM_OPC_lstore_3            = 66,
    JVM_OPC_fstore_0            = 67,
    JVM_OPC_fstore_1            = 68,
    JVM_OPC_fstore_2            = 69,
    JVM_OPC_fstore_3            = 70,
    JVM_OPC_dstore_0            = 71,
    JVM_OPC_dstore_1            = 72,
    JVM_OPC_dstore_2            = 73,
    JVM_OPC_dstore_3            = 74,
    JVM_OPC_astore_0            = 75,
    JVM_OPC_astore_1            = 76,
    JVM_OPC_astore_2            = 77,
    JVM_OPC_astore_3            = 78,
    JVM_OPC_iastore             = 79,
    JVM_OPC_lastore             = 80,
    JVM_OPC_fastore             = 81,
    JVM_OPC_dastore             = 82,
    JVM_OPC_aastore             = 83,
    JVM_OPC_bastore             = 84,
    JVM_OPC_castore             = 85,
    JVM_OPC_sastore             = 86,
    JVM_OPC_pop                 = 87,
    JVM_OPC_pop2                = 88,
    JVM_OPC_dup                 = 89,
    JVM_OPC_dup_x1              = 90,
    JVM_OPC_dup_x2              = 91,
    JVM_OPC_dup2                = 92,
    JVM_OPC_dup2_x1             = 93,
    JVM_OPC_dup2_x2             = 94,
    JVM_OPC_swap                = 95,
    JVM_OPC_iadd                = 96,
    JVM_OPC_ladd                = 97,
    JVM_OPC_fadd                = 98,
    JVM_OPC_dadd                = 99,
    JVM_OPC_isub                = 100,
    JVM_OPC_lsub                = 101,
    JVM_OPC_fsub                = 102,
    JVM_OPC_dsub                = 103,
    JVM_OPC_imul                = 104,
    JVM_OPC_lmul                = 105,
    JVM_OPC_fmul                = 106,
    JVM_OPC_dmul                = 107,
    JVM_OPC_idiv                = 108,
    JVM_OPC_ldiv                = 109,
    JVM_OPC_fdiv                = 110,
    JVM_OPC_ddiv                = 111,
    JVM_OPC_irem                = 112,
    JVM_OPC_lrem                = 113,
    JVM_OPC_frem                = 114,
    JVM_OPC_drem                = 115,
    JVM_OPC_ineg                = 116,
    JVM_OPC_lneg                = 117,
    JVM_OPC_fneg                = 118,
    JVM_OPC_dneg                = 119,
    JVM_OPC_ishl                = 120,
    JVM_OPC_lshl                = 121,
    JVM_OPC_ishr                = 122,
    JVM_OPC_lshr                = 123,
    JVM_OPC_iushr               = 124,
    JVM_OPC_lushr               = 125,
    JVM_OPC_iand                = 126,
    JVM_OPC_land                = 127,
    JVM_OPC_ior                 = 128,
    JVM_OPC_lor                 = 129,
    JVM_OPC_ixor                = 130,
    JVM_OPC_lxor                = 131,
    JVM_OPC_iinc                = 132,
    JVM_OPC_i2l                 = 133,
    JVM_OPC_i2f                 = 134,
    JVM_OPC_i2d                 = 135,
    JVM_OPC_l2i                 = 136,
    JVM_OPC_l2f                 = 137,
    JVM_OPC_l2d                 = 138,
    JVM_OPC_f2i                 = 139,
    JVM_OPC_f2l                 = 140,
    JVM_OPC_f2d                 = 141,
    JVM_OPC_d2i                 = 142,
    JVM_OPC_d2l                 = 143,
    JVM_OPC_d2f                 = 144,
    JVM_OPC_i2b                 = 145,
    JVM_OPC_i2c                 = 146,
    JVM_OPC_i2s                 = 147,
    JVM_OPC_lcmp                = 148,
    JVM_OPC_fcmpl               = 149,
    JVM_OPC_fcmpg               = 150,
    JVM_OPC_dcmpl               = 151,
    JVM_OPC_dcmpg               = 152,
    JVM_OPC_ifeq                = 153,
    JVM_OPC_ifne                = 154,
    JVM_OPC_iflt                = 155,
    JVM_OPC_ifge                = 156,
    JVM_OPC_ifgt                = 157,
    JVM_OPC_ifle                = 158,
    JVM_OPC_if_icmpeq           = 159,
    JVM_OPC_if_icmpne           = 160,
    JVM_OPC_if_icmplt           = 161,
    JVM_OPC_if_icmpge           = 162,
    JVM_OPC_if_icmpgt           = 163,
    JVM_OPC_if_icmple           = 164,
    JVM_OPC_if_acmpeq           = 165,
    JVM_OPC_if_acmpne           = 166,
    JVM_OPC_goto                = 167,
    JVM_OPC_jsr                 = 168,
    JVM_OPC_ret                 = 169,
    JVM_OPC_tableswitch         = 170,
    JVM_OPC_lookupswitch        = 171,
    JVM_OPC_ireturn             = 172,
    JVM_OPC_lreturn             = 173,
    JVM_OPC_freturn             = 174,
    JVM_OPC_dreturn             = 175,
    JVM_OPC_areturn             = 176,
    JVM_OPC_return              = 177,
    JVM_OPC_getstatic           = 178,
    JVM_OPC_putstatic           = 179,
    JVM_OPC_getfield            = 180,
    JVM_OPC_putfield            = 181,
    JVM_OPC_invokevirtual       = 182,
    JVM_OPC_invokespecial       = 183,
    JVM_OPC_invokestatic        = 184,
    JVM_OPC_invokeinterface     = 185,
    JVM_OPC_invokedynamic       = 186,
    JVM_OPC_new                 = 187,
    JVM_OPC_newarray            = 188,
    JVM_OPC_anewarray           = 189,
    JVM_OPC_arraylength         = 190,
    JVM_OPC_athrow              = 191,
    JVM_OPC_checkcast           = 192,
    JVM_OPC_instanceof          = 193,
    JVM_OPC_monitorenter        = 194,
    JVM_OPC_monitorexit         = 195,
    JVM_OPC_wide                = 196,
    JVM_OPC_multianewarray      = 197,
    JVM_OPC_ifnull              = 198,
    JVM_OPC_ifnonnull           = 199,
    JVM_OPC_goto_w              = 200,
    JVM_OPC_jsr_w               = 201,
    JVM_OPC_breakpoint          = 202,

    JVM_OPC_impdep1             = 254,
    JVM_OPC_impdep2             = 255,
    JVM_OPC_invokenative        = JVM_OPC_impdep1,
    JVM_OPC_MAX                 = 255
};

export const unsigned char opcode_length[] = {
   1,   /* nop */
   1,   /* aconst_null */
   1,   /* iconst_m1 */
   1,   /* iconst_0 */
   1,   /* iconst_1 */
   1,   /* iconst_2 */
   1,   /* iconst_3 */
   1,   /* iconst_4 */
   1,   /* iconst_5 */
   1,   /* lconst_0 */
   1,   /* lconst_1 */
   1,   /* fconst_0 */
   1,   /* fconst_1 */
   1,   /* fconst_2 */
   1,   /* dconst_0 */
   1,   /* dconst_1 */
   2,   /* bipush */
   3,   /* sipush */
   2,   /* ldc */
   3,   /* ldc_w */
   3,   /* ldc2_w */
   2,   /* iload */
   2,   /* lload */
   2,   /* fload */
   2,   /* dload */
   2,   /* aload */
   1,   /* iload_0 */
   1,   /* iload_1 */
   1,   /* iload_2 */
   1,   /* iload_3 */
   1,   /* lload_0 */
   1,   /* lload_1 */
   1,   /* lload_2 */
   1,   /* lload_3 */
   1,   /* fload_0 */
   1,   /* fload_1 */
   1,   /* fload_2 */
   1,   /* fload_3 */
   1,   /* dload_0 */
   1,   /* dload_1 */
   1,   /* dload_2 */
   1,   /* dload_3 */
   1,   /* aload_0 */
   1,   /* aload_1 */
   1,   /* aload_2 */
   1,   /* aload_3 */
   1,   /* iaload */
   1,   /* laload */
   1,   /* faload */
   1,   /* daload */
   1,   /* aaload */
   1,   /* baload */
   1,   /* caload */
   1,   /* saload */
   2,   /* istore */
   2,   /* lstore */
   2,   /* fstore */
   2,   /* dstore */
   2,   /* astore */
   1,   /* istore_0 */
   1,   /* istore_1 */
   1,   /* istore_2 */
   1,   /* istore_3 */
   1,   /* lstore_0 */
   1,   /* lstore_1 */
   1,   /* lstore_2 */
   1,   /* lstore_3 */
   1,   /* fstore_0 */
   1,   /* fstore_1 */
   1,   /* fstore_2 */
   1,   /* fstore_3 */
   1,   /* dstore_0 */
   1,   /* dstore_1 */
   1,   /* dstore_2 */
   1,   /* dstore_3 */
   1,   /* astore_0 */
   1,   /* astore_1 */
   1,   /* astore_2 */
   1,   /* astore_3 */
   1,   /* iastore */
   1,   /* lastore */
   1,   /* fastore */
   1,   /* dastore */
   1,   /* aastore */
   1,   /* bastore */
   1,   /* castore */
   1,   /* sastore */
   1,   /* pop */
   1,   /* pop2 */
   1,   /* dup */
   1,   /* dup_x1 */
   1,   /* dup_x2 */
   1,   /* dup2 */
   1,   /* dup2_x1 */
   1,   /* dup2_x2 */
   1,   /* swap */
   1,   /* iadd */
   1,   /* ladd */
   1,   /* fadd */
   1,   /* dadd */
   1,   /* isub */
   1,   /* lsub */
   1,   /* fsub */
   1,   /* dsub */
   1,   /* imul */
   1,   /* lmul */
   1,   /* fmul */
   1,   /* dmul */
   1,   /* idiv */
   1,   /* ldiv */
   1,   /* fdiv */
   1,   /* ddiv */
   1,   /* irem */
   1,   /* lrem */
   1,   /* frem */
   1,   /* drem */
   1,   /* ineg */
   1,   /* lneg */
   1,   /* fneg */
   1,   /* dneg */
   1,   /* ishl */
   1,   /* lshl */
   1,   /* ishr */
   1,   /* lshr */
   1,   /* iushr */
   1,   /* lushr */
   1,   /* iand */
   1,   /* land */
   1,   /* ior */
   1,   /* lor */
   1,   /* ixor */
   1,   /* lxor */
   3,   /* iinc */
   1,   /* i2l */
   1,   /* i2f */
   1,   /* i2d */
   1,   /* l2i */
   1,   /* l2f */
   1,   /* l2d */
   1,   /* f2i */
   1,   /* f2l */
   1,   /* f2d */
   1,   /* d2i */
   1,   /* d2l */
   1,   /* d2f */
   1,   /* i2b */
   1,   /* i2c */
   1,   /* i2s */
   1,   /* lcmp */
   1,   /* fcmpl */
   1,   /* fcmpg */
   1,   /* dcmpl */
   1,   /* dcmpg */
   3,   /* ifeq */
   3,   /* ifne */
   3,   /* iflt */
   3,   /* ifge */
   3,   /* ifgt */
   3,   /* ifle */
   3,   /* if_icmpeq */
   3,   /* if_icmpne */
   3,   /* if_icmplt */
   3,   /* if_icmpge */
   3,   /* if_icmpgt */
   3,   /* if_icmple */
   3,   /* if_acmpeq */
   3,   /* if_acmpne */
   3,   /* goto */
   3,   /* jsr */
   2,   /* ret */
   99,  /* tableswitch */
   99,  /* lookupswitch */
   1,   /* ireturn */
   1,   /* lreturn */
   1,   /* freturn */
   1,   /* dreturn */
   1,   /* areturn */
   1,   /* return */
   3,   /* getstatic */
   3,   /* putstatic */
   3,   /* getfield */
   3,   /* putfield */
   3,   /* invokevirtual */
   3,   /* invokespecial */
   3,   /* invokestatic */
   5,   /* invokeinterface */
   5,   /* invokedynamic */
   3,   /* new */
   2,   /* newarray */
   3,   /* anewarray */
   1,   /* arraylength */
   1,   /* athrow */
   3,   /* checkcast */
   3,   /* instanceof */
   1,   /* monitorenter */
   1,   /* monitorexit */
   0,   /* wide */
   4,   /* multianewarray */
   3,   /* ifnull */
   3,   /* ifnonnull */
   5,   /* goto_w */
   5    /* jsr_w */
};

export const char *opcode_names[] = { 
    "nop",

    /* Constants [0x01 ... 0x14] */
    "aconst_null",
    "iconst_m1", "iconst_0", "iconst_1", "iconst_2", "iconst_3", "iconst_4", "iconst_5",
    "lconst_0", "lconst_1",
    "fconst_0", "fconst_1", "fconst_2",
    "dconst_0", "dconst_1",
    "bipush", "sipush",
    "ldc", "ldc_w", "ldc2_w",

    /* Loads [0x15 ... 0x35] */
    "iload", "lload", "fload", "dload", "aload",
    "iload_0", "iload_1", "iload_2", "iload_3",
    "lload_0", "lload_1", "lload_2", "lload_3",
    "fload_0", "fload_1", "fload_2", "fload_3",
    "dload_0", "dload_1", "dload_2", "dload_3",
    "aload_0", "aload_1", "aload_2", "aload_3",
    "iaload", "laload", "faload", "daload", "aaload", "baload", "caload", "saload",

    /* Stores [0x36 ... 0x56] */
    "istore", "lstore", "fstore", "dstore", "astore",
    "istore_0", "istore_1", "istore_2", "istore_3",
    "lstore_0", "lstore_1", "lstore_2", "lstore_3",
    "fstore_0", "fstore_1", "fstore_2", "fstore_3",
    "dstore_0", "dstore_1", "dstore_2", "dstore_3",
    "astore_0", "astore_1", "astore_2", "astore_3",
    "iastore", "lastore", "fastore", "dastore", "aastore", "bastore", "castore", "sastore",

    /* Stack [0x57 ... 0x5f] */
    "pop", "pop2", "dup", "dup_x1", "dup_x2", "dup2", "dup2_x1", "dup2_x2", "swap",

    /* Math [0x60 ... 0x84] */
    "iadd", "ladd", "fadd", "dadd",
    "isub", "lsub", "fsub", "dsub",
    "imul", "lmul", "fmul", "dmul",
    "idiv", "ldiv", "fdiv", "ddiv",
    "irem", "lrem", "frem", "drem",
    "ineg", "lneg", "fneg", "dneg",
    "ishl", "lshl", "ishr", "lshr", "iushr", "lushr",
    "iand", "land", "ior", "lor", "ixor", "lxor", "iinc",

    /* Conversions [0x85 ... 0x93] */
    "i2l", "i2f", "i2d",
    "l2i", "l2f", "l2d",
    "f2i", "f2l", "f2d",
    "d2i", "d2l", "d2f",
    "i2b", "i2c", "i2s",

    /* Comparisons [0x94 ... 0xa6] */
    "lcmp", "fcmpl", "fcmpg", "dcmpl", "dcmpg",
    "ifeq", "ifne", "iflt", "ifge", "ifgt", "ifle",
    "if_icmpeq", "if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt", "if_icmple",
    "if_acmpeq", "if_acmpne",

    /* Control [0xa7 ... 0xb1] */
    "goto", "jsr", "ret", "tableswitch", "lookupswitch",
    "ireturn", "lreturn", "freturn", "dreturn","areturn", "return",

    /* References [0xb2 ... 0xc3] */
    "getstatic", "putstatic", "getfield", "putfield",
    "invokevirtual", "invokespecial", "invokestatic", "invokeinterface", "invokedynamic",
    "new", "newarray", "anewarray", "arraylength",
    "athrow", "checkcast", "instanceof", "monitorenter", "monitorexit",

    /* Extended [0xc4 ... 0xc9] */
    "wide", "multianewarray", "ifnull", "ifnonnull", "goto_w", "jsr_w",

    /* Reserved [0xca ... 0xff] */
    "breakpoint",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "unused", "unused", "unused", "unused", "unused",
    "unused", "unused", "unused", "invokenative", "impdep2"
};

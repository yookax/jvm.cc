import java.util.Arrays;

public enum EnumTest {
    L_TYPE('L', Object.class),  // all reference types
    I_TYPE('I', int.class),
    J_TYPE('J', long.class),
    F_TYPE('F', float.class),
    D_TYPE('D', double.class),  // all primitive types
    V_TYPE('V', void.class);    // not valid in all contexts

    static final EnumTest[] ALL_TYPES = EnumTest.values();
    static final EnumTest[] ARG_TYPES = Arrays.copyOf(ALL_TYPES, ALL_TYPES.length-1);

    static final int ARG_TYPE_LIMIT = ARG_TYPES.length;
    static final int TYPE_LIMIT = ALL_TYPES.length;

    // Derived int constants, which (unlike the enums) can be constant folded.
    // We can remove them when JDK-8161245 is fixed.
    static final byte
            L_TYPE_NUM = (byte) L_TYPE.ordinal(),
            I_TYPE_NUM = (byte) I_TYPE.ordinal(),
            J_TYPE_NUM = (byte) J_TYPE.ordinal(),
            F_TYPE_NUM = (byte) F_TYPE.ordinal(),
            D_TYPE_NUM = (byte) D_TYPE.ordinal(),
            V_TYPE_NUM = (byte) V_TYPE.ordinal();

    final char btChar;
    final Class<?> btClass;

    EnumTest(char btChar, Class<?> btClass) {
        this.btChar = btChar;
        this.btClass = btClass;
    }

    static { assert(checkBasicType()); }
    private static boolean checkBasicType() {
        for (int i = 0; i < ARG_TYPE_LIMIT; i++) {
            assert ARG_TYPES[i].ordinal() == i;
            assert ARG_TYPES[i] == ALL_TYPES[i];
        }
        for (int i = 0; i < TYPE_LIMIT; i++) {
            assert ALL_TYPES[i].ordinal() == i;
        }
        assert ALL_TYPES[TYPE_LIMIT - 1] == V_TYPE;
        assert !Arrays.asList(ARG_TYPES).contains(V_TYPE);
        return true;
    }

    public static void main(String[] args) {
        System.out.println(L_TYPE);
        System.out.println(L_TYPE_NUM);
    }
}


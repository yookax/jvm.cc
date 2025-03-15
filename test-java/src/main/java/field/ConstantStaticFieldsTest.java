package field;

import java.util.ArrayList;
import java.util.List;

/**
 * A constant variable is a final variable of primitive type or type String
 * that is initialized with a constant expression (§15.28).
 */
public class ConstantStaticFieldsTest {
    
    public static final boolean z = true;
    public static final byte b = 125;
    public static final char c = 'c';
    public static final short s = 300;
    public static final int x = 100;
    public static final int y = x + 18;
    public static final long j = 1L;
    public static final float f = 3.14f;
    public static final double d = 2.71828;
    public static final String str1 = "hello";
    public static final String str2 = str1 + " world!";

    public void test() {
        List<Boolean> list = new ArrayList<>();

        list.add(ConstantStaticFieldsTest.z);
        list.add((Boolean) getFieldValue("z"));
        list.add((byte)125 == (Byte) ConstantStaticFieldsTest.b);
        list.add((byte)125 == (Byte) getFieldValue("b"));
        list.add('c' == ConstantStaticFieldsTest.c);
        list.add('c' == (Character) getFieldValue("c"));
        list.add((short)300 == (Short) ConstantStaticFieldsTest.s);
        list.add((short)300 == (Short) getFieldValue("s"));
        list.add(100 == ConstantStaticFieldsTest.x);
        list.add(100 == (Integer) getFieldValue("x"));
        list.add(118 == ConstantStaticFieldsTest.y);
        list.add(118 == (Integer) getFieldValue("y"));
        list.add(1L == ConstantStaticFieldsTest.j);
        list.add(1L == (Long) getFieldValue("j"));
        list.add(3.14f == (Float) ConstantStaticFieldsTest.f);
        list.add(3.14f == (Float) getFieldValue("f"));
        list.add(2.71828 == (Double) ConstantStaticFieldsTest.d);
        list.add(2.71828 == (Double) getFieldValue("d"));
        list.add("hello".equals(ConstantStaticFieldsTest.str1));
        list.add("hello".equals(getFieldValue("str1")));
        list.add("hello world!".equals(ConstantStaticFieldsTest.str2));
        list.add("hello world!".equals(getFieldValue("str2")));

        for (var b: list) {
            if (!b) {
                System.out.println("failed");
                return;
            }
        }

        System.out.println("passed");
    }
    
    private static Object getFieldValue(String name) {
        try {
            return ConstantStaticFieldsTest.class.getField(name).get(null);
        } catch (ReflectiveOperationException e) {
            throw new RuntimeException(e);
        }
    }

    public static void main(String[] args) {
        new ConstantStaticFieldsTest().test();
    }
    
}

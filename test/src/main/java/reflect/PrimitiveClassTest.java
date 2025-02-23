package reflect;

public class PrimitiveClassTest {
    
    public static void main(String[] args) {
        PrimitiveClassTest test = new PrimitiveClassTest();
        test.test();
    }

    public void test() {
        testPrimitiveClass(void.class,      "void");
        testPrimitiveClass(boolean.class,   "boolean");
        testPrimitiveClass(byte.class,      "byte");
        testPrimitiveClass(char.class,      "char");
        testPrimitiveClass(short.class,     "short");
        testPrimitiveClass(int.class,       "int");
        testPrimitiveClass(long.class,      "long");
        testPrimitiveClass(float.class,     "float");
        testPrimitiveClass(double.class,    "double");
    }
    
    private void testPrimitiveClass(Class<?> c, String name) {
        boolean b1 = (name == c.getName());
        boolean b2 = (null == c.getSuperclass());
        boolean b3 = (0 == c.getFields().length);
        boolean b4 = (0 == c.getDeclaredFields().length);
        boolean b5 = (0 == c.getMethods().length);
        boolean b6 = (0 == c.getDeclaredMethods().length);
        boolean b7 = "java.base".equals(c.getModule().getName());
        System.out.println((b1 && b2 && b3 && b4 && b5 && b6 && b7) ? "Pass. " : "Fail. ");
    }
    
}

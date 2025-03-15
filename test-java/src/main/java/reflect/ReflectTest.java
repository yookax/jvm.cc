package reflect;

import java.util.Arrays;

public class ReflectTest {

    public static void test1() {
        Object obj = new Object();
        Class c1 = obj.getClass();
        Class c2 = c1.getClass();
        Class c3 = c2.getClass();
        System.out.println(c1); // class java.lang.Object
        System.out.println(c2); // class java.lang.Class
        System.out.println(c3); // class java.lang.Class
        System.out.println(c2 == c1); // false
        System.out.println(c2 == c3); // true
    }

    public static void test2() {
        System.out.println(String.class.getName()); // java.lang.String
        System.out.println(byte.class.getName()); // byte
        System.out.println(Object[].class.getName()); // [Ljava.lang.Object;
        System.out.println(int[][][][][][][].class.getName()); // [[[[[[[I
    }

    public static void test3() {
        System.out.println(Object.class); // class java.lang.Object
        System.out.println(Object.class.getInterfaces().length); // 0
        System.out.println(Object.class.getSuperclass()); // null

        System.out.println();

        System.out.println(String.class); // class java.lang.String
        // [interface java.io.Serializable, interface java.lang.Comparable, interface java.lang.CharSequence]
        System.out.println(Arrays.toString(String.class.getInterfaces()));
        System.out.println(String.class.getSuperclass()); // class java.lang.Object

        System.out.println();

        System.out.println(int.class); // int
        System.out.println(int.class.getInterfaces().length); // 0
        System.out.println(int.class.getSuperclass()); // null

        System.out.println();

        System.out.println(int[].class); // class [I
        // [interface java.lang.Cloneable, interface java.io.Serializable]
        System.out.println(Arrays.toString(int[].class.getInterfaces()));
        System.out.println(int[].class.getSuperclass()); // class java.lang.Object

        System.out.println();

        System.out.println(java.io.Serializable.class); // interface java.io.Serializable
        System.out.println(java.io.Serializable.class.getInterfaces().length); // 0
        System.out.println(java.io.Serializable.class.getSuperclass()); // null
    }

    public static void test4() {
        var x = int.class;
        var y = Integer.class;
        System.out.println(x == y);
        System.out.println(int.class == int.class);
        System.out.println(Integer.class == Integer.class);
        System.out.println(int.class == Integer.class);
        System.out.println(int.class == Integer.TYPE);
    }

    private static void printClassInfo(Class<?> c) {
        System.out.println("----------");
        System.out.println(c.getName());
        System.out.println(c.getSimpleName());
        System.out.println(c.getModifiers());
        System.out.println(c.desiredAssertionStatus());
        System.out.println(c.isInterface());
        System.out.println(c.isArray());
        System.out.println(c.isPrimitive());
        System.out.println(c.getComponentType());
        System.out.println(Arrays.toString(c.getSigners()));
        System.out.println(Arrays.toString(c.getInterfaces()));
        System.out.println(c.getEnclosingMethod());
        System.out.println(Arrays.toString(c.getAnnotations()));
        System.out.println(c.getDeclaringClass());
        System.out.println(Arrays.toString(c.getDeclaredClasses()));
    }

    public static void test5() {
        printClassInfo(int.class);
        printClassInfo(int[].class);
        printClassInfo(int[][][].class);

        printClassInfo(ReflectTest.class);
    }

    public static void main(String[] args) {
        ReflectTest.test1();
        ReflectTest.test2();
        ReflectTest.test3();
        ReflectTest.test4();
        ReflectTest.test5();
    }
}

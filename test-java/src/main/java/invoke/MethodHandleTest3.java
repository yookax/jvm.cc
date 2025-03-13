package invoke;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class MethodHandleTest3 {
    private static class A {
        int i = 4;

        int test(Integer k) {
            System.out.println("--------------  test ----------------");
            System.out.println(k);
            MethodHandles.Lookup lookup = MethodHandles.lookup();
            System.out.println(lookup);
            System.out.println(lookup.lookupClass());
            System.out.println("--------------  test over ----------------");
            return 10;
        }
    }

    public static void main(String[] args) throws Throwable {
        MethodHandles.Lookup lookup = MethodHandles.lookup();
        System.out.println(lookup);
        System.out.println(lookup.lookupClass());

        System.out.println("-----------------------------------");

        ClassLoader loader = MethodHandleTest.class.getClassLoader();
        MethodHandle mh = lookup.findVirtual(A.class, "test",
                MethodType.fromMethodDescriptorString("(Ljava/lang/Integer;)I", loader));
        System.out.println(mh.toString());
        System.out.println(mh.getClass());
        System.out.println("+++++++++++++++++++++++++++++++++++");
        System.out.println(mh.invoke(new A(), 6));

        // invokeExact 无法工作 java.lang.invoke.WrongMethodTypeException: expected (A,Integer)int but found (A,Integer)Object
        // System.out.println(mh.invokeExact(new A(), Integer.valueOf(6)));

        MethodType mt = mh.type();
        System.out.println(mt);
        Class<?>[]ptypes = mt.parameterArray();
        for (Class<?> ptype : ptypes) {
            System.out.println(ptype);
        }

        System.out.println("-----------------------------------");

        MethodHandle getter = lookup.findGetter(A.class, "i", int.class);
        System.out.println(getter);
        System.out.println(getter.invoke(new A()));
    }
}

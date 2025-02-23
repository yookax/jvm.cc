package invoke;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class MethodHandleTest {
    
    public static void main(String[] args) throws Throwable {
        MethodHandles.Lookup lookup = MethodHandles.lookup();
        MethodType mt;
        MethodHandle mh;

        mt = MethodType.fromMethodDescriptorString("()Z", null);
        mh = lookup.findStatic(MethodHandleTest.class, "test", mt);
        boolean b1 = (Boolean) mh.invoke();

        mt = MethodType.fromMethodDescriptorString("()I", null);
        mh = lookup.findStatic(MethodHandleTest.class, "test1", mt);
        boolean b2 = ((Integer) mh.invoke() == 7);

        mt = MethodType.fromMethodDescriptorString("(Ljava/lang/Integer;Ljava/lang/Long;)J", null);
        mh = lookup.findStatic(MethodHandleTest.class, "test2", mt);
        boolean b3 = (((Long) mh.invoke(3, 4L)) == 7L);

        mt = MethodType.fromMethodDescriptorString("(IJ)J", null);
        mh = lookup.findStatic(MethodHandleTest.class, "test3", mt);
        boolean b4 = (((Long) mh.invoke(Integer.valueOf(3), 4L)) == 7L);

        mt = MethodType.fromMethodDescriptorString(
                    "(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Object;", null);
        mh = lookup.findStatic(MethodHandleTest.class, "test4", mt);
        boolean b5 = ((Long) mh.invoke((Object) Integer.valueOf(3), (Object) Long.valueOf(4L))) == 7L;

        System.out.println(b1 && b2 && b3 && b4 && b5);
    }
    
    public static boolean test() {
        return true;
    }

    public static int test1() {
        return 7;
    }

    public static long test2(Integer i, Long j) {
        return i + j;
    }

    public static long test3(int i, long j) {
        return i + j;
    }

    public static Object test4(Integer i, Long j) {
        return i + j;
    }
}

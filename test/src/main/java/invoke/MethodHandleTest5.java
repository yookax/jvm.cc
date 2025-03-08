package invoke;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class MethodHandleTest5 {
    public MethodHandle getHandler() {
        MethodHandle mh = null;
        MethodType mt = MethodType.methodType(void.class);
        MethodHandles.Lookup lk = MethodHandles.lookup();

        try {
            mh = lk.findVirtual(MethodHandleTest2.class, "print", mt);
        } catch (Throwable e) {
            e.printStackTrace();
        }

        return mh;
    }

    public void print() {
        System.out.println("print");
    }

    public static void main(String[] args) throws Throwable {
        var mht = new MethodHandleTest4();
        MethodHandle mh = mht.getHandler();

        int result1 = (int) mh.invoke(mht);
        Object result2 = mh.invoke(mht);

        System.out.println("result 1:" + result1);
        System.out.println("result 2:" + result2);
    }
}

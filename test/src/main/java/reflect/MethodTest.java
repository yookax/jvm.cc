package reflect;

import java.lang.reflect.Method;
import java.util.concurrent.Callable;

public class MethodTest implements Callable<Integer> {
    
    public static void main(String[] args) throws Exception {
        boxReturn();
        System.out.print(", ");
        invokeInterfaceMethod();
    }

    public static void boxReturn() throws Exception {
        Method m = MethodTest.class.getMethod("returnLong");
        Object x = m.invoke(new MethodTest());
        System.out.print(3L == (Long) x);
    }

    public static void invokeInterfaceMethod() throws Exception {
        Method m = Callable.class.getMethod("call");
        Object x = m.invoke(new MethodTest());
        System.out.println(7 == (Integer) x);
    }

    @Override
    public Integer call() {
        return 7;
    }

    public long returnLong() {
        return 3;
    }

}

package lambda;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.function.Function;

public class LambdaTest1 {
    int j = 100;

    public boolean test1(Integer i){
        return (this.j == 100) && (i == 43);
    }

    public boolean testLambda(){
        Function<Integer, Boolean> func = this::test1;
        if (!func.apply(43))
            return false;

        MethodType mt = MethodType.methodType(String.class, int.class);
        try {
            MethodHandle mh = MethodHandles.lookup().findVirtual(String.class, "substring", mt);
            String result = (String) mh.invokeExact("abc", 1);
            if (!result.equals("bc"))
                return false;
        } catch (Throwable e) {
            throw new RuntimeException(e);
        }
        return true;
    }

    public static void main(String[] args) {
        System.out.println(new LambdaTest1().testLambda() ? "passed" : "failed");
    }
}

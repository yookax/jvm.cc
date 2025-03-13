package invoke;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

public class MethodHandleTest4 {

    public static MethodHandle getHandler() {
        MethodHandle mh = null;
        MethodType mt = MethodType.methodType(String.class, int.class, int.class);
        MethodHandles.Lookup lk = MethodHandles.lookup();

        try {
            mh = lk.findVirtual(String.class, "substring", mt);
        } catch (Throwable e) {
            e.printStackTrace();
        }

        return mh;
    }

    private static MethodHandle mh = getHandler();
    private static String str = "hello world";

    /*
     invoke和invokeExact方法的区别，从名字上来看，明显是后者准确性更高，或者说要求更严格。
     invokeExact方法在调用时要求严格的类型匹配，方法的返回值类型也在考虑范围之内。

       与invokeExact方法不同，invoke方法允许更加松散的调用方式。它会尝试在调用的时候进行返回值和参数类型的转换工作。
       这是通过MethodHandle类的asType方法来完成的，asType方法的作用是把当前方法句柄适配到新的MethodType上面，
       并产生一个新的方法句柄。当方法句柄在调用时的类型与其声明的类型完全一致的时候，调用invoke方法等于调用invokeExact方法；
       否则，invoke方法会先调用asType方法来尝试适配到调用时的类型。如果适配成功，则可以继续调用。否则会抛出相关的异常。
       这种灵活的适配机制，使invoke方法成为在绝大多数情况下都应该使用的方法句柄调用方式。
      进行类型匹配的基本规则是对比返回值类型和每个参数的类型是否都可以相互匹配。假设源类型为S，目标类型为T，则基本规则如下：
        1、可以通过java的类型转换来完成，一般从子类转成父类，比如从String到Object类型；
        2、可以通过基本类型的转换来完成，只能将类型范围的扩大，比如从int切换到long；
        3、可以通过基本类型的自动装箱和拆箱机制来完成，例如从int到Integer；
        4、如果S有返回值类型，而T的返回值类型为void，则S的返回值会被丢弃。
        5、如果S的返回值是void，而T的返回值是引用类型，T的返回值会是null；
        6、如果S的返回值是void，而T的返回值是基本类型，T的返回值会是0；
     */

    void test1() throws Throwable {
        Object result = mh.invoke(str, 1, 3);
        System.out.println("test1:" + result);
    }

    void test2() throws Throwable {
        // 执行时报错,因为返回的类型为Object，与声明中为String不符合
        // java.lang.invoke.WrongMethodTypeException: expected (String,int,int)String but found (String,int,int)Object
        Object result = mh.invokeExact(str, 1, 3);
        System.out.println("test2:" + result);
    }

    void test3() throws Throwable {
        Object result = (String) mh.invokeExact(str, 1, 3);
        System.out.println("test3:" + result);
    }
}

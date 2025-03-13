package invoke;


import java.lang.invoke.*;

/**
 * 模拟 invokedynamic 指令的执行
 */
public class InvokeDynamicSimulation {

    public static int testMethod(int i, String s) {
        System.out.println(i + ", " + s);
        return 1;
    }

    public static CallSite bootstrapMethod(MethodHandles.Lookup lookup, String name, MethodType mt) throws Throwable {
        MethodHandle mh = lookup.findStatic(InvokeDynamicSimulation.class, name, mt);
        return new ConstantCallSite(mh);
    }

    public static void main(String[] args) throws Throwable {
        MethodHandles.Lookup lookup = MethodHandles.lookup();

        // 调用 bootstrapMethod 得到 CallSite
        // CallSite 代表真正要执行的目标方法调用
        CallSite cs = bootstrapMethod(lookup, "testMethod",
                MethodType.fromMethodDescriptorString("(ILjava/lang/String;)I", null));

        MethodHandle mh = cs.dynamicInvoker();

        Object o =  mh.invoke(3, "test");
        System.out.println(o);

        // invokeExact 调用必须强制转换为 (int)，否则报错。
        // 参数和返回值的类型都必须完全匹配。
        int i = (int) mh.invokeExact(3, "test");
        System.out.println(i);
    }
}

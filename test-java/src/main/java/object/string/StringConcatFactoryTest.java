package object.string;

import java.lang.invoke.*;

public class StringConcatFactoryTest {
    public static void main(String[] args) throws Throwable {
        test1();
        test2();
    }

    private static void test1() throws Throwable {
        MethodHandles.Lookup lookup = MethodHandles.lookup();
        MethodType concatType = MethodType.methodType(String.class, String.class, String.class);

        // 获取 CallSite
        CallSite callSite = StringConcatFactory.makeConcat(lookup, "concat", concatType);

        // 从 CallSite 中获取 MethodHandle
        MethodHandle concatHandle = callSite.getTarget();

        // 调用拼接方法
        String result = (String) concatHandle.invokeExact("Hello", " World");
        System.out.println(result);
    }

    private static void test2() throws Throwable {
        MethodHandles.Lookup lookup = MethodHandles.lookup();
        // 定义拼接方法的参数类型和返回类型
        MethodType concatType = MethodType.methodType(String.class, String.class, int.class, String.class);

        // 定义常量和拼接配方
        String recipe = "Name: \u0001, Age: \u0001, Occupation: \u0001";
        Object[] constants = {};

        // 创建 CallSite
        CallSite callSite = StringConcatFactory.makeConcatWithConstants(
                                lookup, "concat", concatType, recipe, constants);

        // 从 CallSite 获取 MethodHandle
        MethodHandle concatHandle = callSite.getTarget();

        // 模拟一组人员信息
        String[] names = {"Alice", "Bob", "Charlie"};
        int[] ages = {25, 30, 35};
        String[] occupations = {"Engineer", "Doctor", "Teacher"};

        // 循环拼接每个人的信息
        for (int i = 0; i < names.length; i++) {
            String name = names[i];
            int age = ages[i];
            String occupation = occupations[i];

            // 调用拼接方法
            String result = (String) concatHandle.invokeExact(name, age, occupation);
            System.out.println(result);
        }
    }
}

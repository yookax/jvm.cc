package interface0;

import java.util.Arrays;

public class GetInterfaceMethodTest {

    interface Foo {
        default void test() { }
        void bar();
    }

    public static void main(String[] args) {
        System.out.println(Arrays.toString(Foo.class.getDeclaredMethods()));
    }
}

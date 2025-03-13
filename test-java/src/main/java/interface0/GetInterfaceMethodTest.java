package interface0;

import java.util.Arrays;

public class GetInterfaceMethodTest {

    interface Foo {
        default void test() { }
    }

    public static void main(String[] args) {
        System.out.println("Expect output: [public default void interface0.GetInterfaceMethodTest$Foo.test()]");
        System.out.println(Arrays.toString(Foo.class.getDeclaredMethods()));
    }
}

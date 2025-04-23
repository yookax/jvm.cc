package object.string;

import java.util.Optional;

public class StringConstant {

    private static void testDescribeConstable() {
        String name = "abc";
        Optional<String> optional = name.describeConstable();
        System.out.println(optional.get());
    }

    public static void main(String[] args) {
        testDescribeConstable();
    }
}
